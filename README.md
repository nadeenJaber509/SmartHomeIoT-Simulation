# Smart Home IoT Network Simulation

This project models and evaluates a smart-home IoT network using OMNeT++ and the INET Framework. The network uses IEEE 802.15.4 communication between sensors, a gateway, actuators, and a controller.

## Software Requirements

- OMNeT++ 6.4.0
- INET Framework 4.7.0
- macOS or another OMNeT++-supported operating system

## Project Structure

- `Project/simulations`: NED networks, configuration files, XML files, and simulation scenarios.
- `Project/src`: Communication application source code, Makefile, and executable.
- `Reproducibility_Note.txt`: Detailed instructions for reproducing the experiments.

## Experiments

The project contains the following experiments:

- Scalability with different numbers of sensor nodes.
- Reporting-interval evaluation.
- Critical-event performance under different traffic loads.
- Polling versus Publish/Subscribe communication.
- Packet-size and IEEE 802.15.4 fragmentation.
- Bonus comparison with and without a brick wall.

## Running the Project

1. Import the `Project` folder into the OMNeT++ IDE.
2. Make sure INET 4.7.0 is available and referenced.
3. Build the project in release mode.
4. Open `Project/simulations/omnetpp.ini`.
5. Select the required configuration.
6. Run it using Cmdenv or Qtenv.

Available configurations include:

- `Scalability`
- `ReportingInterval`
- `CriticalEvent`
- `Polling`
- `PubSub`
- `PacketSize`
- `BonusNoWalls`
- `BonusWithWalls`

For complete run and reproducibility details, see `Reproducibility_Note.txt`.

## Authors

Birzeit University  
ENCS5323 — Wireless and Mobile Networks
