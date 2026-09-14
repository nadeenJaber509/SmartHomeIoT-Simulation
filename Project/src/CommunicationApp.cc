#include <omnetpp.h>
#include <string>
#include <algorithm>

#include "inet/common/InitStages.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

using namespace omnetpp;
using namespace inet;

class CommunicationApp :
    public cSimpleModule,
    public UdpSocket::ICallback
{
  private:
    UdpSocket socket;

    cMessage *actionTimer = nullptr;
    cMessage *eventTimer = nullptr;

    std::string role;
    std::string strategy;

    simtime_t stopTime;
    simtime_t eventTime;
    simtime_t pollingInterval;

    bool subscribed = false;
    bool subscriptionConfirmed = false;
    bool eventDetected = false;

    long sequence = 0;

    long packetsSent = 0;
    long packetsReceived = 0;
    long bytesSent = 0;
    long forwardedPackets = 0;
    long eventsGenerated = 0;
    long repliesReceived = 0;
    long socketErrors = 0;

    double detectionDelay = 0;

    // Used to restore metadata if fragmentation/reassembly
    // removes the parameters attached to the packet.
    long lastPollId = -1;
    double lastPollRequestTime = -1;

    cStdDev pollRtt;

    void sendMessage(const char *kind,
                     const char *destination,
                     long id,
                     double requestTime,
                     bool active,
                     double generatedAt)
    {
        const std::string type = kind;

        const bool control =
            type == "POLL" ||
            type == "SUBSCRIBE" ||
            type == "SUBACK";

        int length = control
            ? par("requestBytes").intValue()
            : par("readingBytes").intValue();

        auto packet = new Packet(kind);

        // Application metadata.
        packet->addPar("sequence") = id;
        packet->addPar("requestTime") = requestTime;
        packet->addPar("active") = active;
        packet->addPar("generatedAt") = generatedAt;

        // The requested message size is represented by this payload.
        auto payload = makeShared<ByteCountChunk>(B(length), 0);
        packet->insertAtBack(payload);

        auto address =
            L3AddressResolver().resolve(destination);

        socket.sendTo(
            packet,
            address,
            par("destinationPort").intValue()
        );

        packetsSent++;
        bytesSent += length;

        if (role == "gateway" &&
            (type == "POLL" ||
             type == "READING" ||
             type == "NOTIFY"))
        {
            forwardedPackets++;
        }
    }

    void scheduleNextAction(simtime_t interval)
    {
        simtime_t next = simTime() + interval;

        if (next < stopTime)
            scheduleAt(next, actionTimer);
    }

  protected:
    int numInitStages() const override
    {
        return NUM_INIT_STAGES;
    }

    void initialize(int stage) override
    {
        if (stage == INITSTAGE_LOCAL) {
            role = par("role").stringValue();
            strategy = par("strategy").stringValue();

            stopTime = par("stopTime");
            eventTime = par("eventTime");
            pollingInterval = par("pollingInterval");

            if (role != "controller" &&
                role != "gateway" &&
                role != "sensor")
            {
                throw cRuntimeError(
                    "Invalid CommunicationApp role"
                );
            }

            if (strategy != "polling" &&
                strategy != "pubsub")
            {
                throw cRuntimeError(
                    "Invalid communication strategy"
                );
            }

            if (pollingInterval <= SIMTIME_ZERO) {
                throw cRuntimeError(
                    "pollingInterval must be positive"
                );
            }

            if (par("requestBytes").intValue() <= 0 ||
                par("readingBytes").intValue() <= 0)
            {
                throw cRuntimeError(
                    "Message sizes must be positive"
                );
            }

            actionTimer = new cMessage("actionTimer");
            eventTimer = new cMessage("eventTimer");

            pollRtt.setName("pollRtt");
        }

        if (stage == INITSTAGE_APPLICATION_LAYER) {
            socket.setOutputGate(gate("socketOut"));
            socket.setCallback(this);
            socket.bind(par("localPort").intValue());

            simtime_t start = par("startTime");

            if (start < simTime() ||
                eventTime < simTime())
            {
                throw cRuntimeError(
                    "Start/event time is in the past"
                );
            }

            if (role == "controller" &&
                start < stopTime)
            {
                scheduleAt(start, actionTimer);
            }

            if (role == "sensor" &&
                eventTime < stopTime)
            {
                scheduleAt(eventTime, eventTimer);
            }
        }
    }

    void handleMessage(cMessage *message) override
    {
        if (message == actionTimer) {
            const char *gateway =
                par("gatewayAddress").stringValue();

            if (strategy == "polling") {
                long pollId = sequence++;
                double pollTime = simTime().dbl();

                // Remember the most recently issued poll.
                lastPollId = pollId;
                lastPollRequestTime = pollTime;

                sendMessage(
                    "POLL",
                    gateway,
                    pollId,
                    pollTime,
                    false,
                    -1
                );

                scheduleNextAction(pollingInterval);
            }
            else if (!subscriptionConfirmed) {
                sendMessage(
                    "SUBSCRIBE",
                    gateway,
                    0,
                    simTime().dbl(),
                    false,
                    -1
                );

                // Retry until the acknowledgement arrives.
                scheduleNextAction(SimTime(1));
            }
        }
        else if (message == eventTimer) {
            eventsGenerated++;

            if (strategy == "pubsub") {
                sendMessage(
                    "PUBLISH",
                    par("gatewayAddress").stringValue(),
                    0,
                    -1,
                    true,
                    eventTime.dbl()
                );
            }
        }
        else {
            socket.processMessage(message);
        }
    }

    void socketDataArrived(
        UdpSocket *,
        Packet *packet
    ) override
    {
        packetsReceived++;

        if (simTime() >= stopTime) {
            delete packet;
            return;
        }

        std::string kind = packet->getName();

        // Parameters attached directly to a Packet may be absent
        // after fragmentation and reassembly.
        long id = packet->hasPar("sequence")
            ? packet->par("sequence").longValue()
            : -1;

        double issued = packet->hasPar("requestTime")
            ? packet->par("requestTime").doubleValue()
            : -1;

        bool active = packet->hasPar("active")
            ? packet->par("active").boolValue()
            : false;

        double generated = packet->hasPar("generatedAt")
            ? packet->par("generatedAt").doubleValue()
            : -1;

        if (role == "gateway") {
            if (strategy == "polling" &&
                kind == "POLL")
            {
                // Store the metadata before forwarding the poll.
                lastPollId = id;
                lastPollRequestTime = issued;

                sendMessage(
                    "POLL",
                    par("sensorAddress").stringValue(),
                    id,
                    issued,
                    false,
                    -1
                );
            }
            else if (strategy == "polling" &&
                     kind == "READING")
            {
                // Restore metadata if the reading was fragmented.
                if (id < 0)
                    id = lastPollId;

                if (issued < 0)
                    issued = lastPollRequestTime;

                if (!packet->hasPar("active")) {
                    active = simTime() >= eventTime;
                    generated =
                        active ? eventTime.dbl() : -1;
                }

                sendMessage(
                    "READING",
                    par("controllerAddress").stringValue(),
                    id,
                    issued,
                    active,
                    generated
                );
            }
            else if (strategy == "pubsub" &&
                     kind == "SUBSCRIBE")
            {
                subscribed = true;

                sendMessage(
                    "SUBACK",
                    par("controllerAddress").stringValue(),
                    id,
                    issued,
                    false,
                    -1
                );
            }
            else if (strategy == "pubsub" &&
                     kind == "PUBLISH" &&
                     subscribed)
            {
                sendMessage(
                    "NOTIFY",
                    par("controllerAddress").stringValue(),
                    id,
                    issued,
                    active,
                    generated
                );
            }
        }
        else if (role == "sensor" &&
                 strategy == "polling" &&
                 kind == "POLL")
        {
            bool eventActive =
                simTime() >= eventTime;

            sendMessage(
                "READING",
                par("gatewayAddress").stringValue(),
                id,
                issued,
                eventActive,
                eventActive ? eventTime.dbl() : -1
            );
        }
        else if (role == "controller") {
            if (strategy == "pubsub" &&
                kind == "SUBACK")
            {
                subscriptionConfirmed = true;
                cancelEvent(actionTimer);
            }

            if (strategy == "polling" &&
                kind == "READING")
            {
                // Restore metadata if the forwarded reading
                // was fragmented on the gateway-controller hop.
                if (id < 0)
                    id = lastPollId;

                if (issued < 0)
                    issued = lastPollRequestTime;

                if (!packet->hasPar("active")) {
                    active = simTime() >= eventTime;
                    generated =
                        active ? eventTime.dbl() : -1;
                }

                repliesReceived++;

                if (issued >= 0) {
                    pollRtt.collect(
                        simTime().dbl() - issued
                    );
                }
            }

            bool isReading =
                strategy == "polling" &&
                kind == "READING";

            bool isNotification =
                strategy == "pubsub" &&
                kind == "NOTIFY";

            if ((isReading || isNotification) &&
                active &&
                !eventDetected)
            {
                eventDetected = true;

                if (generated >= 0) {
                    detectionDelay =
                        simTime().dbl() - generated;
                }
            }
        }

        delete packet;
    }

    void socketErrorArrived(
        UdpSocket *,
        Indication *indication
    ) override
    {
        socketErrors++;
        delete indication;
    }

    void socketClosed(UdpSocket *) override
    {
    }

    void finish() override
    {
        recordScalar(
            "appPacketsSent",
            packetsSent
        );

        recordScalar(
            "appPacketsReceived",
            packetsReceived
        );

        recordScalar(
            "appBytesSent",
            bytesSent
        );

        recordScalar(
            "forwardedPackets",
            forwardedPackets
        );

        recordScalar(
            "socketErrors",
            socketErrors
        );

        if (role == "sensor") {
            recordScalar(
                "eventsGenerated",
                eventsGenerated
            );
        }

        if (role == "controller") {
            recordScalar(
                "eventDetected",
                eventDetected ? 1 : 0
            );

            recordScalar(
                "pollRepliesReceived",
                repliesReceived
            );

            if (eventDetected) {
                recordScalar(
                    "eventDetectionDelay",
                    detectionDelay
                );
            }

            if (pollRtt.getCount() > 0) {
                recordScalar(
                    "pollRttMean",
                    pollRtt.getMean()
                );

                recordScalar(
                    "pollRttMax",
                    pollRtt.getMax()
                );
            }

            if (strategy == "pubsub") {
                recordScalar(
                    "subscriptionConfirmed",
                    subscriptionConfirmed ? 1 : 0
                );
            }
        }
    }

  public:
    ~CommunicationApp() override
    {
        cancelAndDelete(actionTimer);
        cancelAndDelete(eventTimer);
    }
};

Define_Module(CommunicationApp);
