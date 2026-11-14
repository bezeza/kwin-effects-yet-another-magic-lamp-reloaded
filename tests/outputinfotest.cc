/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "geometry/OutputInfo.h"

#include <QTest>

#include <limits>

using CoordinateMapper::OutputInfo;
using CoordinateMapper::TransformKind;

class OutputInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void acceptsFractionalScale();
    void rejectsInvalidScale_data();
    void rejectsInvalidScale();
    void rejectsInvalidGeometry();
    void rejectsNonFiniteGeometry();
    void rejectsOverflowingGeometry();
    void rejectsMissingModeSize();
    void rejectsUnknownTransform();
};

void OutputInfoTest::acceptsFractionalScale()
{
    OutputInfo info;
    info.id = QStringLiteral("DP-1");
    info.name = QStringLiteral("DisplayPort-1");
    info.geometry = QRectF(-1706.6667, 0.0, 1706.6667, 960.0);
    info.scale = 1.5;
    info.nativePixelSize = QSize(2560, 1440);
    info.transformedPixelSize = QSize(2560, 1440);
    info.transform = TransformKind::Normal;

    QVERIFY(info.isValid());
}

void OutputInfoTest::rejectsInvalidScale_data()
{
    QTest::addColumn<qreal>("scale");
    QTest::newRow("zero") << qreal(0.0);
    QTest::newRow("negative") << qreal(-1.0);
    QTest::newRow("nan") << std::numeric_limits<qreal>::quiet_NaN();
    QTest::newRow("infinity") << std::numeric_limits<qreal>::infinity();
}

void OutputInfoTest::rejectsInvalidScale()
{
    QFETCH(qreal, scale);

    OutputInfo info;
    info.id = QStringLiteral("DP-1");
    info.geometry = QRectF(0.0, 0.0, 1920.0, 1080.0);
    info.scale = scale;
    info.nativePixelSize = QSize(1920, 1080);
    info.transformedPixelSize = QSize(1920, 1080);
    info.transform = TransformKind::Normal;

    QVERIFY(!info.isValid());
}

void OutputInfoTest::rejectsInvalidGeometry()
{
    OutputInfo info;
    info.id = QStringLiteral("DP-1");
    info.geometry = QRectF(0.0, 0.0, 0.0, 1080.0);
    info.scale = 1.0;
    info.nativePixelSize = QSize(1920, 1080);
    info.transformedPixelSize = QSize(1920, 1080);
    info.transform = TransformKind::Normal;

    QVERIFY(!info.isValid());
}

void OutputInfoTest::rejectsNonFiniteGeometry()
{
    OutputInfo info;
    info.id = QStringLiteral("DP-1");
    info.geometry = QRectF(std::numeric_limits<qreal>::quiet_NaN(), 0.0, 1920.0, 1080.0);
    info.scale = 1.0;
    info.nativePixelSize = QSize(1920, 1080);
    info.transformedPixelSize = QSize(1920, 1080);
    info.transform = TransformKind::Normal;

    QVERIFY(!info.isValid());
}

void OutputInfoTest::rejectsOverflowingGeometry()
{
    OutputInfo info;
    info.id = QStringLiteral("DP-1");
    info.geometry = QRectF(
        std::numeric_limits<qreal>::max(), 0.0,
        std::numeric_limits<qreal>::max(), 1080.0);
    info.scale = 1.0;
    info.nativePixelSize = QSize(1920, 1080);
    info.transformedPixelSize = QSize(1920, 1080);
    info.transform = TransformKind::Normal;

    QVERIFY(!info.isValid());
}

void OutputInfoTest::rejectsMissingModeSize()
{
    OutputInfo info;
    info.id = QStringLiteral("DP-1");
    info.geometry = QRectF(0.0, 0.0, 1920.0, 1080.0);
    info.scale = 1.0;
    info.nativePixelSize = QSize();
    info.transformedPixelSize = QSize(1920, 1080);
    info.transform = TransformKind::Normal;

    QVERIFY(!info.isValid());
}

void OutputInfoTest::rejectsUnknownTransform()
{
    OutputInfo info;
    info.id = QStringLiteral("DP-1");
    info.geometry = QRectF(0.0, 0.0, 1920.0, 1080.0);
    info.scale = 1.0;
    info.nativePixelSize = QSize(1920, 1080);
    info.transformedPixelSize = QSize(1920, 1080);
    info.transform = TransformKind::Unknown;

    QVERIFY(!info.isValid());
}

QTEST_GUILESS_MAIN(OutputInfoTest)

#include "outputinfotest.moc"
