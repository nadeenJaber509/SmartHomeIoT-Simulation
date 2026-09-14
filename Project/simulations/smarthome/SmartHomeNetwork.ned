package smarthomeiot.simulations.smarthome;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.SensorNode;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedMultiCanvasVisualizer;
import inet.environment.common.PhysicalEnvironment;

network SmartHomeNetwork
{
    @display("bgb=900,600");

    submodules:
        temperatureSensor: SensorNode {
            @display("p=150,150;i=misc/sensor");
        }

        humiditySensor: SensorNode {
            @display("p=300,150;i=misc/sensor");
        }

        motionSensor1: SensorNode {
            @display("p=450,150;i=misc/sensor");
        }

        motionSensor2: SensorNode {
            @display("p=600,150;i=misc/sensor");
        }

        smokeSensor: SensorNode {
            @display("p=750,150;i=misc/sensor");
        }

        airQualitySensor: SensorNode {
            @display("p=200,300;i=misc/sensor");
        }

        doorSensor: SensorNode {
            @display("p=350,300;i=misc/sensor");
        }

        lightSensor: SensorNode {
            @display("p=500,300;i=misc/sensor");
        }

        lightActuator1: SensorNode {
            @display("p=150,450;i=status/bulb");
        }

        lightActuator2: SensorNode {
            @display("p=300,450;i=status/bulb");
        }

        hvacActuator: SensorNode {
            @display("p=500,450;i=misc/sensor");
        }

        alarmActuator: SensorNode {
            @display("p=650,450;i=misc/sensor");
        }

        gateway: SensorNode {
            @display("p=700,300;i=misc/sensorgateway");
        }

        configurator: Ipv4NetworkConfigurator {
            @display("p=70,60");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=490,60");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=210,60");
        }

        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=350,60");
        }
}