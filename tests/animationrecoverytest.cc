/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version.
 */

#include "geometry/AnimationRecovery.h"

#include <QTest>

using CoordinateMapper::AnimationRecoveryDecision;
using CoordinateMapper::MinimizeRetryDecision;
using CoordinateMapper::MissingMinimizeTargetAction;
using CoordinateMapper::decideAnimationRecovery;
using CoordinateMapper::decideMinimizeRetry;
using CoordinateMapper::decideMissingMinimizeTarget;

class AnimationRecoveryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void dropsRecoveryWhenWindowHasBeenDeleted();
    void dropsRecoveryWhenFullscreenEffectTakesOver();
    void retriesWhileDisconnectedOutputIsSettling();
    void resumesWhenTargetReturnsAfterOutputReconnects();
    void retriesWhenTaskManagerPanelDisappears();
    void dropsWhenPanelRemainsUnavailableAfterSettling();
    void keepsRestoredAnimationPausedUntilNewTargetAppears();
    void dropsRestoredAnimationWhenTargetNeverReturns();
    void dropsAfterRetryLimitIsReached();
    void startUnpausesAndReversesRunningTimeline();
    void startUnpausesFreshTimeline();
    void waitsForTargetWhenNoAnimationExists();
    void cancelsActiveAnimationWhenMinimizeTargetIsMissing();
    void dropsMinimizeRetryWhenWindowHasBeenDeleted();
    void dropsMinimizeRetryWhenWindowWasRestored();
    void dropsMinimizeRetryWhenFullscreenTakesOver();
    void retriesWhileTaskButtonsAreStillSettling();
    void animatesWhenTargetAppearsOnLastRetry();
    void dropsWhenTaskButtonsAreSettledWithoutTarget();
    void dropsWhenMinimizeRetryLimitIsReached();
    void dropsWhenMinimizeRetryLimitIsDisabled();
};

void AnimationRecoveryTest::dropsRecoveryWhenWindowHasBeenDeleted()
{
    QCOMPARE(decideAnimationRecovery(false, false, false, false, 1, 10),
        AnimationRecoveryDecision::Drop);
}

void AnimationRecoveryTest::dropsRecoveryWhenFullscreenEffectTakesOver()
{
    QCOMPARE(decideAnimationRecovery(true, true, false, false, 1, 10),
        AnimationRecoveryDecision::Drop);
}

void AnimationRecoveryTest::retriesWhileDisconnectedOutputIsSettling()
{
    QCOMPARE(decideAnimationRecovery(true, false, false, false, 1, 10),
        AnimationRecoveryDecision::Retry);
}

void AnimationRecoveryTest::resumesWhenTargetReturnsAfterOutputReconnects()
{
    QCOMPARE(decideAnimationRecovery(true, false, true, false, 2, 10),
        AnimationRecoveryDecision::Resume);
}

void AnimationRecoveryTest::retriesWhenTaskManagerPanelDisappears()
{
    QCOMPARE(decideAnimationRecovery(true, false, false, false, 1, 10),
        AnimationRecoveryDecision::Retry);
}

void AnimationRecoveryTest::dropsWhenPanelRemainsUnavailableAfterSettling()
{
    QCOMPARE(decideAnimationRecovery(true, false, false, true, 1, 10),
        AnimationRecoveryDecision::Drop);
}

void AnimationRecoveryTest::keepsRestoredAnimationPausedUntilNewTargetAppears()
{
    // A restored window must not be dropped just because it is no longer
    // minimized; the new task-manager target may still be in the QML layout.
    QCOMPARE(decideAnimationRecovery(true, false, false, false, 1, 10),
        AnimationRecoveryDecision::Retry);
    QCOMPARE(decideAnimationRecovery(true, false, true, false, 2, 10),
        AnimationRecoveryDecision::Resume);
}

void AnimationRecoveryTest::dropsRestoredAnimationWhenTargetNeverReturns()
{
    QCOMPARE(decideAnimationRecovery(true, false, false, true, 1, 10),
        AnimationRecoveryDecision::Drop);
}

void AnimationRecoveryTest::dropsAfterRetryLimitIsReached()
{
    QCOMPARE(decideAnimationRecovery(true, false, false, false, 10, 10),
        AnimationRecoveryDecision::Drop);
}

void AnimationRecoveryTest::startUnpausesAndReversesRunningTimeline()
{
    const auto state = CoordinateMapper::prepareAnimationStart(
        {true, false, false}, true);

    QVERIFY(!state.paused);
    QVERIFY(!state.done);
    QVERIFY(state.reverseTimeline);
}

void AnimationRecoveryTest::startUnpausesFreshTimeline()
{
    const auto state = CoordinateMapper::prepareAnimationStart(
        {true, true, false}, false);

    QVERIFY(!state.paused);
    QVERIFY(state.done);
    QVERIFY(!state.reverseTimeline);
}

void AnimationRecoveryTest::waitsForTargetWhenNoAnimationExists()
{
    QCOMPARE(decideMissingMinimizeTarget(false),
        MissingMinimizeTargetAction::WaitForTarget);
}

void AnimationRecoveryTest::cancelsActiveAnimationWhenMinimizeTargetIsMissing()
{
    QCOMPARE(decideMissingMinimizeTarget(true),
        MissingMinimizeTargetAction::CancelExistingAnimation);
}


void AnimationRecoveryTest::dropsMinimizeRetryWhenWindowHasBeenDeleted()
{
    QCOMPARE(decideMinimizeRetry(false, true, false, false, false, 1, 6, 1),
        MinimizeRetryDecision::Drop);
}

void AnimationRecoveryTest::dropsMinimizeRetryWhenWindowWasRestored()
{
    QCOMPARE(decideMinimizeRetry(true, false, false, false, false, 0, 6, 1),
        MinimizeRetryDecision::Drop);
}

void AnimationRecoveryTest::dropsMinimizeRetryWhenFullscreenTakesOver()
{
    QCOMPARE(decideMinimizeRetry(true, true, true, false, false, 1, 6, 1),
        MinimizeRetryDecision::Drop);
}

void AnimationRecoveryTest::retriesWhileTaskButtonsAreStillSettling()
{
    QCOMPARE(decideMinimizeRetry(true, true, false, false, false, 5, 6, 1),
        MinimizeRetryDecision::Retry);
}

void AnimationRecoveryTest::animatesWhenTargetAppearsOnLastRetry()
{
    QCOMPARE(decideMinimizeRetry(true, true, false, true, true, 6, 6, 1),
        MinimizeRetryDecision::Animate);
}

void AnimationRecoveryTest::dropsWhenTaskButtonsAreSettledWithoutTarget()
{
    QCOMPARE(decideMinimizeRetry(true, true, false, false, true, 1, 6, 1),
        MinimizeRetryDecision::Drop);
}

void AnimationRecoveryTest::dropsWhenMinimizeRetryLimitIsReached()
{
    QCOMPARE(decideMinimizeRetry(true, true, false, false, false, 6, 6, 1),
        MinimizeRetryDecision::Drop);
}

void AnimationRecoveryTest::dropsWhenMinimizeRetryLimitIsDisabled()
{
    QCOMPARE(decideMinimizeRetry(true, true, false, false, false, 1, 0, 1),
        MinimizeRetryDecision::Drop);
}

QTEST_GUILESS_MAIN(AnimationRecoveryTest)

#include "animationrecoverytest.moc"
