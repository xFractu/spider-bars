#pragma once

#define WLR_USE_UNSTABLE

#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>
#include <hyprland/src/render/pass/PassElement.hpp>
#include "globals.hpp"

// SpiderOS: plain bar under the window
class CSpiderFooter : public IHyprWindowDecoration {
  public:
    CSpiderFooter(PHLWINDOW);
    virtual ~CSpiderFooter();

    virtual SDecorationPositioningInfo getPositioningInfo();
    virtual void                       onPositioningReply(const SDecorationPositioningReply& reply);
    virtual void                       draw(Render::CRenderContext& ctx, PHLMONITOR monitor, const float& a, const Render::SWindowRenderPresentation& presentation);
    virtual eDecorationType            getDecorationType();
    virtual void                       updateWindow(PHLWINDOW);
    virtual void                       damageEntire();
    virtual eDecorationLayer           getDecorationLayer();
    virtual uint64_t                   getDecorationFlags();
    virtual std::string                getDisplayName();

    PHLWINDOW                          getOwner();
    void                               renderPass(Render::CRenderContext& ctx, PHLMONITOR, float const& a);
    CBox                               assignedBoxGlobal();
    void                               onConfigReloaded();

  private:
    PHLWINDOWREF m_pWindow;
    CBox         m_bAssignedBox;
    int          m_iLastHeight = -1;
};

class CSpiderFooterPassElement : public IPassElement {
  public:
    struct SData {
        CSpiderFooter* deco = nullptr;
        float          a    = 1.F;
    };

    CSpiderFooterPassElement(const SData& data_) : data(data_) {}
    virtual ~CSpiderFooterPassElement() = default;

    virtual std::vector<UP<IPassElement>> draw(Render::CRenderContext& ctx) override;
    virtual bool                          needsLiveBlur(Render::CRenderContext&) override {
        return false;
    }
    virtual bool needsPrecomputeBlur(Render::CRenderContext&) override {
        return false;
    }
    virtual std::optional<CBox> boundingBox(Render::CRenderContext& ctx) override;

    virtual const char*         passName() override {
        return "CSpiderFooterPassElement";
    }
    virtual ePassElementType type() override {
        return EK_CUSTOM;
    }

  private:
    SData data;
};
