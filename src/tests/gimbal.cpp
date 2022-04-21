#include <gtest/gtest.h>
#include "../environment.hpp"
using namespace RASATestingSuite;

class Gimbal : public ::testing::Test {
protected:
    const std::shared_ptr<PassthroughTester> link;
    const TestTargetAddress target;


    Gimbal() :
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

TEST_F(Gimbal, RequestGimbalManagerInformation) {
    auto conf = Environment::getInstance()->getConfig({"Gimbal", "RequestGimbalManagerInformation"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<GIMBAL_MANAGER_INFORMATION>();
}

TEST_F(Gimbal, SetGimbalROILocation) {
    auto conf = Environment::getInstance()->getConfig({"Gimbal", "SetGimbalROILocation"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    link->send<COMMAND_INT>(target, MAV_FRAME_GLOBAL_INT,
                             MAV_CMD_DO_SET_ROI_LOCATION, 0, 0, 154.,
                             NAN, NAN, NAN, 0, 0, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_DO_SET_ROI_LOCATION);
}

TEST_F(Gimbal, SetGimbalROINone) {
    auto conf = Environment::getInstance()->getConfig({"Gimbal", "SetGimbalROINone"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    link->send<COMMAND_INT>(target, MAV_FRAME_GLOBAL_INT,
                            MAV_CMD_DO_SET_ROI_NONE, 0, 0, 154.,
                            NAN, NAN, NAN, 0, 0, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_DO_SET_ROI_NONE);
}
