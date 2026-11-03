/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Numbers marked "observed" were logged by a nested KWin 6.8.2 session with
// two 1024x768 outputs (horizontal: B right of A; vertical: B below A).

#include "geometry/CoordinateMapper.h"

#include <QTest>

class CoordinateMapperTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void surfaceToGlobal_data();
    void surfaceToGlobal();
    void verticalStackFoolsNearestPanelRule();
    void horizontalNearestPanelRuleMatchesOutput();
};

void CoordinateMapperTest::surfaceToGlobal_data()
{
    QTest::addColumn<QRect>("surfaceLocal");
    QTest::addColumn<QPointF>("panelPos");
    QTest::addColumn<QRectF>("global");

    // observed
    QTest::newRow("horizontal, panel on B") << QRect(164, 8, 38, 30) << QPointF(1024, 722) << QRectF(1188, 730, 38, 30);
    QTest::newRow("vertical, panel on B") << QRect(164, 8, 38, 30) << QPointF(0, 1490) << QRectF(164, 1498, 38, 30);
    QTest::newRow("panel on A") << QRect(274, 8, 54, 46) << QPointF(0, 706) << QRectF(274, 714, 54, 46);
    // KWin 6.6 does not normalize output positions to 0,0
    QTest::newRow("negative output origin") << QRect(300, 4, 40, 32) << QPointF(-1920, 1040) << QRectF(-1620, 1044, 40, 32);
    // 2560 px at scale 1.5 is 1706.67 logical px wide
    QTest::newRow("fractional panel position") << QRect(10, 2, 40, 40) << QPointF(1706.6666, 1398) << QRectF(1716.6666, 1400, 40, 40);
}

void CoordinateMapperTest::surfaceToGlobal()
{
    QFETCH(QRect, surfaceLocal);
    QFETCH(QPointF, panelPos);
    QFETCH(QRectF, global);

    const QRectF result = CoordinateMapper::surfaceToGlobal(surfaceLocal, panelPos);
    QCOMPARE(result.topLeft(), global.topLeft());
    QCOMPARE(result.size(), global.size());
}

void CoordinateMapperTest::verticalStackFoolsNearestPanelRule()
{
    // observed: window on the lower output, close to the shared edge. KWin's
    // nearest-panel rule picks the upper output's panel; this is why the
    // effect filters panels by the window's output first.
    const QRectF window(50, 818, 400, 300);
    const QRectF panelOnA(0, 706, 1024, 62);
    const QRectF panelOnB(0, 1490, 1024, 46);

    QCOMPARE(CoordinateMapper::centreDistance(panelOnA, window.center()), 493.0);
    QCOMPARE(CoordinateMapper::centreDistance(panelOnB, window.center()), 807.0);
}

void CoordinateMapperTest::horizontalNearestPanelRuleMatchesOutput()
{
    // observed: side by side, the nearest panel is the window's own.
    const QRectF window(1074, 50, 400, 300);
    const QRectF panelOnA(0, 706, 1024, 62);
    const QRectF panelOnB(1024, 722, 1024, 46);

    QCOMPARE(CoordinateMapper::centreDistance(panelOnA, window.center()), 1299.0);
    QCOMPARE(CoordinateMapper::centreDistance(panelOnB, window.center()), 807.0);
}

QTEST_GUILESS_MAIN(CoordinateMapperTest)

#include "coordinatemappertest.moc"
