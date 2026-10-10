// SPDX-License-Identifier: GPL-2.0-or-later
// bbport: languages of the in-game menu (bbport_overlay.cpp). The choice is BbSettings::language
// ("language=" in bbport.ini, "auto" follows the system UI language); English fills any gap.

#pragma once

namespace BbI18n {

enum Lang : int {
    En = 0, Ru, It, Es, Fr, De, Pt, Pl, Tr, Zh, Ja, Ko, LangCount
};

/// Menu strings. Entries with printf specifiers keep the same specifiers in every language.
enum Str : int {
    Title, FpsLine, SecUpscaler, UpOff, UpTaa, Upscaler, NotSupported, InProgress,
    Fsr4Unavailable, Fsr4Fallback, DlssUnavailable, DlssHint, DlssInvertJitter, Fsr411Hint,
    Fsr4Hint, Fsr4AutoExposure, Fsr4InvertJitter, Fsr4CheckHint, Preset, PresetItem, TaaText,
    ActiveRender, StartupPreset, AutoRenderHint, RenderResHint, NativeAaHint, Sharpen,
    Sharpness, SharpnessHint, Jitter, JitterHint, SecReactive, ReactiveOn, ReactiveHint, Scale,
    Threshold, Maximum, ShowMask, ObjectMotion, ObjectMotionHint, ShowMotion, MotionHint,
    SecOutput, OutputRes, OutputHintFixed, OutputHintLive, LiveAuto, LiveOff, LiveOn, LiveRes,
    LiveHint, SecEffects, LodMax, LodGame, LodLower, LodMin, ModelLod, EffChromatic, EffDof,
    EffMotionBlur, EffSsao, EffGameAa, EffDynamicShadows, EffSsr, EffSkipIntro, EffDebugCamera,
    EffDebugMenu, EffectsHint, DebugHint, RestartNeeded, ApplyRestart, SecOther, ShowFps,
    Language, Close, SavedIn, FpsCounter, NamePrompt, ShowHud, HudHint, HudQuadTopLeft,
    HudQuadTopRight, HudQuadBottomLeft, HudQuadBottomRight, HudPosition, HudOpacity, HudScale,
    SecMk, MkEnable, MkHint, MkSensX, MkSensY, MkInvertX, MkInvertY, MkDeadzone, MkSmoothing,
    StrCount
};

/// The string in the current language.
const char* T(Str id);
/// The language in use (the setting, or the system language for "auto").
Lang Current();
/// Native name ("Italiano") and ini code ("it").
const char* Name(int lang);
const char* Code(int lang);
/// The system UI language, English when not translated.
Lang SystemLanguage();

} // namespace BbI18n
