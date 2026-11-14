/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/ModelGeometry.h"

#include <QTest>

#include <limits>

class ModelGeometryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void acceptsFinitePositiveRectangles();
    void rejectsInvalidRectangles();
    void calculatesDistanceForEachDirection();
    void rejectsZeroAndNegativeAnimationDistances();
    void rejectsNonFiniteInputs();
    void calculatesShapeFactorWithFractionalGeometry();
    void rejectsNonFiniteShapeFactor();
};

void ModelGeometryTest::acceptsFinitePositiveRectangles()
{
    QVERIFY(ModelGeometry::isFinitePositiveRect(QRectF(10.5, -20.25, 800.5, 600.25)));
}

void ModelGeometryTest::rejectsInvalidRectangles()
{
    QVERIFY(!ModelGeometry::isFinitePositiveRect(QRectF()));
    QVERIFY(!ModelGeometry::isFinitePositiveRect(QRectF(0, 0, 0, 100)));
    QVERIFY(!ModelGeometry::isFinitePositiveRect(QRectF(0, 0, 100, -1)));
    QVERIFY(!ModelGeometry::isFinitePositiveRect(QRectF(
        std::numeric_limits<qreal>::quiet_NaN(), 0, 100, 100)));
    QVERIFY(!ModelGeometry::isFinitePositiveRect(QRectF(
        std::numeric_limits<qreal>::max(), 0, std::numeric_limits<qreal>::max(), 100)));
}

void ModelGeometryTest::calculatesDistanceForEachDirection()
{
    const QRectF window(100, 100, 800, 600);

    const auto top = ModelGeometry::distanceToIcon(Direction::Top, window, QRectF(400, 0, 40, 40), 0);
    const auto right = ModelGeometry::distanceToIcon(Direction::Right, window, QRectF(950, 300, 40, 40), 0);
    const auto bottom = ModelGeometry::distanceToIcon(Direction::Bottom, window, QRectF(400, 750, 40, 40), 0);
    const auto left = ModelGeometry::distanceToIcon(Direction::Left, window, QRectF(0, 300, 40, 40), 0);

    QVERIFY(top);
    QVERIFY(right);
    QVERIFY(bottom);
    QVERIFY(left);
    QCOMPARE(*top, 660.0);
    QCOMPARE(*right, 850.0);
    QCOMPARE(*bottom, 650.0);
    QCOMPARE(*left, 860.0);
}

void ModelGeometryTest::rejectsZeroAndNegativeAnimationDistances()
{
    const QRectF window(100, 100, 800, 600);

    QVERIFY(!ModelGeometry::distanceToIcon(Direction::Top, window, QRectF(400, 660, 40, 40), 0));
    QVERIFY(!ModelGeometry::distanceToIcon(Direction::Top, window, QRectF(400, 680, 40, 40), 0));
    QVERIFY(!ModelGeometry::distanceToIcon(Direction::Right, window, QRectF(99, 300, 40, 40), 0));
    QVERIFY(!ModelGeometry::distanceToIcon(Direction::Bottom, window, QRectF(400, 99, 40, 40), 0));
    QVERIFY(!ModelGeometry::distanceToIcon(Direction::Left, window, QRectF(860, 300, 40, 40), 0));
}

void ModelGeometryTest::rejectsNonFiniteInputs()
{
    const QRectF window(100, 100, 800, 600);
    const QRectF icon(400, 0, 40, 40);

    QVERIFY(!ModelGeometry::distanceToIcon(Direction::Top, window, icon,
        std::numeric_limits<qreal>::infinity()));
    QVERIFY(!ModelGeometry::distanceToIcon(Direction::Top, window, icon, -1));
}

void ModelGeometryTest::calculatesShapeFactorWithFractionalGeometry()
{
    const QRectF window(100.5, 100.25, 800.5, 600.5);
    const QRectF icon(400.25, 0.5, 40.5, 40.5);

    const auto factor = ModelGeometry::shapeFactor(Direction::Top, window, icon, 0, 0.2);
    QVERIFY(factor);
    QVERIFY(std::isfinite(*factor));
    QVERIFY(*factor > 0.9);
    QVERIFY(*factor < 1.0);
}

void ModelGeometryTest::rejectsNonFiniteShapeFactor()
{
    const QRectF window(100, 100, 800, 600);
    const QRectF icon(400, 0, 40, 40);

    QVERIFY(!ModelGeometry::shapeFactor(Direction::Top, window, icon, 0,
        std::numeric_limits<qreal>::quiet_NaN()));
}

QTEST_GUILESS_MAIN(ModelGeometryTest)

#include "modelgeometrytest.moc"
