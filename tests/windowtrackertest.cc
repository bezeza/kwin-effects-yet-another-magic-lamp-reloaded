/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/WindowTracker.h"

#include <QTest>

class WindowTrackerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void remainsUnsettledUntilBothTimersExpire();
    void settlesAtExactThreshold();
    void zeroThresholdIsImmediatelySettled();
    void rejectsNegativeThreshold();
    void detectsOutputSwitch();
    void detectsOutputRemovalAndReattachment();
    void ignoresUnchangedOutputIdentity();
};

void WindowTrackerTest::remainsUnsettledUntilBothTimersExpire()
{
    QVERIFY(!CoordinateMapper::WindowTracker::hasSettled(1000, 999, 1000));
    QVERIFY(!CoordinateMapper::WindowTracker::hasSettled(999, 1000, 1000));
}

void WindowTrackerTest::settlesAtExactThreshold()
{
    QVERIFY(CoordinateMapper::WindowTracker::hasSettled(1000, 1000, 1000));
    QVERIFY(CoordinateMapper::WindowTracker::hasSettled(1500, 1200, 1000));
}

void WindowTrackerTest::zeroThresholdIsImmediatelySettled()
{
    QVERIFY(CoordinateMapper::WindowTracker::hasSettled(0, 0, 0));
}

void WindowTrackerTest::rejectsNegativeThreshold()
{
    QVERIFY(!CoordinateMapper::WindowTracker::hasSettled(5000, 5000, -1));
}

void WindowTrackerTest::detectsOutputSwitch()
{
    int firstOutput = 0;
    int secondOutput = 0;

    QVERIFY(CoordinateMapper::WindowTracker::outputIdentityChanged(&firstOutput, &secondOutput));
}

void WindowTrackerTest::detectsOutputRemovalAndReattachment()
{
    int output = 0;

    QVERIFY(CoordinateMapper::WindowTracker::outputIdentityChanged(&output, nullptr));
    QVERIFY(CoordinateMapper::WindowTracker::outputIdentityChanged(nullptr, &output));
}

void WindowTrackerTest::ignoresUnchangedOutputIdentity()
{
    int output = 0;

    QVERIFY(!CoordinateMapper::WindowTracker::outputIdentityChanged(&output, &output));
    QVERIFY(!CoordinateMapper::WindowTracker::outputIdentityChanged(nullptr, nullptr));
}

QTEST_GUILESS_MAIN(WindowTrackerTest)

#include "windowtrackertest.moc"
