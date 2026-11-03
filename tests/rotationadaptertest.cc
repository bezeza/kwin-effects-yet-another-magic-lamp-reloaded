/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/RotationAdapter.h"

#include <QTest>

#include <limits>

using CoordinateMapper::RotationAdapter;
using CoordinateMapper::TransformKind;

class RotationAdapterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void transformedSize_data();
    void transformedSize();
    void rotate90MatchesKWin();
    void roundTripsRect_data();
    void roundTripsRect();
    void rejectsUnknownTransform();
};

void RotationAdapterTest::transformedSize_data()
{
    QTest::addColumn<TransformKind>("transform");
    QTest::addColumn<QSizeF>("expected");

    const QSizeF nativeSize(1920.0, 1080.0);
    QTest::newRow("normal") << TransformKind::Normal << nativeSize;
    QTest::newRow("rotate-90") << TransformKind::Rotate90 << QSizeF(1080.0, 1920.0);
    QTest::newRow("rotate-180") << TransformKind::Rotate180 << nativeSize;
    QTest::newRow("rotate-270") << TransformKind::Rotate270 << QSizeF(1080.0, 1920.0);
    QTest::newRow("flip-x") << TransformKind::FlipX << nativeSize;
    QTest::newRow("flip-x-90") << TransformKind::FlipX90 << QSizeF(1080.0, 1920.0);
    QTest::newRow("flip-x-180") << TransformKind::FlipX180 << nativeSize;
    QTest::newRow("flip-x-270") << TransformKind::FlipX270 << QSizeF(1080.0, 1920.0);
}

void RotationAdapterTest::transformedSize()
{
    QFETCH(TransformKind, transform);
    QFETCH(QSizeF, expected);

    QCOMPARE(RotationAdapter::transformedSize(QSizeF(1920.0, 1080.0), transform), expected);
}

void RotationAdapterTest::rotate90MatchesKWin()
{
    const auto result = RotationAdapter::toTransformed(
        QRectF(100.0, 200.0, 300.0, 400.0), QSizeF(1920.0, 1080.0), TransformKind::Rotate90);

    QVERIFY(result);
    QCOMPARE(*result, QRectF(200.0, 1520.0, 400.0, 300.0));
}

void RotationAdapterTest::roundTripsRect_data()
{
    QTest::addColumn<TransformKind>("transform");
    QTest::newRow("normal") << TransformKind::Normal;
    QTest::newRow("rotate-90") << TransformKind::Rotate90;
    QTest::newRow("rotate-180") << TransformKind::Rotate180;
    QTest::newRow("rotate-270") << TransformKind::Rotate270;
    QTest::newRow("flip-x") << TransformKind::FlipX;
    QTest::newRow("flip-x-90") << TransformKind::FlipX90;
    QTest::newRow("flip-x-180") << TransformKind::FlipX180;
    QTest::newRow("flip-x-270") << TransformKind::FlipX270;
}

void RotationAdapterTest::roundTripsRect()
{
    QFETCH(TransformKind, transform);

    const QRectF original(100.25, 200.5, 300.75, 400.125);
    const QSizeF nativeBounds(1920.0, 1080.0);
    const auto transformed = RotationAdapter::toTransformed(original, nativeBounds, transform);
    QVERIFY(transformed);

    const auto restored = RotationAdapter::toNative(*transformed, nativeBounds, transform);
    QVERIFY(restored);
    QVERIFY(qAbs(restored->x() - original.x()) < 1e-9);
    QVERIFY(qAbs(restored->y() - original.y()) < 1e-9);
    QVERIFY(qAbs(restored->width() - original.width()) < 1e-9);
    QVERIFY(qAbs(restored->height() - original.height()) < 1e-9);
}

void RotationAdapterTest::rejectsUnknownTransform()
{
    QVERIFY(!RotationAdapter::toTransformed(
        QRectF(1.0, 1.0, 10.0, 10.0), QSizeF(1920.0, 1080.0), TransformKind::Unknown));
    QVERIFY(!RotationAdapter::toNative(
        QRectF(1.0, 1.0, 10.0, 10.0), QSizeF(1920.0, 1080.0), TransformKind::Unknown));
    QVERIFY(!RotationAdapter::toTransformed(
        QRectF(std::numeric_limits<qreal>::quiet_NaN(), 0.0, 10.0, 10.0),
        QSizeF(1920.0, 1080.0), TransformKind::Normal));
}

QTEST_GUILESS_MAIN(RotationAdapterTest)

#include "rotationadaptertest.moc"
