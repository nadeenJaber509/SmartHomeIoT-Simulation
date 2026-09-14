package smarthomeiot.simulations.smarthome;

import inet.node.inet.SensorNode;

network CommunicationStrategiesNetwork extends SmartHomeNetwork
{
    submodules:
        controller: SensorNode {
            @display("p=850,450;i=device/laptop");
        }
}