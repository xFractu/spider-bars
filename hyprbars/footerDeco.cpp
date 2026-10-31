#include "footerDeco.hpp"

#include <algorithm>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/window/Window.hpp>
#include <hyprland/src/desktop/view/window/WindowPresentation.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprutils/memory/Casts.hpp>

using namespace Hyprutils::Memory;
using namespace Render::GL;

static CHyprColor configColor(Config::INTEGER color) {
    return CHyprColor{sc<uint64_t>(color)};
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
    info.desiredExtents = {{0, 0}, {0, sc<double>(H)}};
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
    CBox       box     = {DECOBOX.x - pMonitor->m_position.x, DECOBOX.y - pMonitor->m_position.y, DECOBOX.w, DECOBOX.h};
    box.translate(PWINDOW->presentation().floatingOffset()).scale(pMonitor->m_scale).round();

    if (box.w < 1 || box.h < 1)
        return;

    CHyprColor color = configColor(g_pGlobalState->config.spiderFooterColor->value());
    color.a *= a;

    // rounding of the bottom corners (same as the bar's top corners, see bar_rounding)
    const auto BR = g_pGlobalState->config.spiderBarRounding->value();
    const int  R  = BR > 0 ? sc<int>(std::min<double>(BR * pMonitor->m_scale, std::min(box.w, box.h) / 2.0)) : 0;

    g_pHyprOpenGL->scissor(ctx, nullptr);

    if (R < 1) {
        g_pHyprOpenGL->renderRect(ctx, box, color, CHyprOpenGLImpl::SRectRenderData{});
        return;
    }

    // rounded rect, then square off the top half so only the bottom corners stay rounded
    g_pHyprOpenGL->renderRect(ctx, box, color, CHyprOpenGLImpl::SRectRenderData{.round = R, .roundingPower = PWINDOW->presentation().roundingPower()});
    CBox top = {box.x, box.y, box.w, box.h - R};
    top.round();
    if (top.h > 0)
        g_pHyprOpenGL->renderRect(ctx, top, color, CHyprOpenGLImpl::SRectRenderData{});
}

std::vector<UP<IPassElement>> CSpiderFooterPassElement::draw(Render::CRenderContext& ctx) {
    data.deco->renderPass(ctx, ctx.m_data.pMonitor.lock(), data.a);
    return {};
}

std::optional<CBox> CSpiderFooterPassElement::boundingBox(Render::CRenderContext& ctx) {
    return data.deco->assignedBoxGlobal().translate(-ctx.m_data.pMonitor->m_position).expand(10);
}
