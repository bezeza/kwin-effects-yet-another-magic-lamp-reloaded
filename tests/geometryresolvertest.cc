/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/GeometryResolver.h"

#include <QTest>

#include <limits>

using CoordinateMapper::PanelButtonCandidate;
using CoordinateMapper::nearestButtonOnOutput;

class GeometryResolverTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void selectsNearestButtonOnTargetOutput();
    void preservesFirstCandidateOnDistanceTie();
    void ignoresCloserButtonOnAnotherOutput();
    void returnsNoButtonWhenOnlyOtherOutputsExist();
    void ignoresInvalidButtonGeometry();
    void ignoresInvalidPanelGeometry();
    void ignoresOverflowingButtonGeometry();
    void rejectsNonFiniteWindowCenter();
    void ignoresClosingPanelCandidate();
    void selectsAvailableReplacementAfterPanelCloses();
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


void GeometryResolverTest::preservesFirstCandidateOnDistanceTie()
{
    const QRectF firstPanel(0.0, 0.0, 100.0, 100.0);
    const QRectF secondPanel(200.0, 0.0, 100.0, 100.0);
    const QRectF firstButton(10.0, 10.0, 20.0, 20.0);
    const QRectF secondButton(210.0, 10.0, 20.0, 20.0);

    const QList<PanelButtonCandidate> candidates{
        {true, firstPanel, firstButton},
        {true, secondPanel, secondButton},
    };

    const auto result = nearestButtonOnOutput(candidates, QPointF(150.0, 50.0));
    QVERIFY(result);
    QCOMPARE(*result, firstButton);
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

void GeometryResolverTest::ignoresOverflowingButtonGeometry()
{
    const qreal maximum = std::numeric_limits<qreal>::max();
    const QList<PanelButtonCandidate> candidates{
        {true, QRectF(0.0, 700.0, 1024.0, 68.0), QRectF(maximum, 710.0, maximum, 40.0)},
    };

    QVERIFY(!nearestButtonOnOutput(candidates, QPointF(500.0, 740.0)));
}

void GeometryResolverTest::rejectsNonFiniteWindowCenter()
{
    const QList<PanelButtonCandidate> candidates{
        {true, QRectF(0.0, 700.0, 1024.0, 68.0), QRectF(400.0, 710.0, 40.0, 40.0)},
    };

    const QPointF invalidCenter(std::numeric_limits<qreal>::quiet_NaN(), 740.0);
    QVERIFY(!nearestButtonOnOutput(candidates, invalidCenter));
}

void GeometryResolverTest::ignoresClosingPanelCandidate()
{
    const QList<PanelButtonCandidate> candidates{
        {true, QRectF(0.0, 700.0, 1024.0, 68.0), QRectF(400.0, 710.0, 40.0, 40.0), false},
    };

    QVERIFY(!nearestButtonOnOutput(candidates, QPointF(500.0, 740.0)));
}

void GeometryResolverTest::selectsAvailableReplacementAfterPanelCloses()
{
    const QRectF oldButton(400.0, 710.0, 40.0, 40.0);
    const QRectF replacementButton(500.0, 710.0, 40.0, 40.0);
    const QList<PanelButtonCandidate> candidates{
        {true, QRectF(0.0, 700.0, 1024.0, 68.0), oldButton, false},
        {true, QRectF(0.0, 700.0, 1024.0, 68.0), replacementButton, true},
    };

    const auto result = nearestButtonOnOutput(candidates, QPointF(500.0, 740.0));
    QVERIFY(result);
    QCOMPARE(*result, replacementButton);
}

QTEST_GUILESS_MAIN(GeometryResolverTest)

#include "geometryresolvertest.moc"
