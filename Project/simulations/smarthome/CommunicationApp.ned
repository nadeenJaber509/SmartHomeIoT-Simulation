package smarthomeiot.simulations.smarthome;

import inet.applications.contract.IApp;

simple CommunicationApp like IApp
{
    parameters:
        @class(CommunicationApp);
        @display("i=block/app");

        // controller, gateway, or sensor
        string role;

        // polling or pubsub
        string strategy;

        int localPort = default(2000);
        int destinationPort = default(2000);

        string gatewayAddress = default("gateway");
        string sensorAddress = default("smokeSensor");
        string controllerAddress = default("controller");

        double startTime @unit(s) = default(1s);
        double stopTime @unit(s) = default(100s);

        double pollingInterval @unit(s) = default(1s);
        double eventTime @unit(s) = default(30s);

        int requestBytes = default(20);
        int readingBytes = default(50);

    gates:
        input socketIn;
        output socketOut;
}