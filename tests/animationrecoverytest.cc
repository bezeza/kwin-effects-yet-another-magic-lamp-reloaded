/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version.
 */

#include "geometry/AnimationRecovery.h"

#include <QTest>

using CoordinateMapper::AnimationRecoveryDecision;
using CoordinateMapper::decideAnimationRecovery;

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

QTEST_GUILESS_MAIN(AnimationRecoveryTest)

#include "animationrecoverytest.moc"
