#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/render/Texture.hpp>
#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/config/values/types/StringValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/FontWeightValue.hpp>

#include <optional>

inline HANDLE PHANDLE = nullptr;

struct SHyprButton {
    std::string          cmd     = "";
    bool                 userfg  = false;
    CHyprColor           fgcol   = CHyprColor(0, 0, 0, 0);
    CHyprColor           bgcol   = CHyprColor(0, 0, 0, 0);
    float                size    = 10;
    std::string          icon    = "";
    SP<Render::ITexture> iconTex;
    std::optional<float> m_fIconScale;
};

class CHyprBar;
class CSpiderFooter;
class CSpiderOutline;

struct SGlobalState {
    std::vector<SHyprButton>  buttons;
    std::vector<WP<CHyprBar>> bars;
    std::vector<WP<CSpiderFooter>> footers;
    std::vector<WP<CSpiderOutline>> outlines;
    uint32_t                  nobarRuleIdx      = 0;
    uint32_t                  barColorRuleIdx   = 0;
    uint32_t                  titleColorRuleIdx = 0;

    struct {
        SP<Config::Values::CColorValue>      barColor, textColor, inactiveButtonColor;
        SP<Config::Values::CIntValue>        barHeight;
        SP<Config::Values::CIntValue>        barTextSize;
        SP<Config::Values::CFontWeightValue> barTextWeight;
        SP<Config::Values::CIntValue>        barPadding;
        SP<Config::Values::CIntValue>        barButtonPadding;
        SP<Config::Values::CBoolValue>       barBlur, barTitleEnabled, barPartOfWindow, barPrecedenceOverBorder, enabled, iconOnHover, buttonsOnHover;
        SP<Config::Values::CStringValue>     barTextFont, barTextAlign, barButtonsAlignment, onDoubleClick;
        // SpiderOS
        SP<Config::Values::CBoolValue>       spiderStyle;
        SP<Config::Values::CColorValue>      spiderBoxColor, spiderAccentColor, spiderIconColor;
        SP<Config::Values::CIntValue>        spiderAccentWidth;
        SP<Config::Values::CColorValue>      spiderHoverColor, spiderCloseHoverColor;
        SP<Config::Values::CIntValue>        spiderIconSize;
        SP<Config::Values::CFontWeightValue> spiderIconWeight;
        SP<Config::Values::CStringValue>     spiderAppIcons;
        SP<Config::Values::CBoolValue>       spiderSquareTop;
        SP<Config::Values::CIntValue>        spiderBarRounding;
        SP<Config::Values::CIntValue>        spiderFooterHeight;
        SP<Config::Values::CColorValue>      spiderFooterColor;
        SP<Config::Values::CStringValue>     spiderFooterText;
        SP<Config::Values::CColorValue>      spiderFooterTextColor;
        SP<Config::Values::CIntValue>        spiderFooterTextSize;
        SP<Config::Values::CFontWeightValue> spiderFooterTextWeight;
        SP<Config::Values::CIntValue>        spiderOutlineSize;
        SP<Config::Values::CColorValue>      spiderOutlineColor;
        SP<Config::Values::CIntValue>        spiderOutline2Size;
        SP<Config::Values::CColorValue>      spiderOutline2Color;
    } config;
};

inline UP<SGlobalState> g_pGlobalState;
