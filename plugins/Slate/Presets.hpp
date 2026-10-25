// Factory presets (each lists only what it changes from the engine defaults). License: MIT.
// Engine defaults: Classic, threshold -40 dB, ratio 10:1, range 40 dB, knee 6 dB, attack 1 ms, hold 50 ms,
// release 150 ms, no look-ahead, no oversampling, stereo, linked, internal sidechain, wet 0 dB, dry off.
#pragma once
#include "SlateEngine.hpp"
#include <algorithm>

namespace slate {

struct PV { uint32_t id; float v; };
struct PresetDef { const char* name; const char* category; const PV* pv; };
struct Preset { const char* name; const char* category; float v[kParamCount]; };

static const char* const kCategories[] = { "Drums", "Vocals", "Instruments", "Mix & Repair", "Creative", "Ducking" };
static constexpr int kNumCategories = 6;

#define END { kParamCount, 0 }
namespace pv {
static const PV init[] = { END };
// ---------------------------------------------------------------- Drums
static const PV kick[]      = { {kStyle,kClean}, {kThreshold,-30}, {kRatio,100}, {kRange,60}, {kAttack,0.05f}, {kHold,40}, {kRelease,120}, {kKnee,0}, {kLookahead,2}, {kScHpOn,1}, {kScHpFreq,40}, {kScLpOn,1}, {kScLpFreq,150}, END };
static const PV snare[]     = { {kStyle,kClean}, {kThreshold,-26}, {kRatio,100}, {kRange,40}, {kAttack,0.05f}, {kHold,30}, {kRelease,90}, {kKnee,0}, {kLookahead,2}, {kScHpOn,1}, {kScHpFreq,180}, {kScLpOn,1}, {kScLpFreq,6000}, END };
static const PV toms[]      = { {kStyle,kClassic}, {kThreshold,-32}, {kRatio,20}, {kRange,30}, {kAttack,0.2f}, {kHold,80}, {kRelease,300}, {kLookahead,3}, {kScHpOn,1}, {kScHpFreq,60}, {kScLpOn,1}, {kScLpFreq,1200}, END };
static const PV hats[]      = { {kStyle,kClean}, {kThreshold,-34}, {kRatio,8}, {kRange,20}, {kAttack,0.1f}, {kHold,10}, {kRelease,60}, {kScHpOn,1}, {kScHpFreq,4000}, END };
static const PV overheads[] = { {kStyle,kClassic}, {kThreshold,-42}, {kRatio,3}, {kRange,12}, {kAttack,1}, {kHold,60}, {kRelease,250}, {kKnee,10}, END };
static const PV roomCut[]   = { {kStyle,kClassic}, {kThreshold,-30}, {kRatio,4}, {kRange,18}, {kAttack,2}, {kHold,20}, {kRelease,120}, {kKnee,8}, END };
static const PV gatedReverb[]= { {kStyle,kClean}, {kThreshold,-34}, {kRatio,100}, {kRange,80}, {kAttack,0.05f}, {kHold,220}, {kRelease,15}, {kKnee,0}, END };
// ---------------------------------------------------------------- Vocals
static const PV vocalGate[] = { {kStyle,kVocal}, {kThreshold,-44}, {kRatio,4}, {kRange,15}, {kAttack,3}, {kHold,80}, {kRelease,250}, {kKnee,8}, {kLookahead,5}, {kScHpOn,1}, {kScHpFreq,120}, END };
static const PV breaths[]   = { {kStyle,kVocal}, {kThreshold,-40}, {kRatio,2.5f}, {kRange,10}, {kAttack,5}, {kHold,60}, {kRelease,200}, {kKnee,12}, {kLookahead,5}, END };
static const PV podcast[]   = { {kStyle,kVocal}, {kThreshold,-48}, {kRatio,3}, {kRange,18}, {kAttack,4}, {kHold,150}, {kRelease,350}, {kKnee,10}, {kLookahead,6}, {kScHpOn,1}, {kScHpFreq,100}, END };
static const PV voiceOver[] = { {kStyle,kVocal}, {kThreshold,-52}, {kRatio,6}, {kRange,30}, {kAttack,2}, {kHold,120}, {kRelease,300}, {kLookahead,8}, END };
static const PV backingVox[]= { {kStyle,kVocal}, {kThreshold,-36}, {kRatio,5}, {kRange,24}, {kAttack,6}, {kHold,40}, {kRelease,180}, {kKnee,6}, END };
// ---------------------------------------------------------------- Instruments
static const PV guitarHigh[]= { {kStyle,kGuitar}, {kThreshold,-48}, {kRatio,30}, {kRange,50}, {kAttack,0.3f}, {kHold,20}, {kRelease,90}, {kKnee,2}, {kLookahead,1}, END };
static const PV chug[]      = { {kStyle,kGuitar}, {kThreshold,-38}, {kRatio,100}, {kRange,60}, {kAttack,0.05f}, {kHold,5}, {kRelease,25}, {kKnee,0}, {kLookahead,2}, END };
static const PV cleanGtr[]  = { {kStyle,kGuitar}, {kThreshold,-56}, {kRatio,4}, {kRange,20}, {kAttack,1}, {kHold,40}, {kRelease,220}, {kKnee,8}, END };
static const PV bassDI[]    = { {kStyle,kClean}, {kThreshold,-46}, {kRatio,6}, {kRange,24}, {kAttack,2}, {kHold,60}, {kRelease,200}, {kKnee,6}, {kScLpOn,1}, {kScLpFreq,800}, END };
static const PV synthTail[] = { {kStyle,kClassic}, {kThreshold,-50}, {kRatio,3}, {kRange,18}, {kAttack,5}, {kHold,100}, {kRelease,600}, {kKnee,12}, END };
static const PV piano[]     = { {kStyle,kClassic}, {kThreshold,-56}, {kRatio,2}, {kRange,10}, {kAttack,5}, {kHold,150}, {kRelease,800}, {kKnee,14}, END };
// ---------------------------------------------------------------- Mix & Repair
static const PV hissDown[]  = { {kStyle,kClassic}, {kThreshold,-58}, {kRatio,2}, {kRange,8}, {kAttack,3}, {kHold,50}, {kRelease,300}, {kKnee,12}, {kScHpOn,1}, {kScHpFreq,3000}, END };
static const PV humDown[]   = { {kStyle,kClassic}, {kThreshold,-50}, {kRatio,3}, {kRange,15}, {kAttack,5}, {kHold,100}, {kRelease,400}, {kScHpOn,1}, {kScHpFreq,250}, END };
static const PV bleed[]     = { {kStyle,kClean}, {kThreshold,-36}, {kRatio,6}, {kRange,20}, {kAttack,0.5f}, {kHold,60}, {kRelease,200}, {kLookahead,3}, END };
static const PV parallel[]  = { {kStyle,kClean}, {kThreshold,-28}, {kRatio,100}, {kRange,80}, {kAttack,0.05f}, {kHold,20}, {kRelease,60}, {kDry,-6}, {kWet,-3}, END };
static const PV gentleExp[] = { {kStyle,kClassic}, {kThreshold,-45}, {kRatio,1.5f}, {kRange,6}, {kAttack,10}, {kHold,0}, {kRelease,400}, {kKnee,20}, END };
static const PV msSide[]    = { {kStyle,kClassic}, {kChannelMode,1}, {kLink,0}, {kThreshold,-50}, {kRatio,4}, {kRange,12}, {kAttack,3}, {kHold,40}, {kRelease,250}, {kKnee,10}, END };
// ---------------------------------------------------------------- Creative
static const PV punch[]     = { {kStyle,kUpward}, {kThreshold,-24}, {kRatio,1.6f}, {kRange,6}, {kAttack,0.5f}, {kHold,10}, {kRelease,80}, {kKnee,6}, {kWet,-4}, END };
static const PV liveDrums[] = { {kStyle,kUpward}, {kThreshold,-30}, {kRatio,1.4f}, {kRange,8}, {kAttack,1}, {kHold,20}, {kRelease,120}, {kKnee,10}, {kWet,-5}, END };
static const PV duckKey[]   = { {kStyle,kClean}, {kSidechain,1}, {kThreshold,-30}, {kRatio,100}, {kRange,40}, {kAttack,0.1f}, {kHold,30}, {kRelease,90}, END };
static const PV trance[]    = { {kStyle,kClean}, {kSidechain,1}, {kThreshold,-24}, {kRatio,100}, {kRange,60}, {kAttack,0.05f}, {kHold,60}, {kRelease,10}, {kKnee,0}, END };
static const PV midiGate[]  = { {kStyle,kClean}, {kMidiTrigger,1}, {kThreshold,0}, {kRatio,100}, {kRange,80}, {kAttack,0.5f}, {kHold,0}, {kRelease,40}, {kKnee,0}, END };
static const PV stutter[]   = { {kStyle,kClean}, {kMidiTrigger,1}, {kThreshold,0}, {kRatio,100}, {kRange,80}, {kAttack,0.05f}, {kHold,0}, {kRelease,5}, {kKnee,0}, END };
static const PV swell[]     = { {kStyle,kClassic}, {kThreshold,-40}, {kRatio,100}, {kRange,60}, {kAttack,120}, {kHold,0}, {kRelease,40}, {kKnee,0}, END };
// ---------------------------------------------------------------- Ducking (2.0): the sidechain turns the signal down
static const PV voDuck[]    = { {kStyle,kDucking}, {kSidechain,1}, {kThreshold,-38}, {kRatio,4}, {kRange,12}, {kAttack,15}, {kHold,150}, {kRelease,500}, {kKnee,8}, {kScHpOn,1}, {kScHpFreq,120}, END };
static const PV bedDuck[]   = { {kStyle,kDucking}, {kSidechain,1}, {kThreshold,-42}, {kRatio,3}, {kRange,18}, {kAttack,40}, {kHold,300}, {kRelease,900}, {kKnee,12}, END };
static const PV kickDuck[]  = { {kStyle,kDucking}, {kSidechain,1}, {kThreshold,-30}, {kRatio,100}, {kRange,14}, {kAttack,0.5f}, {kHold,10}, {kRelease,140}, {kKnee,0}, {kScLpOn,1}, {kScLpFreq,150}, END };
static const PV pump[]      = { {kStyle,kDucking}, {kSidechain,1}, {kThreshold,-34}, {kRatio,100}, {kRange,30}, {kAttack,0.2f}, {kHold,30}, {kRelease,260}, {kKnee,0}, {kScLpOn,1}, {kScLpFreq,150}, END };
static const PV bassDuck[]  = { {kStyle,kDucking}, {kSidechain,1}, {kThreshold,-28}, {kRatio,8}, {kRange,8}, {kAttack,1}, {kHold,20}, {kRelease,90}, {kKnee,4}, {kScLpOn,1}, {kScLpFreq,120}, END };
static const PV midiDuck[]  = { {kStyle,kDucking}, {kMidiTrigger,1}, {kThreshold,0}, {kRatio,100}, {kRange,40}, {kAttack,1}, {kHold,0}, {kRelease,200}, {kKnee,0}, END };
static const PV selfDuck[]  = { {kStyle,kDucking}, {kThreshold,-14}, {kRatio,2}, {kRange,10}, {kAttack,2}, {kHold,30}, {kRelease,250}, {kKnee,10}, END };
} // namespace pv
#undef END

static const PresetDef kPresetDefs[] = {
    { "Init", "Init", pv::init },
    { "Kick Gate", "Drums", pv::kick }, { "Snare Gate", "Drums", pv::snare }, { "Tom Gate", "Drums", pv::toms },
    { "Hi-Hat Clean-Up", "Drums", pv::hats }, { "Overhead Expander", "Drums", pv::overheads }, { "Room Reducer", "Drums", pv::roomCut },
    { "Gated Reverb", "Drums", pv::gatedReverb },
    { "Vocal Gate", "Vocals", pv::vocalGate }, { "Breath Softener", "Vocals", pv::breaths }, { "Podcast Voice", "Vocals", pv::podcast },
    { "Voice-Over", "Vocals", pv::voiceOver }, { "Backing Vocals", "Vocals", pv::backingVox },
    { "High-Gain Guitar", "Instruments", pv::guitarHigh }, { "Metal Chug", "Instruments", pv::chug }, { "Clean Guitar", "Instruments", pv::cleanGtr },
    { "Bass DI", "Instruments", pv::bassDI }, { "Synth Tails", "Instruments", pv::synthTail }, { "Piano Noise Floor", "Instruments", pv::piano },
    { "Hiss Reducer", "Mix & Repair", pv::hissDown }, { "Hum Reducer", "Mix & Repair", pv::humDown }, { "Bleed Control", "Mix & Repair", pv::bleed },
    { "Parallel Gate", "Mix & Repair", pv::parallel }, { "Gentle Expander", "Mix & Repair", pv::gentleExp }, { "Mid/Side Expander", "Mix & Repair", pv::msSide },
    { "Upward Punch", "Creative", pv::punch }, { "Live Drum Lift", "Creative", pv::liveDrums }, { "Key Gate (SC)", "Creative", pv::duckKey },
    { "Trance Gate (SC)", "Creative", pv::trance }, { "MIDI Gate", "Creative", pv::midiGate }, { "MIDI Stutter", "Creative", pv::stutter },
    { "Slow Swell", "Creative", pv::swell },
    // 2.0 (appended, so the program numbers of the presets above stay the same)
    { "Voice-Over Duck (SC)", "Ducking", pv::voDuck }, { "Music Bed Duck (SC)", "Ducking", pv::bedDuck }, { "Kick Duck (SC)", "Ducking", pv::kickDuck },
    { "Pump (SC)", "Ducking", pv::pump }, { "Bass Under Kick (SC)", "Ducking", pv::bassDuck }, { "MIDI Duck", "Ducking", pv::midiDuck },
    { "Peak Tamer", "Ducking", pv::selfDuck },
};
static constexpr uint32_t kNumPresets = sizeof(kPresetDefs) / sizeof(kPresetDefs[0]);

inline const Preset* presets() {
    static Preset table[kNumPresets];
    static bool done = false;
    if (done) return table;
    done = true;
    Engine* e = new Engine();
    for (uint32_t i = 0; i < kNumPresets; ++i) {
        table[i].name = kPresetDefs[i].name; table[i].category = kPresetDefs[i].category;
        std::copy(e->p, e->p + kParamCount, table[i].v);
        for (const PV* x = kPresetDefs[i].pv; x->id != kParamCount; ++x) table[i].v[x->id] = x->v;
    }
    delete e;
    return table;
}

} // namespace slate
