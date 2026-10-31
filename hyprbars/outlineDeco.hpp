#pragma once

#define WLR_USE_UNSTABLE

#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>
#include <hyprland/src/render/pass/PassElement.hpp>
#include "globals.hpp"

// SpiderOS: border around the whole window (title bar + window + footer)
class CSpiderOutline : public IHyprWindowDecoration {
  public:
    CSpiderOutline(PHLWINDOW);
    virtual ~CSpiderOutline();

    virtual SDecorationPositioningInfo getPositioningInfo();
    virtual void                       onPositioningReply(const SDecorationPositioningReply& reply);
    virtual void                       draw(Render::CRenderContext& ctx, PHLMONITOR monitor, const float& a, const Render::SWindowRenderPresentation& presentation);
    virtual eDecorationType            getDecorationType();
    virtual void                       updateWindow(PHLWINDOW);
    virtual void                       damageEntire();
    virtual eDecorationLayer           getDecorationLayer();
    virtual uint64_t                   getDecorationFlags();
    virtual std::string                getDisplayName();

    void                               renderPass(Render::CRenderContext& ctx, PHLMONITOR, float const& a);
    CBox                               fullBoxGlobal();
    void                               onConfigReloaded();

  private:
    PHLWINDOWREF m_pWindow;
    CBox         m_bAssignedBox;
    int          m_iLastSize = -1;
};

class CSpiderOutlinePassElement : public IPassElement {
  public:
    struct SData {
        CSpiderOutline* deco = nullptr;
        float           a    = 1.F;
    };

    CSpiderOutlinePassElement(const SData& data_) : data(data_) {}
    virtual ~CSpiderOutlinePassElement() = default;

    virtual std::vector<UP<IPassElement>> draw(Render::CRenderContext& ctx) override;
    virtual bool                          needsLiveBlur(Render::CRenderContext&) override {
        return false;
    }
    virtual bool needsPrecomputeBlur(Render::CRenderContext&) override {
        return false;
    }
    virtual std::optional<CBox> boundingBox(Render::CRenderContext& ctx) override;

    virtual const char*         passName() override {
        return "CSpiderOutlinePassElement";
    }
    virtual ePassElementType type() override {
        return EK_CUSTOM;
    }

  private:
    SData data;
};
