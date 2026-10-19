// Slate — hover tips and the in-plugin help pages. License: MIT.
#pragma once
#include "SlateEngine.hpp"

namespace slate {

static const char* const kManualURL = "https://github.com/natehouk/slate/blob/main/docs/MANUAL.md";

inline const char* styleTip(int s) {
    switch (s) {
    case kClassic: return "CLASSIC: a smooth RMS expander. After a long opening it closes a little more gently, so sustained sounds fade naturally.";
    case kClean: return "CLEAN: a fast peak detector and timing exactly as you set it. Precise and neutral; the choice for drums.";
    case kVocal: return "VOCAL: a slower RMS detector, a softer knee and 4 dB of hysteresis, so the gate doesn't chatter on breaths and word endings.";
    case kGuitar: return "GUITAR: a peak detector with 8 dB of hysteresis that closes quickly once a note dies away. Tight gating for high-gain guitars.";
    case kUpward: return "UPWARD: upward expansion. Instead of turning quiet parts down, it turns parts above the threshold up, by up to Range. Adds punch and liveliness.";
    default: return "";
    }
}

inline const char* paramTip(uint32_t p) {
    switch (p) {
    case kStyle: return "The gate's character: CLASSIC, CLEAN, VOCAL, GUITAR or UPWARD (upward expansion). Hover a style for details.";
    case kThreshold: return "The level where the gate opens. Below it the signal is turned down (by up to Range); above it the signal passes. You can also drag the dashed line in the display, or use the mouse wheel over the display.";
    case kRatio: return "How fast the attenuation grows below the threshold. 2:1 is a gentle expander; 100:1 is a hard gate. In UPWARD style it sets how much the signal above the threshold is boosted.";
    case kRange: return "The most the gate will turn the signal down when closed (or, in UPWARD style, the most it boosts). A small range keeps some of the background, which often sounds more natural than full silence.";
    case kAttack: return "How fast the gate opens once the signal crosses the threshold. Very short attack keeps transients sharp; a longer attack softens them or makes swells.";
    case kHold: return "How long the gate stays fully open after the signal drops below the threshold, before it starts to close. Stops it from chattering on decaying notes.";
    case kRelease: return "How fast the gate closes after the hold time. Short release cuts tails off; long release lets them fade.";
    case kKnee: return "How gradually the gate starts to act around the threshold. 0 dB is a hard switch; a soft knee makes the transition smoother.";
    case kLookahead: return "Lets the gate see up to 10 ms ahead, so it is already open when a sudden hit arrives and the very first transient isn't cut. Adds that much latency (reported to your host). 0 ms is zero latency.";
    case kOversampling: return "Applies the gate's gain at 2x or 4x the sample rate, with linear-phase filters. Cleaner very fast attacks and releases on high-frequency material; adds 32 or 48 samples of latency.";
    case kChannelMode: return "STEREO gates left and right. MID/SIDE gates the centre and the stereo width separately (unlink to treat them independently).";
    case kLink: return "How much the two channels share the decision. 100%: both open together when either is loud, so the stereo image stays put. 0%: each channel is gated on its own.";
    case kSidechain: return "Expert: INTERNAL listens to the input. EXTERNAL listens to the plugin's sidechain input instead, so another track can open and close the gate (route it in your host).";
    case kScGain: return "Expert: the level of the signal the gate listens to. Raising it opens the gate more easily, without moving the threshold.";
    case kScHpOn: return "Expert: a 48 dB/oct high-pass on the signal the gate listens to, so low-frequency rumble or bleed doesn't open it.";
    case kScHpFreq: return "Expert: the sidechain high-pass frequency.";
    case kScLpOn: return "Expert: a 48 dB/oct low-pass on the signal the gate listens to, so cymbals or hiss don't open it.";
    case kScLpFreq: return "Expert: the sidechain low-pass frequency.";
    case kScListen: return "Expert: listen to the (filtered) signal the gate hears, to tune the sidechain filters.";
    case kWet: return "Expert: level of the gated signal.";
    case kDry: return "Expert: level of the original, ungated signal mixed back in (time-aligned). All the way down is off. Mix them for parallel gating.";
    case kPan: return "Expert: moves the gated (wet) signal left or right.";
    case kMidiTrigger: return "MIDI notes open the gate: while you hold a key, the gate is fully open; let go and it closes (with hold and release). Set the threshold to 0 dB to play the gate purely from MIDI.";
    case kExpert: return "Shows the expert controls: external sidechain, sidechain level and filters, audition, and wet, dry and pan.";
    case kDisplayRange: return "How many dB the level display shows: 30, 60 or 90.";
    case kMidiOn: return "Turns MIDI on or off: note triggering and learned controllers.";
    default: return "";
    }
}

enum TipTarget { kTipUndo, kTipRedo, kTipLearn, kTipMidi, kTipHelp, kTipA, kTipB, kTipCopy, kTipPrev, kTipPresets, kTipNext,
                 kTipDisplay, kTipThresholdLine, kTipMeters, kTipTransfer, kTipState, kNumTipTargets };
inline const char* targetTitle(int t) {
    static const char* const n[kNumTipTargets] = { "Undo", "Redo", "MIDI Learn", "MIDI", "Help", "Setting A", "Setting B", "Copy A/B",
                                                   "Previous preset", "Presets", "Next preset", "Level display", "Threshold", "Meters", "Transfer curve", "Gate state" };
    return n[t];
}
inline const char* targetTip(int t) {
    switch (t) {
    case kTipUndo: return "Steps back through your edits, including preset loads and A/B switches. Cmd/Ctrl-Z.";
    case kTipRedo: return "Re-applies an edit you undid. Cmd/Ctrl-Shift-Z or Cmd/Ctrl-Y.";
    case kTipLearn: return "Click, then click any control and move a knob or fader on your MIDI controller to assign it. Right-click a control to unassign.";
    case kTipMidi: return paramTip(kMidiOn);
    case kTipHelp: return "Opens the help pages: styles, controls, expert mode, MIDI and shortcuts.";
    case kTipA: case kTipB: return "Slate keeps two complete settings, A and B. Click the other letter to switch and compare.";
    case kTipCopy: return "Copies the current setting to the other slot.";
    case kTipPrev: return "Load the previous preset. The mouse wheel over the preset name also steps through presets.";
    case kTipPresets: return "Open the preset browser. A * after the name means you have changed it.";
    case kTipNext: return "Load the next preset.";
    case kTipDisplay: return "The last ten seconds: input in grey, output in colour, the level the gate hears as a yellow line, and the gain change from the top (red: turned down, green: boosted). Drag the dashed threshold line; the mouse wheel moves it too.";
    case kTipThresholdLine: return "Drag up or down to set the threshold. The faint line below it shows where a gate with hysteresis (Vocal, Guitar) closes again.";
    case kTipMeters: return "Input, gain change and output levels. Click to reset the peak readouts.";
    case kTipTransfer: return "Input level (across) against output level (up). The dot shows where the gate is right now. Drag sideways to move the threshold.";
    case kTipState: return "Whether the gate is open, closing or closed right now, and by how much it is turning the signal down (or up).";
    default: return "";
    }
}

struct HelpPage { const char* title; const char* body; };
static const HelpPage kHelpPages[] = {
    { "Overview",
      "Slate is a gate and expander. It turns the signal down when it falls below the threshold: removing bleed, noise and room sound between notes "
      "and hits, tightening drums, or cutting tails short. Its UPWARD style does the opposite, lifting what rises above the threshold.\n\n"
      "Start with a preset, then set the THRESHOLD just above the noise you want to remove (watch the yellow line in the display), "
      "and adjust RANGE, ATTACK, HOLD and RELEASE until it sounds natural.\n\n"
      "Hover over anything for a moment to see what it does." },
    { "Styles",
      "CLASSIC: a smooth RMS expander that closes more gently after long openings.\n"
      "CLEAN: a fast peak detector, timing exactly as set. Precise; the choice for drums.\n"
      "VOCAL: slower detection, softer knee and 4 dB of hysteresis, so breaths and word endings don't make it chatter.\n"
      "GUITAR: peak detection with 8 dB of hysteresis that closes quickly as notes die away.\n"
      "UPWARD: upward expansion. Boosts what rises above the threshold, by up to Range, for punch and life." },
    { "Controls",
      "THRESHOLD: where the gate opens.\nRATIO: how hard it acts below the threshold (2:1 expander, 100:1 gate).\n"
      "RANGE: how far it turns the signal down when closed.\nKNEE: how gradually it starts.\n"
      "ATTACK, HOLD, RELEASE: how fast it opens, how long it stays open after the signal drops, and how fast it closes.\n"
      "LOOKAHEAD: opens before a hit arrives (adds up to 10 ms of latency).\n\n"
      "STEREO or MID/SIDE with LINK, OVERSAMPLING (2x, 4x), and MIDI TRIGGER sit in the right-hand panel." },
    { "Expert mode",
      "Click EXPERT for more controls:\n\n"
      "SIDECHAIN: INTERNAL or EXTERNAL (another track, routed to the plugin's sidechain input, opens the gate).\n"
      "LEVEL: raises or lowers the signal the gate listens to.\n"
      "HIGH PASS and LOW PASS: 48 dB/oct filters on the signal the gate listens to, so only the right part of the spectrum opens it. "
      "AUDITION lets you hear that filtered signal.\n"
      "WET, DRY and PAN: mix the gated and the original signal (parallel gating) and place the gated signal." },
    { "MIDI",
      "MIDI TRIGGER: hold a note and the gate is fully open; release it and the gate closes with its hold and release. "
      "Set the threshold to 0 dB to play the gate entirely from your keyboard or a MIDI clip (see the MIDI Gate and MIDI Stutter presets).\n\n"
      "MIDI LEARN: click LEARN, click a control, move a controller. Right-click unassigns.\n\n"
      "The MIDI button switches all MIDI handling off." },
    { "A/B, undo & shortcuts",
      "A/B holds two complete settings; COPY copies the current one to the other slot. Every edit is an undo step (Cmd/Ctrl-Z, Cmd/Ctrl-Shift-Z).\n\n"
      "Double-click a knob to type a value; Cmd/Ctrl-click resets it; Shift-drag is fine adjustment. "
      "Drag the threshold line in the display or in the transfer curve. Esc closes this window." },
};
static constexpr int kNumHelpPages = sizeof(kHelpPages) / sizeof(kHelpPages[0]);

} // namespace slate
