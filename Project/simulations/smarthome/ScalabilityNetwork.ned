package smarthomeiot.simulations.smarthome;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.SensorNode;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedMultiCanvasVisualizer;

network ScalabilityNetwork
{
    parameters:
        int numSensors = default(5);

        @display("bgb=900,600");

    submodules:

        sensor[numSensors]: SensorNode {
            @display("i=misc/sensor");
        }

        gateway: SensorNode {
            @display("p=700,300;i=misc/sensorgateway");
        }

        configurator: Ipv4NetworkConfigurator {
            @display("p=70,60");
        }

        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=210,60");
        }

        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=350,60");
        }
}