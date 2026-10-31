#include "footerDeco.hpp"

#include <algorithm>
#include <cctype>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/window/Window.hpp>
#include <hyprland/src/desktop/view/window/WindowPresentation.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/layout/target/Target.hpp>
#include <hyprutils/memory/Casts.hpp>

using namespace Hyprutils::Memory;
using namespace Render::GL;

static CHyprColor configColor(Config::INTEGER color) {
    return CHyprColor{sc<uint64_t>(color)};
}

// "org.gnome.Nautilus" -> "NAUTILUS", "firefox" -> "FIREFOX"
static std::string appName(const std::string& appID) {
    std::string name = appID;
    if (const auto dot = name.find_last_of('.'); dot != std::string::npos && dot + 1 < name.size())
        name = name.substr(dot + 1);

    std::string out;
    for (unsigned char c : name) {
        if (std::isspace(c))
            continue;
        out += sc<char>(std::toupper(c));
    }
    return out.empty() ? "APP" : out;
}

static std::string footerText(const std::string& appID) {
    std::string       fmt = g_pGlobalState->config.spiderFooterText->value();
    const std::string APP = appName(appID);

    size_t            pos = 0;
    while ((pos = fmt.find("{app}", pos)) != std::string::npos) {
        fmt.replace(pos, 5, APP);
        pos += APP.size();
    }
    return fmt;
}

static int footerHeight() {
    if (!g_pGlobalState->config.enabled->value())
        return 0;
    return std::max<int>(0, g_pGlobalState->config.spiderFooterHeight->value());
}

CSpiderFooter::CSpiderFooter(PHLWINDOW pWindow) : IHyprWindowDecoration(pWindow), m_pWindow(pWindow) {
    ;
}

CSpiderFooter::~CSpiderFooter() {
    damageEntire();
}

SDecorationPositioningInfo CSpiderFooter::getPositioningInfo() {
    const auto                 H = footerHeight();

    SDecorationPositioningInfo info;
    info.policy         = DECORATION_POSITION_STICKY;
    info.edges          = DECORATION_EDGE_BOTTOM;
    info.priority       = 5000;
    info.reserved       = true;
    info.desiredExtents = {Vector2D{0.0, 0.0}, Vector2D{0.0, sc<double>(H)}};
    return info;
}

void CSpiderFooter::onPositioningReply(const SDecorationPositioningReply& reply) {
    m_bAssignedBox = reply.assignedGeometry;
}

std::string CSpiderFooter::getDisplayName() {
    return "SpiderFooter";
}

eDecorationType CSpiderFooter::getDecorationType() {
    return DECORATION_CUSTOM;
}

eDecorationLayer CSpiderFooter::getDecorationLayer() {
    return DECORATION_LAYER_UNDER;
}

uint64_t CSpiderFooter::getDecorationFlags() {
    return 0;
}

PHLWINDOW CSpiderFooter::getOwner() {
    return m_pWindow.lock();
}

void CSpiderFooter::updateWindow(PHLWINDOW) {
    damageEntire();
}

void CSpiderFooter::onConfigReloaded() {
    m_pTextTex = nullptr;
    m_szTextKey.clear();
    g_pDecorationPositioner->repositionDeco(this);
    damageEntire();
}

CBox CSpiderFooter::assignedBoxGlobal() {
    if (!validMapped(m_pWindow))
        return {};

    CBox box = m_bAssignedBox;
    box.translate(g_pDecorationPositioner->getEdgeDefinedPoint(DECORATION_EDGE_BOTTOM, m_pWindow.lock()));

    const auto PWORKSPACE      = m_pWindow->m_workspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !(m_pWindow->m_state & Desktop::View::WINDOW_STATE_PINNED) ? PWORKSPACE->m_renderOffset->value() : Vector2D();

    return box.translate(WORKSPACEOFFSET);
}

void CSpiderFooter::damageEntire() {
    g_pHyprRenderer->damageBox(assignedBoxGlobal().expand(2));
}

void CSpiderFooter::draw(Render::CRenderContext& ctx, PHLMONITOR monitor, const float& a, const Render::SWindowRenderPresentation& presentation) {
    if (!validMapped(m_pWindow) || footerHeight() < 1)
        return;

    const auto PWINDOW = m_pWindow.lock();
    if (!PWINDOW->m_ruleApplicator->decorate().valueOrDefault())
        return;

    auto data = CSpiderFooterPassElement::SData{this, a};
    Render::IHyprRenderer::addPassElement(ctx, makeUnique<CSpiderFooterPassElement>(data));
}

void CSpiderFooter::renderPass(Render::CRenderContext& ctx, PHLMONITOR pMonitor, const float& a) {
    const auto PWINDOW = m_pWindow.lock();
    if (!PWINDOW)
        return;

    const auto H = footerHeight();
    if (H != m_iLastHeight) {
        m_iLastHeight = H;
        PWINDOW->layoutTarget()->recalc();
    }
    if (H < 1)
        return;

    const auto DECOBOX = assignedBoxGlobal();

    // place the footer right under the window (+ its border), using the real window geometry
    const auto PWORKSPACE      = PWINDOW->m_workspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !(PWINDOW->m_state & Desktop::View::WINDOW_STATE_PINNED) ? PWORKSPACE->m_renderOffset->value() : Vector2D();
    const auto WINPOS          = PWINDOW->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    const auto WINSIZE         = PWINDOW->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    const auto BORDER          = PWINDOW->presentation().borderSize();

    CBox       box = {DECOBOX.x - pMonitor->m_position.x, WINPOS.y + WINSIZE.y + BORDER + WORKSPACEOFFSET.y - pMonitor->m_position.y, DECOBOX.w, sc<double>(H)};
    box.translate(PWINDOW->presentation().floatingOffset()).scale(pMonitor->m_scale).round();

    if (box.w < 1 || box.h < 1)
        return;

    CHyprColor color = configColor(g_pGlobalState->config.spiderFooterColor->value());
    color.a *= a;

    // rounding of the bottom corners (same as the bar's top corners, see bar_rounding)
    const auto BR = g_pGlobalState->config.spiderBarRounding->value();
    const int  R  = BR > 0 ? sc<int>(std::min<double>(BR * pMonitor->m_scale, std::min(box.w, box.h) / 2.0)) : 0;

    g_pHyprOpenGL->scissor(ctx, nullptr);

    if (R < 1)
        g_pHyprOpenGL->renderRect(ctx, box, color, CHyprOpenGLImpl::SRectRenderData{});
    else {
        // rounded rect, then square off the top half so only the bottom corners stay rounded
        g_pHyprOpenGL->renderRect(ctx, box, color, CHyprOpenGLImpl::SRectRenderData{.round = R, .roundingPower = PWINDOW->presentation().roundingPower()});
        CBox top = {box.x, box.y, box.w, box.h - R};
        top.round();
        if (top.h > 0)
            g_pHyprOpenGL->renderRect(ctx, top, color, CHyprOpenGLImpl::SRectRenderData{});
    }

    // ---------- text ----------
    if (g_pGlobalState->config.spiderFooterText->value().empty())
        return;

    const std::string TEXT     = footerText(PWINDOW->metadata().initialAppID());
    const int         SIZE     = sc<int>(std::round(g_pGlobalState->config.spiderFooterTextSize->value() * pMonitor->m_scale));
    const auto        TEXTCOL  = g_pGlobalState->config.spiderFooterTextColor->value();
    const int         PAD      = sc<int>(std::round(g_pGlobalState->config.barPadding->value() * pMonitor->m_scale));
    const int         MAXWIDTH = std::max(1, sc<int>(box.w) - PAD * 2);
    const std::string KEY      = TEXT + "|" + std::to_string(SIZE) + "|" + std::to_string(TEXTCOL) + "|" + std::to_string(MAXWIDTH);

    if (!m_pTextTex || m_pTextTex->m_texID == 0 || m_szTextKey != KEY) {
        m_pTextTex  = g_pHyprRenderer->renderText(TEXT, configColor(TEXTCOL), SIZE, false, g_pGlobalState->config.barTextFont->value(), MAXWIDTH);
        m_szTextKey = KEY;
    }

    if (!m_pTextTex || m_pTextTex->m_texID == 0)
        return;

    CBox textBox = {box.x + PAD, box.y + (box.h - m_pTextTex->m_size.y) / 2.0, m_pTextTex->m_size.x, m_pTextTex->m_size.y};
    textBox.round();
    g_pHyprOpenGL->renderTexture(ctx, m_pTextTex, textBox, {.a = a});
}

std::vector<UP<IPassElement>> CSpiderFooterPassElement::draw(Render::CRenderContext& ctx) {
    data.deco->renderPass(ctx, ctx.m_data.pMonitor.lock(), data.a);
    return {};
}

std::optional<CBox> CSpiderFooterPassElement::boundingBox(Render::CRenderContext& ctx) {
    return data.deco->assignedBoxGlobal().translate(-ctx.m_data.pMonitor->m_position).expand(10);
}
