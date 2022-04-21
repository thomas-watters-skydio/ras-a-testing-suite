## RAS-A Standard Testing Suite

[![GitHub Actions Status](https://github.com/Auterion/mavlink-testing-suite/workflows/Build/badge.svg?branch=main)](https://github.com/Auterion/ras-a-testing-suite/actions?query=branch%3Amaster)

This project aims to provide a testing suite to test standard compliance of RAS-A-enabled components/systems. RAS-A is based upon mavlink. The definition of mavlink can be found here: https://mavlink.io/en/messages/common.html

Build the testing suite, on Linux/macOS:
```
  mkdir build
  cd build
  cmake ..
  make -j8
```

### Running a test suite

**⚠️ Note:** Make sure the vehicle is in a state in which spool-up can not cause harm, e.g. set to simulation-in-hardware mode or removed props. **The testing suite will send arm and takeoff commands as part of the testing!**

1. Connect the vehicle to your computer, this can be any connection method, e.g. USB, Ethernet, wireless etc.

2. Find the connection URL of your vehicle. The testing suite supports UDP, TCP and serial links. 
For a udp link, the URL would be `udp://<IP>:<PORT>`, e.g. `udp://0.0.0.0:15667`. For a serial link `serial://<serial device>`, e.g. `serial:///dev/ttyACM0`.

3. The test suite contains configurations for different components. To run the tests for the set of minimally required autopilot functionalities, run 
`./ras_a_testing_suite <connection url> ../config/ras_a_minimal_autopilot.yaml` 

#### Changing settings

The config file may need some modifications to your vehicle. For example, the tests for the integer and float params require you to specify an existing param on your vehicle to test against. Also, for the mission protocol, a home location from which the test missions will be planned can be set.


