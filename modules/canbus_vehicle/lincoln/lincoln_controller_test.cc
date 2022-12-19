/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "modules/canbus_vehicle/lincoln/lincoln_controller.h"

#include "cyber/common/file.h"
#include "gtest/gtest.h"

#include "modules/canbus/proto/canbus_conf.pb.h"
#include "modules/common_msgs/chassis_msgs/chassis.pb.h"
#include "modules/canbus_vehicle/lincoln/proto/lincoln.pb.h"
#include "modules/canbus_vehicle/lincoln/lincoln_message_manager.h"
#include "modules/common_msgs/basic_msgs/vehicle_signal.pb.h"
#include "modules/common_msgs/control_msgs/control_cmd.pb.h"
#include "modules/drivers/canbus/can_comm/can_sender.h"

namespace apollo {
namespace canbus {
namespace lincoln {

using apollo::common::ErrorCode;
using apollo::common::VehicleSignal;
using apollo::control::ControlCommand;

class LincolnControllerTest : public ::testing::Test {
 public:
  virtual void SetUp() {
    std::string canbus_conf_file =
        "/apollo/modules/canbus/testdata/conf/mkz_canbus_conf_test.pb.txt";
    cyber::common::GetProtoFromFile(canbus_conf_file, &canbus_conf_);
    params_ = canbus_conf_.vehicle_parameter();
    control_cmd_.set_throttle(20.0);
    control_cmd_.set_brake(0.0);
    control_cmd_.set_steering_rate(80.0);
    control_cmd_.set_horn(false);
  }

 protected:
  LincolnController controller_;
  ControlCommand control_cmd_;
  VehicleSignal vehicle_signal_;
  CanSender<::apollo::canbus::Lincoln> sender_;
  LincolnMessageManager msg_manager_;
  CanbusConf canbus_conf_;
  VehicleParameter params_;
};

TEST_F(LincolnControllerTest, Init) {
  ErrorCode ret = controller_.Init(params_, &sender_, &msg_manager_);
  EXPECT_EQ(ret, ErrorCode::OK);
}

TEST_F(LincolnControllerTest, SetDrivingMode) {
  Chassis chassis;
  chassis.set_driving_mode(Chassis::COMPLETE_AUTO_DRIVE);

  controller_.Init(params_, &sender_, &msg_manager_);
  controller_.set_driving_mode(chassis.driving_mode());
  EXPECT_EQ(controller_.driving_mode(), chassis.driving_mode());
  EXPECT_EQ(controller_.SetDrivingMode(chassis.driving_mode()), ErrorCode::OK);
}

TEST_F(LincolnControllerTest, Status) {
  controller_.Init(params_, &sender_, &msg_manager_);
  controller_.set_driving_mode(Chassis::COMPLETE_AUTO_DRIVE);
  EXPECT_EQ(controller_.Update(control_cmd_), ErrorCode::OK);
  controller_.SetHorn(control_cmd_);
  controller_.SetBeam(control_cmd_);
  controller_.SetTurningSignal(control_cmd_);
  EXPECT_FALSE(controller_.CheckChassisError());
  EXPECT_EQ(controller_.chassis_error_code(), Chassis::NO_ERROR);
}

TEST_F(LincolnControllerTest, UpdateDrivingMode) {
  controller_.Init(params_, &sender_, &msg_manager_);
  controller_.set_driving_mode(Chassis::COMPLETE_AUTO_DRIVE);
  EXPECT_EQ(controller_.SetDrivingMode(Chassis::COMPLETE_MANUAL),
            ErrorCode::OK);
  controller_.set_driving_mode(Chassis::COMPLETE_AUTO_DRIVE);
  EXPECT_EQ(controller_.SetDrivingMode(Chassis::AUTO_STEER_ONLY),
            ErrorCode::OK);
  controller_.set_driving_mode(Chassis::COMPLETE_AUTO_DRIVE);
  EXPECT_EQ(controller_.SetDrivingMode(Chassis::AUTO_SPEED_ONLY),
            ErrorCode::OK);
  EXPECT_EQ(controller_.SetDrivingMode(Chassis::COMPLETE_AUTO_DRIVE),
            ErrorCode::CANBUS_ERROR);
}

}  // namespace lincoln
}  // namespace canbus
}  // namespace apollo