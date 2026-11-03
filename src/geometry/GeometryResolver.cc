/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

// Own
#include "GeometryResolver.h"
#include "OutputManager.h"

// KWin
#include <core/output.h>
#include <effect/effectwindow.h>
#include <wayland/plasmawindowmanagement.h>
#include <wayland_server.h>
#include <window.h>

// Qt
#include <QLoggingCategory>

// std
#include <optional>

Q_LOGGING_CATEGORY(lcGeometryResolver, "kwin_effect_yetanothermagiclamp", QtWarningMsg)

namespace CoordinateMapper {

namespace {

quintptr windowId(const void* window)
{
    return reinterpret_cast<quintptr>(window);
}

std::optional<QRectF> buttonOnOutput(KWin::WaylandServer* server,
    const KWin::Window* window, KWin::LogicalOutput* output,
    const QPointF& center, quintptr logId)
{
    if (!window || !window->windowManagementInterface()) {
        return std::nullopt;
    }

    QList<PanelButtonCandidate> candidates;
    const auto geometries = window->windowManagementInterface()->minimizedGeometries();
    candidates.reserve(geometries.size());

    for (auto it = geometries.constBegin(); it != geometries.constEnd(); ++it) {
        KWin::Window* panel = server->findWindow(it.key());
        if (!panel) {
            continue;
        }

        const QRectF panelRect = panel->frameGeometry();
        const QRectF button = CoordinateMapper::surfaceToGlobal(QRect(it.value()), panel->pos());
        const bool belongsToTargetOutput = panel->output() == output;

        qCDebug(lcGeometryResolver).noquote() << "button.candidate"
                                              << "window=" << logId
                                              << "owner=" << windowId(window)
                                              << "panel=" << windowId(panel)
                                              << "panelGeometry=" << panelRect
                                              << "panelOutput=" << (panel->output() ? panel->output()->name() : QStringLiteral("null"))
                                              << "button=" << button;

        candidates.append({belongsToTargetOutput, panelRect, button});
    }

    return nearestButtonOnOutput(candidates, center);
}

} // namespace

GeometryResolver::GeometryResolver(OutputManager* outputs)
    : m_outputs(outputs)
{
}

std::optional<QRectF> GeometryResolver::resolve(KWin::EffectWindow* effectWindow) const
{
    KWin::Window* window = effectWindow ? effectWindow->window() : nullptr;
    KWin::WaylandServer* server = KWin::waylandServer();
    if (!window || !server) {
        qCDebug(lcGeometryResolver).noquote() << "resolve.unsupported"
                                              << "window=" << windowId(effectWindow);
        return std::nullopt;
    }

    KWin::LogicalOutput* output = window->output();
    if (!output || !m_outputs || !m_outputs->infoFor(output)) {
        qCDebug(lcGeometryResolver).noquote() << "resolve.no-tracked-output"
                                              << "window=" << windowId(effectWindow);
        return std::nullopt;
    }

    const QPointF center = QRectF(window->frameGeometry()).center();
    std::optional<QRectF> target = buttonOnOutput(server, window, output, center, windowId(effectWindow));
    if (!target) {
        const auto mainWindows = window->mainWindows();
        for (const KWin::Window* mainWindow : mainWindows) {
            target = buttonOnOutput(server, mainWindow, output, center, windowId(effectWindow));
            if (target) {
                break;
            }
        }
    }

    qCDebug(lcGeometryResolver).noquote() << "resolve"
                                          << "window=" << windowId(effectWindow)
                                          << "windowGeometry=" << QRectF(window->frameGeometry())
                                          << "windowOutput=" << output->name()
                                          << "kwinIconGeometry=" << QRectF(effectWindow->iconGeometry())
                                          << "target=" << (target ? *target : QRectF());
    return target;
}

} // namespace CoordinateMapper
