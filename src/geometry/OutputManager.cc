/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "OutputManager.h"

namespace CoordinateMapper {

OutputManager::OutputManager(KWin::EffectsHandler* effects, QObject* parent)
    : QObject(parent)
    , m_effects(effects)
{
    if (!effects) {
        return;
    }

    connect(effects, &KWin::EffectsHandler::screenAdded,
        this, &OutputManager::addOutput);
    connect(effects, &KWin::EffectsHandler::screenRemoved,
        this, &OutputManager::removeOutput);

    for (KWin::LogicalOutput* output : effects->screens()) {
        addOutput(output);
    }
}

QList<OutputInfo> OutputManager::outputs() const
{
    QList<OutputInfo> result;
    if (!m_effects) {
        return result;
    }

    // Preserve KWin's output ordering instead of relying on QHash iteration.
    for (KWin::LogicalOutput* output : m_effects->screens()) {
        const auto it = m_entries.constFind(output);
        if (it != m_entries.constEnd() && !it->output.isNull()) {
            result.append(it->info);
        }
    }
    return result;
}

std::optional<OutputInfo> OutputManager::infoFor(KWin::LogicalOutput* output) const
{
    if (!output) {
        return std::nullopt;
    }

    const auto it = m_entries.constFind(output);
    if (it == m_entries.constEnd() || it->output.isNull()) {
        return std::nullopt;
    }
    return it->info;
}

KWin::LogicalOutput* OutputManager::outputForId(const QString& id) const
{
    for (auto it = m_entries.constBegin(); it != m_entries.constEnd(); ++it) {
        if (it->info.id == id) {
            return it->output.data();
        }
    }
    return nullptr;
}

void OutputManager::addOutput(KWin::LogicalOutput* output)
{
    if (!output || m_entries.contains(output)) {
        return;
    }

    Entry entry;
    entry.output = output;
    entry.info = readOutputInfo(output);

    const auto refresh = [this, output]() {
        refreshOutput(output);
    };
    entry.connections.append(connect(output, &KWin::LogicalOutput::geometryChanged, this, refresh));
    entry.connections.append(connect(output, &KWin::LogicalOutput::scaleChanged, this, refresh));
    entry.connections.append(connect(output, &KWin::LogicalOutput::transformChanged, this, refresh));
    entry.connections.append(connect(output, &KWin::LogicalOutput::currentModeChanged, this, refresh));

    const QString id = entry.info.id;
    m_entries.insert(output, entry);

    Q_EMIT outputAdded(id);
    Q_EMIT outputsChanged();
}

void OutputManager::removeOutput(KWin::LogicalOutput* output)
{
    auto it = m_entries.find(output);
    if (it == m_entries.end()) {
        return;
    }

    const QString id = it->info.id;
    for (const QMetaObject::Connection& connection : it->connections) {
        QObject::disconnect(connection);
    }
    m_entries.erase(it);

    Q_EMIT outputRemoved(id);
    Q_EMIT outputsChanged();
}

void OutputManager::refreshOutput(KWin::LogicalOutput* output)
{
    auto it = m_entries.find(output);
    if (it == m_entries.end() || it->output.isNull()) {
        return;
    }

    const OutputInfo updated = readOutputInfo(output);
    if (it->info == updated) {
        return;
    }

    it->info = updated;
    Q_EMIT outputUpdated(updated.id);
    Q_EMIT outputsChanged();
}

OutputInfo OutputManager::readOutputInfo(const KWin::LogicalOutput* output)
{
    OutputInfo info;
    if (!output) {
        return info;
    }

    info.id = output->uuid();
    if (info.id.isEmpty()) {
        info.id = output->name();
    }
    info.name = output->name();
    info.geometry = QRectF(output->geometryF());
    info.scale = output->scale();
    info.nativePixelSize = output->modeSize();
    info.transformedPixelSize = output->pixelSize();

    switch (output->transform().kind()) {
    case KWin::OutputTransform::Normal:
        info.transform = TransformKind::Normal;
        break;
    case KWin::OutputTransform::Rotate90:
        info.transform = TransformKind::Rotate90;
        break;
    case KWin::OutputTransform::Rotate180:
        info.transform = TransformKind::Rotate180;
        break;
    case KWin::OutputTransform::Rotate270:
        info.transform = TransformKind::Rotate270;
        break;
    case KWin::OutputTransform::FlipX:
        info.transform = TransformKind::FlipX;
        break;
    case KWin::OutputTransform::FlipX90:
        info.transform = TransformKind::FlipX90;
        break;
    case KWin::OutputTransform::FlipX180:
        info.transform = TransformKind::FlipX180;
        break;
    case KWin::OutputTransform::FlipX270:
        info.transform = TransformKind::FlipX270;
        break;
    default:
        info.transform = TransformKind::Unknown;
        break;
    }

    return info;
}

} // namespace CoordinateMapper
