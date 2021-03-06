// ----------------- BEGIN LICENSE BLOCK ---------------------------------
//
// Copyright (c) 2018-2019 Intel Corporation
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
//    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//    OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
//    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//    POSSIBILITY OF SUCH DAMAGE.
//
// ----------------- END LICENSE BLOCK -----------------------------------

#include "TestSupport.hpp"
#include "ad_rss/core/RssSituationChecking.hpp"

namespace ad_rss {
namespace situation {

class RssSituationCheckingTestsIntersectionNoPriority : public testing::Test
{
protected:
  VehicleState leadingVehicle;
  VehicleState followingVehicle;
  Situation situation;
  state::ResponseState responseState;
};

TEST_F(RssSituationCheckingTestsIntersectionNoPriority, ego_following_no_overlap)
{
  for (auto situationType : {SituationType::IntersectionSamePriority, SituationType::IntersectionObjectHasPriority})
  {
    core::RssSituationChecking situationChecking;
    situation.egoVehicleState = createVehicleStateForLongitudinalMotion(120);
    situation.egoVehicleState.distanceToEnterIntersection = Distance(15);
    situation.egoVehicleState.distanceToLeaveIntersection = Distance(15);

    situation.egoVehicleState.dynamics.alphaLon.accelMax = Acceleration(2.);
    situation.egoVehicleState.dynamics.alphaLon.brakeMin = Acceleration(4.);

    situation.otherVehicleState = createVehicleStateForLongitudinalMotion(10);
    situation.otherVehicleState.dynamics.alphaLon.accelMax = Acceleration(2.);
    situation.otherVehicleState.dynamics.alphaLon.brakeMin = Acceleration(4.);
    situation.otherVehicleState.distanceToEnterIntersection = Distance(16);
    situation.otherVehicleState.distanceToLeaveIntersection = Distance(16);

    situation.relativePosition = createRelativeLongitudinalPosition(LongitudinalRelativePosition::AtBack, Distance(1.));
    situation.situationType = situationType;
    situation.timeIndex = 1u;
    if (situationType == SituationType::IntersectionObjectHasPriority)
    {
      situation.otherVehicleState.hasPriority = true;
    }
    else
    {
      situation.otherVehicleState.hasPriority = false;
    }

    EXPECT_TRUE(situationChecking.checkSituationInputRangeChecked(situation, true, responseState));
    EXPECT_EQ(responseState.longitudinalState,
              TestSupport::stateWithInformation(cTestSupport.cLongitudinalSafe, situation));
    EXPECT_EQ(responseState.lateralStateLeft, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
    EXPECT_EQ(responseState.lateralStateRight, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));

    // next situation we have overlap
    situation.timeIndex++;
    situation.egoVehicleState.velocity.speedLon = Speed(10);

    EXPECT_TRUE(situationChecking.checkSituationInputRangeChecked(situation, true, responseState));
    if (situationType == SituationType::IntersectionObjectHasPriority)
    {
      EXPECT_EQ(responseState.longitudinalState,
                TestSupport::stateWithInformation(cTestSupport.cLongitudinalBrakeMin, situation));
    }
    else
    {
      EXPECT_EQ(responseState.longitudinalState,
                TestSupport::stateWithInformation(cTestSupport.cLongitudinalSafe, situation));
    }
    EXPECT_EQ(responseState.lateralStateLeft, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
    EXPECT_EQ(responseState.lateralStateRight, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
  }
}

TEST_F(RssSituationCheckingTestsIntersectionNoPriority, 50kmh_safe_distance_ego_following)
{
  for (auto situationType : {SituationType::IntersectionSamePriority, SituationType::IntersectionObjectHasPriority})
  {
    core::RssSituationChecking situationChecking;
    leadingVehicle = createVehicleStateForLongitudinalMotion(120);
    leadingVehicle.distanceToEnterIntersection = Distance(2);
    leadingVehicle.distanceToLeaveIntersection = Distance(2);
    leadingVehicle.dynamics.alphaLon.accelMax = Acceleration(2.);
    leadingVehicle.dynamics.alphaLon.brakeMin = Acceleration(4.);
    if (situationType == SituationType::IntersectionObjectHasPriority)
    {
      leadingVehicle.hasPriority = true;
    }
    else
    {
      leadingVehicle.hasPriority = false;
    }

    followingVehicle = createVehicleStateForLongitudinalMotion(30);
    followingVehicle.dynamics.alphaLon.accelMax = Acceleration(2.);
    followingVehicle.dynamics.alphaLon.brakeMin = Acceleration(4.);
    followingVehicle.distanceToEnterIntersection = Distance(12.);
    followingVehicle.distanceToLeaveIntersection = Distance(12.);

    situation.egoVehicleState = followingVehicle;
    situation.otherVehicleState = leadingVehicle;
    situation.relativePosition
      = createRelativeLongitudinalPosition(LongitudinalRelativePosition::AtBack, Distance(10.));
    situation.situationType = situationType;
    situation.timeIndex = 1u;
    if (situationType == SituationType::IntersectionObjectHasPriority)
    {
      situation.otherVehicleState.hasPriority = true;
    }
    else
    {
      situation.otherVehicleState.hasPriority = false;
    }

    ASSERT_TRUE(situationChecking.checkSituationInputRangeChecked(situation, true, responseState));
    EXPECT_EQ(responseState.longitudinalState,
              TestSupport::stateWithInformation(cTestSupport.cLongitudinalSafe, situation));
    EXPECT_EQ(responseState.lateralStateLeft, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
    EXPECT_EQ(responseState.lateralStateRight, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
  }
}

TEST_F(RssSituationCheckingTestsIntersectionNoPriority, 50kmh_safe_distance_ego_leading)
{
  for (auto situationType : {SituationType::IntersectionSamePriority, SituationType::IntersectionObjectHasPriority})
  {
    core::RssSituationChecking situationChecking;
    leadingVehicle = createVehicleStateForLongitudinalMotion(50);
    leadingVehicle.distanceToEnterIntersection = Distance(10.);
    leadingVehicle.distanceToLeaveIntersection = Distance(10.);
    followingVehicle = createVehicleStateForLongitudinalMotion(50);
    followingVehicle.dynamics.alphaLon.accelMax = Acceleration(2.);
    followingVehicle.dynamics.alphaLon.brakeMin = Acceleration(4.);
    followingVehicle.distanceToEnterIntersection = Distance(70.);
    followingVehicle.distanceToLeaveIntersection = Distance(70.);

    situation.egoVehicleState = leadingVehicle;
    situation.otherVehicleState = followingVehicle;
    situation.relativePosition
      = createRelativeLongitudinalPosition(LongitudinalRelativePosition::InFront, Distance(60.));
    situation.situationType = situationType;
    situation.timeIndex++;
    if (situationType == SituationType::IntersectionObjectHasPriority)
    {
      situation.otherVehicleState.hasPriority = true;
    }
    else
    {
      situation.otherVehicleState.hasPriority = false;
    }

    ASSERT_TRUE(situationChecking.checkSituationInputRangeChecked(situation, true, responseState));
    ASSERT_TRUE(responseState.longitudinalState.isSafe);
    EXPECT_EQ(responseState.longitudinalState,
              TestSupport::stateWithInformation(cTestSupport.cLongitudinalSafe, situation));
    EXPECT_EQ(responseState.lateralStateLeft, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
    EXPECT_EQ(responseState.lateralStateRight, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
  }
}

TEST_F(RssSituationCheckingTestsIntersectionNoPriority, 50km_h_stop_before_intersection)
{
  for (auto situationType : {SituationType::IntersectionSamePriority, SituationType::IntersectionObjectHasPriority})
  {
    core::RssSituationChecking situationChecking;
    leadingVehicle = createVehicleStateForLongitudinalMotion(50);
    leadingVehicle.distanceToEnterIntersection = Distance(80.);
    leadingVehicle.distanceToLeaveIntersection = Distance(80.);
    leadingVehicle.dynamics.alphaLon.accelMax = Acceleration(2.);
    leadingVehicle.dynamics.alphaLon.brakeMin = Acceleration(4.);
    followingVehicle = createVehicleStateForLongitudinalMotion(50);
    followingVehicle.dynamics.alphaLon.accelMax = Acceleration(2.);
    followingVehicle.dynamics.alphaLon.brakeMin = Acceleration(4.);
    followingVehicle.distanceToEnterIntersection = Distance(110.);
    followingVehicle.distanceToLeaveIntersection = Distance(110.);

    situation.egoVehicleState = leadingVehicle;
    situation.otherVehicleState = followingVehicle;
    situation.relativePosition
      = createRelativeLongitudinalPosition(LongitudinalRelativePosition::InFront, Distance(30.));
    situation.situationType = situationType;
    situation.timeIndex = 1u;
    if (situationType == SituationType::IntersectionObjectHasPriority)
    {
      situation.otherVehicleState.hasPriority = true;
    }
    else
    {
      situation.otherVehicleState.hasPriority = false;
    }

    // both vehicles can stop safely
    ASSERT_TRUE(situationChecking.checkSituationInputRangeChecked(situation, true, responseState));
    ASSERT_TRUE(responseState.longitudinalState.isSafe);
    ASSERT_EQ(responseState.longitudinalState,
              TestSupport::stateWithInformation(cTestSupport.cLongitudinalSafe, situation));
    EXPECT_EQ(responseState.lateralStateLeft, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
    EXPECT_EQ(responseState.lateralStateRight, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));

    // ego vehicle cannot stop safely anymore
    // but other vehicle still
    situation.timeIndex++;

    situation.egoVehicleState.distanceToEnterIntersection = Distance(70);
    situation.egoVehicleState.distanceToLeaveIntersection = Distance(70);
    situation.otherVehicleState.distanceToEnterIntersection = Distance(100);
    situation.otherVehicleState.distanceToLeaveIntersection = Distance(100);

    ASSERT_TRUE(situationChecking.checkSituationInputRangeChecked(situation, true, responseState));
    if (situation.otherVehicleState.hasPriority)
    {
      ASSERT_FALSE(responseState.longitudinalState.isSafe);
      ASSERT_EQ(responseState.longitudinalState,
                TestSupport::stateWithInformation(cTestSupport.cLongitudinalBrakeMin, situation));
    }
    else
    {
      ASSERT_TRUE(responseState.longitudinalState.isSafe);
      ASSERT_EQ(responseState.longitudinalState,
                TestSupport::stateWithInformation(cTestSupport.cLongitudinalSafe, situation));
    }
    EXPECT_EQ(responseState.lateralStateLeft, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
    EXPECT_EQ(responseState.lateralStateRight, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));

    // both cannot stop safely anymore
    situation.timeIndex++;

    situation.egoVehicleState.distanceToEnterIntersection = Distance(70.);
    situation.egoVehicleState.distanceToLeaveIntersection = Distance(70.);
    situation.otherVehicleState.distanceToEnterIntersection = Distance(70.);
    situation.otherVehicleState.distanceToLeaveIntersection = Distance(70.);

    ASSERT_TRUE(situationChecking.checkSituationInputRangeChecked(situation, true, responseState));
    ASSERT_FALSE(responseState.longitudinalState.isSafe);
    ASSERT_EQ(responseState.longitudinalState,
              TestSupport::stateWithInformation(cTestSupport.cLongitudinalBrakeMin, situation));
    EXPECT_EQ(responseState.lateralStateLeft, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
    EXPECT_EQ(responseState.lateralStateRight, TestSupport::stateWithInformation(cTestSupport.cLateralNone, situation));
  }
}

} // namespace situation
} // namespace ad_rss
