/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/ScaleAdapter.h"

#include <QTest>

#include <limits>

using CoordinateMapper::ScaleAdapter;

class ScaleAdapterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void mapsFractionalScale();
    void roundTripsPoint();
    void alignsPixelCoverage();
    void rejectsInvalidScale_data();
    void rejectsInvalidScale();
    void rejectsInvalidGeometry();
};

void ScaleAdapterTest::mapsFractionalScale()
{
    const auto result = ScaleAdapter::toPixels(QPointF(100.25, 200.5), 1.5);
    QVERIFY(result);
    QCOMPARE(*result, QPointF(150.375, 300.75));
}

void ScaleAdapterTest::roundTripsPoint()
{
    const QPointF logical(1706.6666, 1398.25);
    const auto pixels = ScaleAdapter::toPixels(logical, 1.25);
    QVERIFY(pixels);

    const auto restored = ScaleAdapter::toLogical(*pixels, 1.25);
    QVERIFY(restored);
    QVERIFY(qAbs(restored->x() - logical.x()) < 1e-9);
    QVERIFY(qAbs(restored->y() - logical.y()) < 1e-9);
}

void ScaleAdapterTest::alignsPixelCoverage()
{
    const auto result = ScaleAdapter::toAlignedPixels(QRectF(0.2, 0.4, 10.2, 20.2), 1.25);
    QVERIFY(result);
    QCOMPARE(*result, QRect(0, 0, 13, 26));
}

void ScaleAdapterTest::rejectsInvalidScale_data()
{
    QTest::addColumn<qreal>("scale");
    QTest::newRow("zero") << qreal(0.0);
    QTest::newRow("negative") << qreal(-1.0);
    QTest::newRow("nan") << std::numeric_limits<qreal>::quiet_NaN();
    QTest::newRow("infinity") << std::numeric_limits<qreal>::infinity();
}

void ScaleAdapterTest::rejectsInvalidScale()
{
    QFETCH(qreal, scale);

    QVERIFY(!ScaleAdapter::toPixels(QPointF(10.0, 20.0), scale));
    QVERIFY(!ScaleAdapter::toLogical(QPointF(10.0, 20.0), scale));
}

void ScaleAdapterTest::rejectsInvalidGeometry()
{
    QVERIFY(!ScaleAdapter::toPixels(QRectF(0.0, 0.0, 0.0, 20.0), 1.5));
    QVERIFY(!ScaleAdapter::toLogical(QRectF(0.0, 0.0, 20.0, -1.0), 1.5));
    QVERIFY(!ScaleAdapter::toPixels(
        QRectF(std::numeric_limits<qreal>::quiet_NaN(), 0.0, 10.0, 10.0), 1.5));
    QVERIFY(!ScaleAdapter::toPixels(QPointF(1e308, 0.0), 2.0));
}

QTEST_GUILESS_MAIN(ScaleAdapterTest)

#include "scaleadaptertest.moc"
