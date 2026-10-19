# Slate 1.0 — User Manual

Slate is a gate and expander. It turns the signal down when it falls below a threshold. Typical uses:

- removing bleed, hiss, hum and room sound between notes and hits
- tightening drums
- cutting reverb tails short

Its UPWARD style does the opposite: it lifts what rises above the threshold, for extra punch. Slate has five styles, look‑ahead, mid/side processing, an external sidechain with filters and audition, MIDI note triggering, separate wet and dry levels, and up to 4× oversampling.

Slate runs as VST3, AU, CLAP and VST2 on macOS, Windows and Linux.

**Contents**

1. [Installing](#1-installing)
2. [A quick tour](#2-a-quick-tour)
3. [How a gate works](#3-how-a-gate-works)
4. [Styles](#4-styles)
5. [Main controls](#5-main-controls)
6. [Channels, oversampling and MIDI trigger](#6-channels-oversampling-and-midi-trigger)
7. [Expert mode](#7-expert-mode)
8. [The display and meters](#8-the-display-and-meters)
9. [Knobs](#9-knobs)
10. [A/B, undo and redo](#10-ab-undo-and-redo)
11. [MIDI](#11-midi)
12. [Presets](#12-presets)
13. [Help and hover tips](#13-help-and-hover-tips)
14. [Recipes](#14-recipes)
15. [Parameter reference](#15-parameter-reference)
16. [Troubleshooting](#16-troubleshooting)

---

## 1. Installing

Download the zip for your system from the project's **Releases** page and copy the plugin files:

| System | Copy | To |
|---|---|---|
| macOS | `Slate.vst3`, `Slate.component` (AU), `Slate.clap` | `~/Library/Audio/Plug-Ins/VST3`, `.../Components`, `.../CLAP` |
| Windows | the `Slate.vst3` folder, `Slate.clap` | `C:\Program Files\Common Files\VST3\`, `...\CLAP\` |
| Linux | `Slate.vst3`, `Slate.clap` | `~/.vst3/`, `~/.clap/` |

**macOS:** if the build isn't notarized and macOS blocks it, open Terminal and run:

```
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/*/Slate*
```

Rescan plugins in your DAW. Slate appears under **Epsilon Audio** as a dynamics effect. It has a stereo input, a stereo output and a stereo **sidechain** input, and it accepts MIDI.

---

## 2. A quick tour

![Slate](../screenshot.png)

- **Header:** preset browser, A/B and COPY, UNDO/REDO, LEARN (MIDI Learn) and MIDI on/off, HELP, and the current latency.
- **Level display** (top left), the last ten seconds:
  - the input in grey and the output in blue
  - the level the gate hears as a yellow line
  - the gain change from the top: red where the gate turns the signal down, green where UPWARD boosts it
  - the threshold as a dashed line you can drag
- **Meters:** input, gain change and output.
- **Transfer curve** (top right): input level against output level. The dot shows where the gate is right now.
- **Gate state:** OPEN, CLOSING or CLOSED (or BOOSTING / IDLE in UPWARD style), the gain change in dB and the level the gate hears.
- **Main panel:** style, threshold, range, ratio, knee, lookahead, attack, hold and release, with a description of the current style.
- **Side panel:**
  - channels, link, oversampling, MIDI trigger, display range and EXPERT
  - with EXPERT on, the expert controls

---

## 3. How a gate works

Slate listens to the signal (or a sidechain) and compares its level with the **threshold**.

- **Above the threshold:** the gate is open and the signal passes untouched.
- **Below the threshold:** the gate turns the signal down.
  - **Ratio** sets how quickly the attenuation grows: 2:1 is a gentle expander, 100:1 a hard gate. For example, at 2:1, a signal 10 dB under the threshold is turned down 10 dB more.
  - **Range** caps how far it goes. A range of 15–20 dB keeps a little background, which often sounds more natural than silence.
- **Timing:**
  - **Attack:** how fast the gate opens. It opens as a smooth fade in level, never a click.
  - **Hold:** how long it stays fully open after the signal drops.
  - **Release:** how fast it closes afterwards, an even fade in dB.
- **Knee:** how gradually it starts to act.
- **Lookahead:** lets the gate open just before a hit arrives.

A fresh instance starts closed, so nothing slips through before the gate has listened.

---

## 4. Styles

| Style | Detector | Character |
|---|---|---|
| **CLASSIC** | RMS, 4 ms | A smooth expander. After a long opening it closes more gently, so sustained sounds fade naturally. |
| **CLEAN** | peak | Fast and precise; timing exactly as you set it. The choice for drums. |
| **VOCAL** | RMS, 12 ms | A softer knee and 4 dB of hysteresis: once open, it closes only 4 dB below the threshold, so breaths and word endings don't make it chatter. Release is slightly slower. |
| **GUITAR** | peak | 8 dB of hysteresis, and it closes faster once a note is dying away. Tight gating for high‑gain guitars. |
| **UPWARD** | RMS, 4 ms | Upward expansion: parts above the threshold are boosted by (level − threshold) × (ratio − 1), up to Range, at most 36 dB. Nothing below the threshold changes. |

For VOCAL and GUITAR the display shows a faint second line where the gate closes again.

---

## 5. Main controls

| Control | Range | What it does |
|---|---|---|
| **THRESHOLD** | −80 – 0 dB | Where the gate opens. Also: drag the dashed line in the display, drag in the transfer curve, or use the mouse wheel over either. |
| **RANGE** | 0 – 80 dB | How far the gate turns the signal down when closed (UPWARD: the most it boosts). |
| **RATIO** | 1:1 – 100:1 | How hard it acts past the threshold. |
| **KNEE** | 0 – 30 dB | How gradually it starts to act. |
| **LOOKAHEAD** | 0 – 10 ms | Opens before a hit arrives, so the first transient isn't cut. Adds that much latency. At 0 ms (and no oversampling) Slate has no latency at all. |
| **ATTACK** | 0.005 – 250 ms | How fast the gate opens. Long attacks make swells. |
| **HOLD** | 0 – 500 ms | How long it stays open after the signal drops below the threshold. |
| **RELEASE** | 5 ms – 5 s | How fast it closes after the hold. |

---

## 6. Channels, oversampling and MIDI trigger

- **STEREO / MID/SIDE:** gate left and right, or the centre (mid) and the width (side) of the stereo image.
- **LINK** (0–100%): how much the two channels share the decision. At 100% both open together when either is loud, so the stereo image never shifts. At 0% each channel (or mid and side) is gated on its own.
- **OVERSAMPLING** (OFF, 2X, 4X): applies the gain at 2× or 4× the sample rate with linear‑phase filters. Very fast attacks and releases on bright material sound cleaner. Adds 32 or 48 samples of latency.
- **MIDI TRIGGER:** MIDI notes open the gate.
  - While any note is held, the gate is fully open. When you let go, it closes with its hold and release.
  - Set the threshold to 0 dB to play the gate purely from MIDI: rhythmic gating from a MIDI clip, or live from a keyboard.
- **DISPLAY RANGE:** 30, 60 or 90 dB for the level display, meters and transfer curve.

---

## 7. Expert mode

Click **EXPERT** to show:

- **SIDECHAIN: INTERNAL / EXTERNAL.** EXTERNAL makes the gate listen to the plugin's sidechain input. Route another track to it in your DAW, for example a kick to open a bass, or a hi‑hat pattern to chop a pad.
- **AUDITION:** listen to the signal the gate hears, after the sidechain level and filters. Use it to tune the filters.
- **SC LEVEL** (±36 dB): raises or lowers what the gate hears, so it opens more or less easily, without moving the threshold.
- **HIGH PASS / LOW PASS** with **HP FREQ / LP FREQ:** 48 dB/oct filters on what the gate hears. Examples:
  - a snare gate with a 180 Hz high‑pass ignores kick bleed
  - a kick gate with a 150 Hz low‑pass ignores cymbals
- **WET** (−36 – +12 dB), **DRY** (off – +12 dB) and **WET PAN:**
  - *WET:* the level of the gated signal.
  - *DRY:* mixes in the original signal, time‑aligned, for parallel gating.
  - *WET PAN:* places the gated signal.

---

## 8. The display and meters

- The level display scrolls over ten seconds. Its scale runs from 0 dBFS at the top to the display range at the bottom.
- The yellow detector line is what the gate compares with the threshold: after the sidechain source, level and filters, and after linking.
- The meters show input peak, gain change and output peak; the numbers above them hold the peaks. Click the meters to reset them.

---

## 9. Knobs

- **Drag** up or down; **Shift** for fine adjustment. **Grab the rim** of a knob to turn it in a circle.
- **Mouse wheel** turns a knob.
- **Double‑click** to type a value, for example `-35`, `4` (for 4:1), `250ms`, `1.5s`, `8k`. For DRY, type `off`; for PAN, `C`, `L30` or `R30`.
- **Cmd/Ctrl‑click** resets to the default.

---

## 10. A/B, undo and redo

- **A / B** hold two complete settings. Click the other letter to switch; the first time, B starts as a copy of A.
- **COPY** copies the current setting into the other slot.
- Every edit is an undo step, including preset loads and A/B switches.
  - Undo: Cmd/Ctrl‑Z
  - Redo: Cmd/Ctrl‑Shift‑Z or Cmd/Ctrl‑Y
- EXPERT and DISPLAY RANGE are view settings: they are saved with your project, but presets, A/B and undo don't change them.

---

## 11. MIDI

- **MIDI TRIGGER:** see section 6.
- **MIDI Learn:**
  1. Click **LEARN**.
  2. Click a control.
  3. Move a knob or fader on your controller.
  4. Click **LEARN** again (or press Esc) to finish.

  Right‑click a control while learning to unassign it. Assignments are saved with your project.
- The **MIDI** button switches all MIDI handling off: triggering and learned controllers.

---

## 12. Presets

32 factory presets in five groups, plus your own:

| Group | Presets |
|---|---|
| Drums | Kick Gate, Snare Gate, Tom Gate, Hi‑Hat Clean‑Up, Overhead Expander, Room Reducer, Gated Reverb |
| Vocals | Vocal Gate, Breath Softener, Podcast Voice, Voice‑Over, Backing Vocals |
| Instruments | High‑Gain Guitar, Metal Chug, Clean Guitar, Bass DI, Synth Tails, Piano Noise Floor |
| Mix & Repair | Hiss Reducer, Hum Reducer, Bleed Control, Parallel Gate, Gentle Expander, Mid/Side Expander |
| Creative | Upward Punch, Live Drum Lift, Key Gate (SC), Trance Gate (SC), MIDI Gate, MIDI Stutter, Slow Swell |

Thresholds depend on how loud your material is, so adjust THRESHOLD first. The (SC) presets expect a signal on the sidechain input. The MIDI presets expect MIDI notes.

- **SAVE CURRENT AS…** stores your own presets, in one of these folders:
  - macOS: `~/Library/Audio/Presets/Epsilon Audio/Slate`
  - Windows: `%APPDATA%\Epsilon Audio\Slate\Presets`
  - Linux: `~/.config/epsilon-audio/slate/presets`
- To delete one of your presets, right‑click it in the browser and click again to confirm.

---

## 13. Help and hover tips

- Rest the pointer on any control, the display, the meters, the transfer curve or the gate state to see what it does and its current value. Over the style buttons, the tip describes the style under the pointer.
- **HELP** opens the in‑plugin pages. The switch at the bottom of the help window turns tips on or off.
- **FULL MANUAL (WEB)** opens this manual.

---

## 14. Recipes

- **Snare without kick and hat bleed:**
  1. CLEAN style, ratio 100:1, range 30–40 dB.
  2. Expert: high‑pass at about 180 Hz, low‑pass at about 6 kHz.
  3. Lookahead 2 ms, hold 30 ms, release around 90 ms.
  4. Set the threshold just below the softest snare hits.
- **Natural vocal clean‑up:** VOCAL style, ratio 3–4:1, range 12–18 dB, lookahead 5 ms, hold 80 ms, release 250 ms.
- **Tight metal guitars:** GUITAR style, fast attack, very short hold and release, ratio 100:1, range 60 dB (see *Metal Chug*).
- **Gated reverb:** put Slate after a big reverb on the drums. Set a long hold (200–250 ms) and a very short release (see *Gated Reverb*).
- **Trance gate:** an external sidechain with a rhythmic pattern, ratio 100:1, very short release. Or use MIDI TRIGGER with a MIDI clip (*MIDI Gate*, *MIDI Stutter*).
- **Parallel gating:** gate hard, then bring back some of the original with DRY.
- **More punch:** UPWARD with a low ratio (1.3–1.6:1) and a few dB of range; turn WET down to match levels.

---

## 15. Parameter reference

There are 26 parameters: 24 automatable controls and 2 view settings. New parameters are only ever added at the end, so sessions and automation keep working across versions.

- **Main:**
  - Style
  - Threshold (−80 – 0 dB), Ratio (1–100), Range (0–80 dB), Knee (0–30 dB)
  - Attack (0.005–250 ms), Hold (0–500 ms), Release (5 ms – 5 s), Lookahead (0–10 ms)
- **Side panel:** Oversampling, Channel Mode, Stereo Link, MIDI Trigger, MIDI.
- **Expert:**
  - Sidechain, Sidechain Level (±36 dB)
  - Sidechain High Pass + HP Freq, Sidechain Low Pass + LP Freq (20 Hz – 20 kHz)
  - Sidechain Audition
  - Wet Level (−36 – +12 dB), Dry Level (off – +12 dB), Wet Pan
- **View settings** (not automatable): Expert Mode, Display Range.

---

## 16. Troubleshooting

- **Slate doesn't appear in my DAW.** Check the install folders above and rescan plugins. On macOS, run the `xattr` command from section 1.
- **The gate chatters.** Raise HOLD, lengthen RELEASE, try VOCAL or GUITAR (hysteresis), or soften the KNEE.
- **The first hit is cut.** Add 1–3 ms of LOOKAHEAD, or shorten ATTACK.
- **The external sidechain does nothing.** Route a track to Slate's sidechain input in your DAW and set SIDECHAIN to EXTERNAL in expert mode.
- **MIDI trigger does nothing.** Turn MIDI TRIGGER on, make sure the MIDI button is on, and route MIDI to Slate's track. With a low threshold, the gate is already open from the audio.
- **The latency changed.** Lookahead adds up to 10 ms and oversampling adds 32–48 samples. Your host compensates automatically.
