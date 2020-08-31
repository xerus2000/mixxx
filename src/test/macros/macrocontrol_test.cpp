#include "engine/controls/macrocontrol.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "macro_test.h"
#include "track/track.h"

class MacroControlTest : public MacroControl, public testing::Test {
  public:
    MacroControlTest()
            : MacroControl(kChannelGroup, nullptr, 2) {
        EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
        trackLoaded(pLoadedTrack);
        EXPECT_EQ(getStatus(), MacroControl::Status::Empty);
        EXPECT_FALSE(getMacro()->isDirty());
        EXPECT_FALSE(pLoadedTrack->isDirty());
        EXPECT_FALSE(isRecording());
    }

    MOCK_METHOD1(seekExact, void(double));

    const TrackPointer pLoadedTrack = Track::newTemporary();
};

TEST_F(MacroControlTest, Dirty) {
    getMacro()->setLabel("hello");
    EXPECT_TRUE(getMacro()->isDirty());
    EXPECT_TRUE(pLoadedTrack->isDirty());
}

TEST_F(MacroControlTest, RecordSeek) {
    // Start recording
    slotRecord(1);
    EXPECT_TRUE(isRecording());
    EXPECT_EQ(getMacro()->getLabel(), "[Recording]");
    // Prepare recording
    int frameRate = 1'000;
    auto seek = [this, frameRate](double position) {
        notifySeek(position);
        setCurrentSample(position, 99'000, frameRate);
        process(0, position, 2);
    };
    seek(0);
    ASSERT_EQ(getStatus(), MacroControl::Status::Armed);

    // Initial jump
    double startPos = 1'100;
    slotJumpQueued();
    seek(startPos);
    // Jump kAction
    seek(kAction.getSourcePositionSample());
    slotJumpQueued();
    seek(kAction.getTargetPositionSample());

    // Stop recording
    EXPECT_CALL(*this, seekExact(startPos));
    slotRecord(0);
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    // Check recording result
    checkMacroAction(getMacro());
    EXPECT_EQ(getMacro()->getActions().first().getTargetPositionSample(), startPos);
    EXPECT_TRUE(pLoadedTrack->isDirty());
    // Check generated label
    EXPECT_EQ(startPos / mixxx::kEngineChannelCount / frameRate, 0.55);
    EXPECT_EQ(getMacro()->getLabel().toStdString(), "0.6");
    // Activate
    EXPECT_CALL(*this, seekExact(startPos));
    slotGotoPlay();
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    // Check status on eject
    trackLoaded(nullptr);
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
}

TEST_F(MacroControlTest, ControlObjects) {
    ControlProxy status(kChannelGroup, "macro_2_status");
#define ASSERT_STATUS(expected) ASSERT_EQ(MacroControl::Status(status.get()), expected);

    ControlProxy record(kChannelGroup, "macro_2_record");
    record.set(1);
    ASSERT_STATUS(MacroControl::Status::Armed);
    record.set(0);
    ASSERT_STATUS(MacroControl::Status::Empty);

    ControlProxy activate(kChannelGroup, "macro_2_activate");
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Armed);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Empty);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Armed);

    // Record
    ControlProxy(kChannelGroup, "macro_2_enable").set(0);
    ControlProxy(kChannelGroup, "macro_2_loop").set(1);
    EXPECT_TRUE(getMacro()->isLooped());
    slotJumpQueued();
    notifySeek(0);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Recorded);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Playing);

    // Restart
    EXPECT_CALL(*this, seekExact(0));
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Playing);

    ControlProxy play(kChannelGroup, "macro_2_play");
    play.set(0);
    ASSERT_STATUS(MacroControl::Status::Recorded);
}

TEST_F(MacroControlTest, LoadTrackAndPlay) {
    MacroAction jumpAction(40'000, 0);

    pLoadedTrack->setMacros({{2,
            std::make_shared<Macro>(
                    QList{kAction, jumpAction},
                    "test",
                    Macro::State())}});
    trackLoaded(pLoadedTrack);
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);

    slotActivate();
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    EXPECT_CALL(*this, seekExact(jumpAction.targetFrame));
    process(0, jumpAction.getSourcePositionSample(), 2);
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);

    // LOOP
    slotLoop(1);
    EXPECT_CALL(*this, seekExact(kAction.getTargetPositionSample()));
    slotActivate();
    slotActivate();
    // Jump
    EXPECT_CALL(*this, seekExact(jumpAction.getTargetPositionSample()));
    process(0, jumpAction.getSourcePositionSample(), 2);
    // Loop back
    EXPECT_CALL(*this, seekExact(kAction.getTargetPositionSample()));
    process(0, kAction.getSourcePositionSample(), 2);
    // Jump again
    EXPECT_CALL(*this, seekExact(jumpAction.getTargetPositionSample()));
    process(0, jumpAction.getSourcePositionSample(), 2);
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
}
