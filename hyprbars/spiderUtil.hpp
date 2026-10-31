#pragma once

#include <hyprland/src/desktop/view/window/Window.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>

// SpiderOS: true when the window is maximized or fullscreen -> draw square corners
inline bool spiderIsMaximized(PHLWINDOW w) {
    return w && Fullscreen::controller()->getFullscreenModes(w).internal != Fullscreen::FSMODE_NONE;
}
