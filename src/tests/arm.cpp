#include <gtest/gtest.h>
#include "../environment.hpp"
using namespace RASATestingSuite;

class Arm : public ::testing::Test {
protected:
    const std::shared_ptr<PassthroughTester> link;
    const TestTargetAddress target;

    Arm() :
          link(Environment::getInstance()->getPassthroughTester()),
          target(Environment::getInstance()->getTargetAddress()) {
        link->flushAll();
    }
};

TEST_F(Arm, ArmDisarm) {
    auto conf = Environment::getInstance()->getConfig({"Arm", "ArmDisarm"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    link->send<COMMAND_LONG>(target, MAV_CMD_COMPONENT_ARM_DISARM,
                             0, 1., NAN, NAN, NAN, NAN, NAN, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);

    EXPECT_TRUE(link->expectCondition<HEARTBEAT>(target, 10, 2000, [](auto msg) {
        return (msg.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) == MAV_MODE_FLAG_SAFETY_ARMED;
    })) << "MAV_MODE_SAFETY_ARMED not set to true";


    link->send<COMMAND_LONG>(target, MAV_CMD_COMPONENT_ARM_DISARM,
                             0, 0.f, NAN, NAN, NAN, NAN, NAN, NAN);
    ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);

    link->flush<HEARTBEAT>(target);

    EXPECT_TRUE(link->expectCondition<HEARTBEAT>(target, 10, 2000, [](auto msg) {
        return (msg.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) == 0;
    })) << "MAV_MODE_SAFETY_ARMED not re-set to false";

}

