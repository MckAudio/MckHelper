#pragma once

#include <jack/midiport.h>
#include <string.h>
#include <algorithm>

namespace mck
{
struct MetroData
{
    double bpm;
    bool sync;
    unsigned bar;
    unsigned beat;
    MetroData() : bpm(0.0), sync(false), bar(0), beat(0) {}
};
struct MetroBeat
{
    unsigned beat;
    unsigned time;
    unsigned total;
    MetroBeat() : beat(0), time(0), total(4) {}
};
class Metronome
{
public:
    Metronome();
    ~Metronome();
    bool Init(unsigned samplerate, unsigned buffersize);
    void ProcessSysEx(jack_midi_event_t *event);
    void EndProcess();
    float GetTempo() { return m_tempo; };
    void GetRTData(MetroData &mrt);
    MetroBeat GetBeat() { return m_beat;};

private:
    bool m_initialized;
    float m_tempo;
    unsigned m_bufferSize;
    unsigned m_sampleRate;

    unsigned m_numBeats;
    bool m_start;
    unsigned m_startIdx;
    unsigned m_sampleIdx;

    bool m_isRunning;
    bool m_print;
    bool m_clkSet;
    unsigned m_clkIdx;
    unsigned m_clkCount;
    unsigned m_clkLen;
    unsigned m_clkBuf;
    unsigned *m_clkBuffer;

    unsigned m_clickHighCount;
    unsigned m_clickLowCount;

    MetroBeat m_beat;
};
}; // namespace mck