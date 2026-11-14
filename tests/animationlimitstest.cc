/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/AnimationLimits.h"

#include <QTest>

#include <limits>

class AnimationLimitsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void clampsGridCellSize_data();
    void clampsGridCellSize();
    void clampsAnimationLimits_data();
    void clampsAnimationLimits();
};

void AnimationLimitsTest::clampsGridCellSize_data()
{
    QTest::addColumn<unsigned int>("configured");
    QTest::addColumn<int>("expected");

    QTest::newRow("zero-cannot-reach-KWin") << 0u << 16;
    QTest::newRow("one-is-too-dense") << 1u << 16;
    QTest::newRow("below-minimum") << 15u << 16;
    QTest::newRow("minimum") << 16u << 16;
    QTest::newRow("default") << 30u << 30;
    QTest::newRow("maximum") << 99u << 99;
    QTest::newRow("above-maximum") << 100u << 99;
    QTest::newRow("unsigned-overflow-value") << std::numeric_limits<unsigned int>::max() << 99;
}

void AnimationLimitsTest::clampsGridCellSize()
{
    QFETCH(unsigned int, configured);
    QFETCH(int, expected);

    QCOMPARE(CoordinateMapper::boundedGridCellSize(configured), expected);
}

void AnimationLimitsTest::clampsAnimationLimits_data()
{
    QTest::addColumn<unsigned int>("duration");
    QTest::addColumn<int>("expectedDuration");
    QTest::addColumn<unsigned int>("bump");
    QTest::addColumn<int>("expectedBump");

    QTest::newRow("zero-duration") << 0u << 1 << 0u << 0;
    QTest::newRow("defaults") << 300u << 300 << 0u << 0;
    QTest::newRow("ui-maximum") << 9999u << 9999 << 99u << 99;
    QTest::newRow("oversized-kwinrc") << std::numeric_limits<unsigned int>::max()
                                      << 9999 << std::numeric_limits<unsigned int>::max() << 99;
    QTest::newRow("negative-base-duration") << 0u << 1 << 0u << 0;
}

void AnimationLimitsTest::clampsAnimationLimits()
{
    QFETCH(unsigned int, duration);
    QFETCH(int, expectedDuration);
    QFETCH(unsigned int, bump);
    QFETCH(int, expectedBump);

    QCOMPARE(CoordinateMapper::boundedAnimationDuration(duration), expectedDuration);
    QCOMPARE(CoordinateMapper::boundedBumpDistance(bump), expectedBump);
}

QTEST_GUILESS_MAIN(AnimationLimitsTest)

#include "animationlimitstest.moc"
