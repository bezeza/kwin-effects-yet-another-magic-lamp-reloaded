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
    void estimatesGridWork_data();
    void estimatesGridWork();
    void gridWorkScalesWithWindowArea();
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
    QTest::addColumn<qint64>("duration");
    QTest::addColumn<int>("expectedDuration");
    QTest::addColumn<unsigned int>("bump");
    QTest::addColumn<int>("expectedBump");

    QTest::newRow("negative-base-duration") << qint64(-1) << 1 << 0u << 0;
    QTest::newRow("zero-duration") << qint64(0) << 1 << 0u << 0;
    QTest::newRow("defaults") << qint64(300) << 300 << 0u << 0;
    QTest::newRow("ui-maximum") << qint64(9999) << 9999 << 99u << 99;
    QTest::newRow("oversized-kwinrc") << std::numeric_limits<qint64>::max()
                                      << 9999 << std::numeric_limits<unsigned int>::max() << 99;
}

void AnimationLimitsTest::clampsAnimationLimits()
{
    QFETCH(qint64, duration);
    QFETCH(int, expectedDuration);
    QFETCH(unsigned int, bump);
    QFETCH(int, expectedBump);

    QCOMPARE(CoordinateMapper::boundedAnimationDuration(duration), expectedDuration);
    QCOMPARE(CoordinateMapper::boundedBumpDistance(bump), expectedBump);
}


void AnimationLimitsTest::estimatesGridWork_data()
{
    QTest::addColumn<quint64>("width");
    QTest::addColumn<quint64>("height");
    QTest::addColumn<unsigned int>("cellSize");
    QTest::addColumn<quint64>("expected");

    QTest::newRow("1080p-default") << quint64(1920) << quint64(1080) << 30u << quint64(2304);
    QTest::newRow("4k-default") << quint64(3840) << quint64(2160) << 30u << quint64(9216);
    QTest::newRow("4k-high-detail") << quint64(3840) << quint64(2160) << 16u << quint64(32400);
    QTest::newRow("4k-coarse") << quint64(3840) << quint64(2160) << 99u << quint64(858);
    QTest::newRow("8k-default") << quint64(7680) << quint64(4320) << 30u << quint64(36864);
}

void AnimationLimitsTest::estimatesGridWork()
{
    QFETCH(quint64, width);
    QFETCH(quint64, height);
    QFETCH(unsigned int, cellSize);
    QFETCH(quint64, expected);

    QCOMPARE(CoordinateMapper::estimatedGridQuadCount(width, height, cellSize), expected);
}

void AnimationLimitsTest::gridWorkScalesWithWindowArea()
{
    const quint64 fullHd = CoordinateMapper::estimatedGridQuadCount(1920, 1080, 30);
    const quint64 fourK = CoordinateMapper::estimatedGridQuadCount(3840, 2160, 30);
    const quint64 fourKCoarse = CoordinateMapper::estimatedGridQuadCount(3840, 2160, 60);

    QCOMPARE(fourK, fullHd * 4);
    QCOMPARE(fourKCoarse, fullHd);
    QCOMPARE(CoordinateMapper::estimatedGridQuadCount(0, 2160, 30), quint64(0));
    QCOMPARE(CoordinateMapper::estimatedGridQuadCount(3840, 2160, 0), quint64(0));
    QCOMPARE(CoordinateMapper::estimatedGridQuadCount(
                 std::numeric_limits<quint64>::max(),
                 std::numeric_limits<quint64>::max(), 1),
             std::numeric_limits<quint64>::max());
}

QTEST_GUILESS_MAIN(AnimationLimitsTest)

#include "animationlimitstest.moc"
