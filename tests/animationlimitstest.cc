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

QTEST_GUILESS_MAIN(AnimationLimitsTest)

#include "animationlimitstest.moc"
