/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

// Own
#include "OutputInfo.h"

// KWin
#include <core/output.h>
#include <effect/effecthandler.h>

// Qt
#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

// std
#include <optional>

namespace CoordinateMapper {

// Tracks KWin logical outputs and publishes value snapshots.
//
// The manager observes output lifecycle and geometry/scale/transform changes.
// Consumers should use OutputInfo for calculations and retain the LogicalOutput
// pointer only when they need to call back into KWin.
class OutputManager final : public QObject
{
    Q_OBJECT

public:
    explicit OutputManager(KWin::EffectsHandler* effects, QObject* parent = nullptr);
    ~OutputManager() override = default;

    QList<OutputInfo> outputs() const;
    std::optional<OutputInfo> infoFor(KWin::LogicalOutput* output) const;
    KWin::LogicalOutput* outputForId(const QString& id) const;

Q_SIGNALS:
    void outputsChanged();
    void outputAdded(const QString& id);
    void outputUpdated(const QString& id);
    // Emitted before the entry is erased and before KWin releases the output.
    void outputAboutToBeRemoved(KWin::LogicalOutput* output);
    void outputRemoved(const QString& id);

private:
    struct Entry {
        QPointer<KWin::LogicalOutput> output;
        OutputInfo info;
        QList<QMetaObject::Connection> connections;
    };

    void addOutput(KWin::LogicalOutput* output);
    void removeOutput(KWin::LogicalOutput* output);
    void refreshOutput(KWin::LogicalOutput* output);

    static OutputInfo readOutputInfo(const KWin::LogicalOutput* output);

    QPointer<KWin::EffectsHandler> m_effects;
    QHash<KWin::LogicalOutput*, Entry> m_entries;
};

} // namespace CoordinateMapper
