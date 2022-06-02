## RAS-A Standard Testing Suite

[![GitHub Actions Status](https://github.com/Auterion/mavlink-testing-suite/workflows/Build/badge.svg?branch=main)](https://github.com/Auterion/ras-a-testing-suite/actions?query=branch%3Amaster)

This project aims to provide a testing suite to test standard compliance of RAS-A-enabled components/systems. RAS-A is based upon mavlink. The definition of mavlink can be found here: https://mavlink.io/en/messages/common.html

## Building the test suite

Build the testing suite, on Linux/macOS:
```
  mkdir build
  cd build
  cmake ..
  make -j8
```

## Running a test suite

**⚠️ Note:** Make sure the vehicle is in a state in which spool-up can not cause harm, e.g. set to simulation-in-hardware mode or removed props. **The testing suite will send arm and takeoff commands as part of the testing!**

1. Connect the vehicle to your computer, this can be any connection method, e.g. USB, Ethernet, wireless etc.

2. Find the connection URL of your vehicle. The testing suite supports UDP, TCP and serial links. 
For a udp link, the URL would be `udp://<IP>:<PORT>`, e.g. `udp://0.0.0.0:15667`. For a serial link `serial://<serial device>`, e.g. `serial:///dev/ttyACM0`.

3. The test suite contains configurations for different components. To run the tests for the set of minimally required autopilot functionalities, run 
`./ras_a_testing_suite <connection url> ../config/ras_a_minimal_autopilot.yaml` 

4. To store the test-results as a file, you can add the option `--gtest_output=xml`. This will create an XML that you can share with the test results.

#### Changing settings

The config file may need some modifications to your vehicle. For example, the tests for the integer and float params require you to specify an existing param on your vehicle to test against. Also, for the mission protocol, a home location from which the test missions will be planned can be set.


## Configuration options

The tests to run in the test suite are configured in *yaml* configuration files. The *config* folder contains configuration files to test the minimally required feature set of RAS-A for autopilot, gimbal and camera components. 

### System IDs / Component IDs

Each configuration file specifies the target system and component ids at the beginning of the file. All tests in the configuration file are then run against this specified system and component ids. 

If your system uses different system / component ids, then these values have to adjusted for each component. 

### Skipping tests

Each test can be skipped by either setting a `skip: true` or by removing the configuration block for the specific test in the config file.

### Time stretching

If the tests are run against a slower-than-real-time simultation, you can, for example for the *Telemetry* test suite, set a `sim_factor: 0.75`. This factor should correspond to the effective speed the simultation runs compared to real time. It relaxes the timing constraints in the test suite.


## Running in CI

The return value of the `ras_a_testing_suite` binary can be used to determine if the test run was succesful or not. The testing framework is built on google test (gtest). The test result XML can be used for reporting in the CI system. 
