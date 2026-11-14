/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

// Qt
#include <QElapsedTimer>
#include <QHash>
#include <QMetaObject>
#include <QObject>
#include <QPointer>

namespace KWin {
class EffectWindow;
class LogicalOutput;
}

namespace CoordinateMapper {

class OutputManager;

// Tracks the time windows have spent on their current output. Plasma task
// managers may publish a new task button after a window is added or moves to
// another output; consumers can use this bounded settling window when retrying
// geometry resolution.
class WindowTracker final : public QObject
{
    Q_OBJECT

public:
    explicit WindowTracker(OutputManager* outputs, QObject* parent = nullptr);
    ~WindowTracker() override = default;

    // Returns true only when a new window was registered.
    bool trackWindow(KWin::EffectWindow* window);
    void untrackWindow(KWin::EffectWindow* window);

    bool isTracked(KWin::EffectWindow* window) const;
    bool taskButtonsSettled(KWin::EffectWindow* window, qint64 settleMs) const;

    // Pure threshold logic, exposed for deterministic unit tests.
    static bool hasSettled(qint64 sinceAddedMs, qint64 sinceOutputChangeMs, qint64 settleMs)
    {
        return settleMs >= 0
            && sinceAddedMs >= settleMs
            && sinceOutputChangeMs >= settleMs;
    }

    // Pointer identity is enough here: output objects are only compared, never dereferenced.
    static bool outputIdentityChanged(const void* previous, const void* current) noexcept
    {
        return previous != current;
    }

Q_SIGNALS:
    void windowOutputChanged(KWin::EffectWindow* window);

private:
    struct Entry {
        QElapsedTimer sinceAdded;
        QElapsedTimer sinceOutputChange;
        QPointer<KWin::LogicalOutput> output;
        QMetaObject::Connection geometryConnection;
        QMetaObject::Connection outputConnection;
        QMetaObject::Connection destroyedConnection;
    };

    void updateWindowOutput(KWin::EffectWindow* window);
    void handleOutputAboutToBeRemoved(KWin::LogicalOutput* output);
    void resetWindowsOnOutput(KWin::LogicalOutput* output);

    QHash<KWin::EffectWindow*, Entry> m_entries;
};

} // namespace CoordinateMapper
