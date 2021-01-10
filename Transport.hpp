#pragma once

#include <nlohmann/json.hpp>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <atomic>

namespace mck
{
    enum TransCmdType
    {
        TC_NOTHING,
        TC_STOP,
        TC_START,
        TC_CONTINUE,
        TC_TEMPO,
        TC_LENGTH
    };

    enum TransState
    {
        TS_IDLE,
        TS_RUNNING,
        TS_PAUSING,
        TS_LENGTH
    };

    struct TransportCommand
    {
        char mode;
        double tempo;
        TransportCommand() : mode(TC_NOTHING), tempo(0.0) {}
    };
    void to_json(nlohmann::json &j, const TransportCommand &t);
    void from_json(const nlohmann::json &j, TransportCommand &t);

    struct TransportState
    {
        char state;
        double tempo;
        unsigned pulseIdx; // sample index of last pulse
        unsigned pulse;
        unsigned nPulses;  // pulses per beat
        unsigned pulseLen; // samples per pulse
        unsigned beat;
        unsigned nBeats;  // beats per bar
        unsigned beatLen; // samples per beat
        unsigned bar;
        unsigned barLen; // samples per bar

        TransportState() : state(TS_IDLE), tempo(0.0), pulse(0), nPulses(0), pulseLen(0), beat(0), nBeats(0), beatLen(0), bar(0), barLen(0) {}
    };
    void to_json(nlohmann::json &j, const TransportState &t);
    void from_json(const nlohmann::json &j, TransportState &t);

    class Transport
    {
    public:
        struct Data
        {
            double bpm;
            unsigned pulse;
            unsigned pulseLen;
            unsigned beat;
            Data() : bpm(0.0), pulse(0), pulseLen(0), beat(0) {}
        };

        struct Beat
        {
            char num;
            char off;
            Beat() : num(0), off(0) {}
        };

        Transport();
        ~Transport();
        bool Init(unsigned samplerate, unsigned buffersize, double tempo);
        void Process(jack_port_t *port, jack_nframes_t nframes, TransportState &ts);

        bool ApplyCommand(TransportCommand &cmd);
        bool GetRTData(TransportState &rt);
        bool GetBeat(Beat &b);

    private:
        void CalcData(double tempo);

        bool m_isInitialized;
        unsigned m_buffersize;
        unsigned m_samplerate;

        std::atomic<char> m_idx;
        std::atomic<bool> m_update;
        Data m_data[2];

        unsigned m_lastPulse;
        unsigned m_beat;                  // 0 -> m_beatsPerBar - 1
        int m_beatOffset;                 // Samples from last, to next beat
        unsigned m_pulse;                 // 0 -> m_numPulses - 1
        const unsigned m_numPulses;       // 24 pulses per beat
        const unsigned m_beatsPerBar;     // 3, 4 (default)
        std::atomic<unsigned> m_pulseLen; // samples per pulse
        std::atomic<double> m_tempo;
        std::atomic<unsigned> m_bar;
        std::atomic<char> m_state;
        std::atomic<char> m_cmd;
        //double m_bpm;

        unsigned m_nextPulse; // Samples till next pulse
    };
}; // namespace mck