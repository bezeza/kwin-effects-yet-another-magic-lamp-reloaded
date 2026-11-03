/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/GeometryResolver.h"

#include <QTest>

using CoordinateMapper::PanelButtonCandidate;
using CoordinateMapper::nearestButtonOnOutput;

class GeometryResolverTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void selectsNearestButtonOnTargetOutput();
    void ignoresCloserButtonOnAnotherOutput();
    void returnsNoButtonWhenOnlyOtherOutputsExist();
    void ignoresInvalidButtonGeometry();
    void ignoresInvalidPanelGeometry();
    void rejectsNonFiniteWindowCenter();
};

void GeometryResolverTest::selectsNearestButtonOnTargetOutput()
{
    const QRectF nearPanel(0.0, 700.0, 1024.0, 68.0);
    const QRectF farPanel(0.0, 0.0, 1024.0, 68.0);
    const QRectF nearButton(400.0, 710.0, 40.0, 40.0);
    const QRectF farButton(400.0, 10.0, 40.0, 40.0);

    const QList<PanelButtonCandidate> candidates{
        {true, farPanel, farButton},
        {true, nearPanel, nearButton},
    };

    const auto result = nearestButtonOnOutput(candidates, QPointF(500.0, 740.0));
    QVERIFY(result);
    QCOMPARE(*result, nearButton);
}

void GeometryResolverTest::ignoresCloserButtonOnAnotherOutput()
{
    const QRectF closerPanel(1400.0, 700.0, 1024.0, 68.0);
    const QRectF targetPanel(0.0, 700.0, 1024.0, 68.0);
    const QRectF otherButton(1800.0, 710.0, 40.0, 40.0);
    const QRectF targetButton(400.0, 710.0, 40.0, 40.0);

    const QList<PanelButtonCandidate> candidates{
        {false, closerPanel, otherButton},
        {true, targetPanel, targetButton},
    };

    const auto result = nearestButtonOnOutput(candidates, QPointF(1500.0, 740.0));
    QVERIFY(result);
    QCOMPARE(*result, targetButton);
}

void GeometryResolverTest::returnsNoButtonWhenOnlyOtherOutputsExist()
{
    const QList<PanelButtonCandidate> candidates{
        {false, QRectF(0.0, 700.0, 1024.0, 68.0), QRectF(400.0, 710.0, 40.0, 40.0)},
    };

    QVERIFY(!nearestButtonOnOutput(candidates, QPointF(500.0, 740.0)));
}

void GeometryResolverTest::ignoresInvalidButtonGeometry()
{
    const QList<PanelButtonCandidate> candidates{
        {true, QRectF(0.0, 700.0, 1024.0, 68.0), QRectF(400.0, 710.0, 0.0, 40.0)},
    };

    QVERIFY(!nearestButtonOnOutput(candidates, QPointF(500.0, 740.0)));
}

void GeometryResolverTest::ignoresInvalidPanelGeometry()
{
    const QList<PanelButtonCandidate> candidates{
        {true, QRectF(0.0, 700.0, 0.0, 68.0), QRectF(400.0, 710.0, 40.0, 40.0)},
    };

    QVERIFY(!nearestButtonOnOutput(candidates, QPointF(500.0, 740.0)));
}

void GeometryResolverTest::rejectsNonFiniteWindowCenter()
{
    const QList<PanelButtonCandidate> candidates{
        {true, QRectF(0.0, 700.0, 1024.0, 68.0), QRectF(400.0, 710.0, 40.0, 40.0)},
    };

    QVERIFY(!nearestButtonOnOutput(candidates, QPointF(std::numeric_limits<qreal>::quiet_NaN(), 740.0)));
}

QTEST_GUILESS_MAIN(GeometryResolverTest)

#include "geometryresolvertest.moc"
