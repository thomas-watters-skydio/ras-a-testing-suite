#include <gtest/gtest.h>
#include "../environment.hpp"
using namespace RASATestingSuite;

class Command : public ::testing::Test {
protected:
    const std::shared_ptr<PassthroughTester> link;
    const TestTargetAddress target;


    Command() :
          link(Environment::getInstance()->getPassthroughTester()),
          target(Environment::getInstance()->getTargetAddress()) {
        link->flushAll();
    }

    template <int MSG>
    void requestMessageCommand() {
        // make sure to flush all existing ALTITUDE messages
        link->flush<MSG>(target);
        link->send<COMMAND_LONG>(target,
                                 MAV_CMD_REQUEST_MESSAGE, 0,
                                 static_cast<float>(msg_helper<MSG>::ID), NAN, NAN, NAN, NAN, NAN, NAN);
        auto ack = link->receive<COMMAND_ACK>(target);
        EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
        EXPECT_EQ(ack.command, MAV_CMD_REQUEST_MESSAGE);
        link->receive<MSG>(target, 1000);
    }
};

TEST_F(Command, RequestMessage) {
    auto conf = Environment::getInstance()->getConfig({"Command", "RequestMessage"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<PROTOCOL_VERSION>();
}

TEST_F(Command, RequestProtocolVersion) {
    auto conf = Environment::getInstance()->getConfig({"Command", "RequestProtocolVersion"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    // make sure there is no PROTOCOL_VERSION being published
    try {
        link->receive<PROTOCOL_VERSION>(target);
        FAIL() << "PROTOCOL_VERSION published before requesting. Can not do test";
    } catch(...) {}
    link->send<COMMAND_LONG>(target,
                             MAV_CMD_REQUEST_PROTOCOL_VERSION, 0,
                             1.f, NAN, NAN, NAN, NAN, NAN, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_REQUEST_PROTOCOL_VERSION);
    auto version = link->receive<PROTOCOL_VERSION>(target, 1000);
    EXPECT_GE(version.version, 200);
}

TEST_F(Command, RequestAutopilotCapabilities) {
    auto conf = Environment::getInstance()->getConfig({"Command", "RequestAutopilotCapabilities"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    // make sure there is no PROTOCOL_VERSION being published
    try {
        link->receive<AUTOPILOT_VERSION>(target);
        FAIL() << "AUTOPILOT_VERSION published before requesting. Can not do test";
    } catch(...) {}
    link->send<COMMAND_LONG>(target,
                             MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES, 0,
                             1.f, NAN, NAN, NAN, NAN, NAN, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES);
    link->receive<AUTOPILOT_VERSION>(target, 1000);
}

TEST_F(Command, RequestAltitude) {
    auto conf = Environment::getInstance()->getConfig({"Command", "RequestAltitude"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<ALTITUDE>();
}

TEST_F(Command, RequestPoiReport) {
    auto conf = Environment::getInstance()->getConfig({"Command", "RequestPoiReport"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<POI_REPORT>();
}

TEST_F(Command, RequestHomePosition) {
    auto conf = Environment::getInstance()->getConfig({"Command", "RequestHomePosition"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<HOME_POSITION>();
}

TEST_F(Command, RequestFlightInformation) {
    auto conf = Environment::getInstance()->getConfig({"Command", "RequestFlightInformation"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<FLIGHT_INFORMATION>();
}

TEST_F(Command, SetMessageInterval) {
    auto conf = Environment::getInstance()->getConfig({"Command", "SetMessageInterval"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    link->flush<MESSAGE_INTERVAL>(target);

    link->send<COMMAND_LONG>(target,
                             MAV_CMD_SET_MESSAGE_INTERVAL, 0,
                             msg_helper<ATTITUDE>::ID, 200000.F, 0.f, NAN, NAN, NAN, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_SET_MESSAGE_INTERVAL);
}
