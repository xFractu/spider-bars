#include "outlineDeco.hpp"
#include "spiderUtil.hpp"

#include <algorithm>

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

static constexpr uint32_t ALLEDGES = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;

static int outlineSize() {
    if (!g_pGlobalState->config.enabled->value())
        return 0;
    return std::max<int>(0, g_pGlobalState->config.spiderOutlineSize->value());
}

// segundo borde (más afuera); solo se dibuja si también hay primer borde
static int outline2Size() {
    if (outlineSize() < 1)
        return 0;
    return std::max<int>(0, g_pGlobalState->config.spiderOutline2Size->value());
}

static int totalSize() {
    return outlineSize() + outline2Size();
}

CSpiderOutline::CSpiderOutline(PHLWINDOW pWindow) : IHyprWindowDecoration(pWindow), m_pWindow(pWindow) {
    ;
}

CSpiderOutline::~CSpiderOutline() {
    damageEntire();
}

SDecorationPositioningInfo CSpiderOutline::getPositioningInfo() {
    const double               S = totalSize();

    SDecorationPositioningInfo info;
    info.policy         = DECORATION_POSITION_STICKY;
    info.edges          = ALLEDGES;
    info.priority       = 1000; // lower than the bars -> sits outside of them
    info.reserved       = true;
    info.desiredExtents = {Vector2D{S, S}, Vector2D{S, S}};
    return info;
}

void CSpiderOutline::onPositioningReply(const SDecorationPositioningReply& reply) {
    m_bAssignedBox = reply.assignedGeometry;
}

std::string CSpiderOutline::getDisplayName() {
    return "SpiderOutline";
}

eDecorationType CSpiderOutline::getDecorationType() {
    return DECORATION_CUSTOM;
}

eDecorationLayer CSpiderOutline::getDecorationLayer() {
    return DECORATION_LAYER_OVER;
}

uint64_t CSpiderOutline::getDecorationFlags() {
    return 0;
}

void CSpiderOutline::updateWindow(PHLWINDOW) {
    damageEntire();
}

void CSpiderOutline::onConfigReloaded() {
    g_pDecorationPositioner->repositionDeco(this);
    damageEntire();
}

CBox CSpiderOutline::fullBoxGlobal() {
    if (!validMapped(m_pWindow))
        return {};

    CBox box = m_bAssignedBox;
    box.translate(g_pDecorationPositioner->getEdgeDefinedPoint(ALLEDGES, m_pWindow.lock()));

    const auto PWORKSPACE      = m_pWindow->m_workspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !(m_pWindow->m_state & Desktop::View::WINDOW_STATE_PINNED) ? PWORKSPACE->m_renderOffset->value() : Vector2D();

    return box.translate(WORKSPACEOFFSET);
}

void CSpiderOutline::damageEntire() {
    g_pHyprRenderer->damageBox(fullBoxGlobal().expand(2));
}

void CSpiderOutline::draw(Render::CRenderContext& ctx, PHLMONITOR monitor, const float& a, const Render::SWindowRenderPresentation& presentation) {
    if (!validMapped(m_pWindow) || totalSize() < 1)
        return;

    const auto PWINDOW = m_pWindow.lock();
    if (!PWINDOW->m_ruleApplicator->decorate().valueOrDefault())
        return;

    auto data = CSpiderOutlinePassElement::SData{this, a};
    Render::IHyprRenderer::addPassElement(ctx, makeUnique<CSpiderOutlinePassElement>(data));
}

void CSpiderOutline::renderPass(Render::CRenderContext& ctx, PHLMONITOR pMonitor, const float& a) {
    const auto PWINDOW = m_pWindow.lock();
    if (!PWINDOW)
        return;

    const auto S  = outlineSize();
    const auto S2 = outline2Size();
    if (S + S2 != m_iLastSize) {
        m_iLastSize = S + S2;
        PWINDOW->layoutTarget()->recalc();
    }
    if (S < 1)
        return;

    CBox box = m_bAssignedBox;
    box.translate(g_pDecorationPositioner->getEdgeDefinedPoint(ALLEDGES, PWINDOW));

    const auto PWORKSPACE      = PWINDOW->m_workspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !(PWINDOW->m_state & Desktop::View::WINDOW_STATE_PINNED) ? PWORKSPACE->m_renderOffset->value() : Vector2D();

    box.translate(PWINDOW->presentation().floatingOffset() - pMonitor->m_position + WORKSPACEOFFSET);

    // inner box (the outlines are drawn outwards from it)
    CBox inner = box;
    inner.expand(-(S + S2));
    CBox middle = inner;   // borde exterior: empieza donde termina el primero
    middle.expand(S);
    inner.scale(pMonitor->m_scale).round();
    middle.scale(pMonitor->m_scale).round();
    box = inner;

    if (box.w < 1 || box.h < 1)
        return;

    const auto BR    = g_pGlobalState->config.spiderBarRounding->value();
    const int  ROUND = (BR > 0 && !spiderIsMaximized(PWINDOW)) ? sc<int>(BR * pMonitor->m_scale) : 0;

    g_pHyprOpenGL->scissor(ctx, nullptr);

    g_pHyprOpenGL->renderBorder(ctx, box, Config::CGradientValueData(CHyprColor{sc<uint64_t>(g_pGlobalState->config.spiderOutlineColor->value())}),
                                CHyprOpenGLImpl::SBorderRenderData{.round = ROUND, .roundingPower = PWINDOW->presentation().roundingPower(), .borderSize = S, .a = a, .outerRound = -1});

    if (S2 > 0) {
        const int ROUND2 = ROUND > 0 ? ROUND + sc<int>(S * pMonitor->m_scale) : 0;
        g_pHyprOpenGL->renderBorder(ctx, middle, Config::CGradientValueData(CHyprColor{sc<uint64_t>(g_pGlobalState->config.spiderOutline2Color->value())}),
                                    CHyprOpenGLImpl::SBorderRenderData{.round = ROUND2, .roundingPower = PWINDOW->presentation().roundingPower(), .borderSize = S2, .a = a, .outerRound = -1});
    }
}

std::vector<UP<IPassElement>> CSpiderOutlinePassElement::draw(Render::CRenderContext& ctx) {
    data.deco->renderPass(ctx, ctx.m_data.pMonitor.lock(), data.a);
    return {};
}

std::optional<CBox> CSpiderOutlinePassElement::boundingBox(Render::CRenderContext& ctx) {
    return data.deco->fullBoxGlobal().translate(-ctx.m_data.pMonitor->m_position).expand(10);
}
