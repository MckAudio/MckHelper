#include <MckHelper/Transport.hpp>

// TRANSPORT COMMAND
void mck::to_json(nlohmann::json &j, const TransportCommand &t)
{
    j["mode"] = t.mode;
    j["tempo"] = t.tempo;
}
void mck::from_json(const nlohmann::json &j, TransportCommand &t)
{
    t.mode = j.at("mode").get<char>();
    t.tempo = j.at("tempo").get<double>();
}
// TRANSPORT STATE
void mck::to_json(nlohmann::json &j, const TransportState &t)
{
    j["state"] = t.state;
    j["tempo"] = t.tempo;
    j["pulseIdx"] = t.pulseIdx;
    j["pulse"] = t.pulse;
    j["nPulses"] = t.nPulses;
    j["pulseLen"] = t.pulseLen;
    j["beat"] = t.beat;
    j["nBeats"] = t.nBeats;
    j["beatLen"] = t.beatLen;
    j["bar"] = t.bar;
    j["barLen"] = t.barLen;
    j["jackTransport"] = t.jackTransport;
}
void mck::from_json(const nlohmann::json &j, TransportState &t)
{
    t.state = j.at("state").get<char>();
    t.tempo = j.at("tempo").get<double>();
    t.pulseIdx = j.at("pulseIdx").get<unsigned>();
    t.pulse = j.at("pulse").get<unsigned>();
    t.nPulses = j.at("nPulses").get<unsigned>();
    t.pulseLen = j.at("pulseLen").get<unsigned>();
    t.beat = j.at("beat").get<unsigned>();
    t.nBeats = j.at("nBeats").get<unsigned>();
    t.beatLen = j.at("beatLen").get<unsigned>();
    t.bar = j.at("bar").get<unsigned>();
    t.barLen = j.at("barLen").get<unsigned>();
    t.jackTransport = j.at("jackTransport").get<char>();
}

static void JackTimebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int newPos, void *arg)
{
    mck::Transport *transport = (mck::Transport *)arg;
    transport->ProcessTimebase(state, nframes, pos, newPos);
}

mck::Transport::Transport()
    : m_isInitialized(false),
      m_buffersize(0),
      m_samplerate(0),
      m_lastPulse(0),
      m_numPulses(24),
      m_beatsPerBar(4),
      m_bar(0),
      m_beat(0),
      m_beatOffset(0),
      m_nextPulse(0),
      m_update(false),
      m_idx(0),
      m_state(TS_IDLE),
      m_cmd(TC_NOTHING),
      m_useJackTransport(false),
      m_isJackTransportMaster(false),
      m_jackTransportMasterSet(false),
      m_oldJackState(JackTransportStopped)
{
}

mck::Transport::~Transport()
{
    if (m_isInitialized)
    {
        m_cmd = TC_STOP;
    }
}
bool mck::Transport::Init(jack_client_t *client, double tempo)
{
    if (m_isInitialized)
    {
        return false;
    }

    if (client == nullptr)
    {
        return false;
    }

    m_buffersize = jack_get_buffer_size(client);
    m_samplerate = jack_get_sample_rate(client);

    CalcData(tempo);

    m_jackClient = client;
    m_isInitialized = true;
    return true;
}

void mck::Transport::Process(jack_port_t *port, jack_nframes_t nframes, TransportState &ts)
{
    if (m_isInitialized == false)
    {
        return;
    }

    if (nframes != m_buffersize) {
        m_buffersize = nframes;
    }

    bool useJack = m_useJackTransport.load();
    bool leadJack = m_isJackTransportMaster.load();

    char state = m_state.load();
    char cmd = m_cmd.load();
    unsigned pulseLen = m_pulseLen.load();

    unsigned char *buffer;
    void *outBuffer = jack_port_get_buffer(port, nframes);
    jack_midi_clear_buffer(outBuffer);
    jack_position_t jackPos;
    jack_transport_state_t jackState = JackTransportStopped;
    if (useJack && m_jackClient != nullptr)
    {
        jackState = jack_transport_query(m_jackClient, &jackPos);

        if (jackPos.valid & JackPositionBBT)
        {
            m_tempo = jackPos.beats_per_minute;
        }

        if (jackState != m_oldJackState)
        {
            switch (jackState)
            {
            case JackTransportStopped:
                cmd = TC_STOP;
                break;
            case JackTransportRolling:
                cmd = TC_START;
                break;
            default:
                cmd = TC_NOTHING;
                break;
            }
            m_cmd = TC_NOTHING;
            m_oldJackState = jackState;
        }
        else
        {
            switch (cmd)
            {
            case TC_START:
            {
                jack_position_t newPos = jackPos;
                newPos.frame = 0;
                jack_transport_reposition(m_jackClient, &newPos);
                jack_transport_start(m_jackClient);
                break;
            }
            case TC_CONTINUE:
                jack_transport_start(m_jackClient);
                break;
            case TC_STOP:
                jack_transport_stop(m_jackClient);
                break;
            default:
                break;
            }
        }
    }

    switch (cmd)
    {
    case TC_START:
        m_bar = 0;
        m_beat = 0;
        m_pulse = 0;
        m_nextPulse = 0;
        state = TS_RUNNING;
        m_cmd = TC_NOTHING;
        // Send MIDI clock command
        buffer = jack_midi_event_reserve(outBuffer, 0, 1);
        buffer[0] = 0xFA;
        break;
    case TC_CONTINUE:
        state = TS_RUNNING;
        m_cmd = TC_NOTHING;
        // Send MIDI clock command
        buffer = jack_midi_event_reserve(outBuffer, 0, 1);
        buffer[0] = 0xFB;
        break;
    case TC_STOP:
        state = TS_IDLE;
        m_cmd = TC_NOTHING;
        // Send MIDI clock command
        buffer = jack_midi_event_reserve(outBuffer, 0, 1);
        buffer[0] = 0xFC;
        break;
    default:
        break;
    }

    m_beatOffset += m_buffersize;
    bool pulseSet = false;

    if (state == TS_RUNNING)
    {
        unsigned samp = 0;

        while (true)
        {
            if (m_nextPulse < m_buffersize)
            {
                samp = m_nextPulse;
                m_pulse += 1;
                pulseSet = true;
                m_lastPulse = samp;
                // Count pulses, beats & bars
                if (m_pulse >= m_numPulses)
                {
                    m_pulse = 0;
                    m_beat += 1;
                    m_beatOffset = samp;
                    if (m_beat >= m_beatsPerBar)
                    {
                        m_beat = 0;
                        m_bar = m_bar.load() + 1;
                    }
                }
                // Send MIDI clock
                buffer = jack_midi_event_reserve(outBuffer, samp, 1);
                buffer[0] = 0xF8;

                // Calc time to next pulse
                m_nextPulse = samp + pulseLen;
            }
            else
            {
                m_nextPulse -= m_buffersize;
                break;
            }
        }

        if (pulseSet == false)
        {
            m_lastPulse += m_buffersize;
        }
    }

    ts.state = state;
    ts.pulseIdx = m_lastPulse;
    ts.pulse = m_pulse;
    ts.nPulses = m_numPulses;
    ts.pulseLen = pulseLen;
    ts.beat = m_beat;
    ts.nBeats = m_beatsPerBar;
    ts.beatLen = ts.pulseLen * ts.nPulses;
    ts.bar = m_bar.load();
    ts.barLen = ts.beatLen * ts.nBeats;
    ts.tempo = m_tempo.load();
    ts.jackTransport = char(useJack) + char(leadJack);

    m_transportState = ts;

    m_state = state;
}

void mck::Transport::ProcessTimebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int newPos)
{
    if (newPos)
    {
        switch (state)
        {
        case JackTransportStopped:
            break;
        case JackTransportStarting:
            break;
        case JackTransportRolling:
            break;
        }
    }

    pos->bar = m_transportState.bar + 1;
    pos->beats_per_bar = m_transportState.barLen;
    pos->beat = m_transportState.beat + 1;
    pos->beat_type = 4;
    pos->beats_per_minute = m_transportState.tempo;
    pos->tick = m_transportState.pulse + 1;
    pos->ticks_per_beat = m_transportState.nPulses;
    pos->valid = JackPositionBBT;
}

bool mck::Transport::ApplyCommand(TransportCommand &cmd)
{
    char state = m_state.load();
    switch (cmd.mode)
    {
    case TC_STOP:
        if (state == TS_RUNNING)
        {
            m_cmd = TC_STOP;
            break;
        }
        break;
    case TC_START:
        if (state == TS_IDLE)
        {
            m_cmd = TC_START;
            break;
        }
        break;
    case TC_CONTINUE:
        if (state == TS_IDLE)
        {
            m_cmd = TC_CONTINUE;
            break;
        }
        break;
    case TC_TEMPO:
        CalcData(cmd.tempo);
        break;
    case TC_BYPASS_JACK:
        SetJackTransport(false, false);
        break;
    case TC_FOLLOW_JACK:
        SetJackTransport(true, false);
        break;
    case TC_LEAD_JACK:
        SetJackTransport(true, true);
        break;
    default:
        return false;
    }
    return true;
}

bool mck::Transport::GetRTData(TransportState &rt)
{
    if (m_isInitialized == false)
    {
        return false;
    }
    rt.bar = m_bar;
    rt.beat = m_beat;
    rt.state = m_state.load();
    rt.nBeats = m_beatsPerBar;
    rt.nPulses = m_numPulses;
    rt.pulse = m_pulse;
    rt.pulseIdx = m_lastPulse;
    rt.tempo = m_tempo.load();

    return true;
}

bool mck::Transport::GetBeat(Beat &b)
{
    if (m_isInitialized == false)
    {
        return false;
    }

    b.num = m_beat;
    b.off = m_beatOffset;

    return true;
}

void mck::Transport::SetJackTransport(bool enable, bool master)
{
    if (enable && master)
    {
        // Set Transport Master
        // Set conditional to not overwrite a current master like ardour
        if (jack_set_timebase_callback(m_jackClient, 1, JackTimebase, this) != 0)
        {
            master = false;
        }
    }
    else
    {
        // Release Transport Master
        jack_release_timebase(m_jackClient);
    }

    m_useJackTransport = enable;
    m_isJackTransportMaster = enable && master;

    return;
}

void mck::Transport::CalcData(double tempo)
{
    m_pulseLen = (unsigned)std::round(((double)m_samplerate / tempo) * (60.0 / (double)m_numPulses));
    m_tempo = tempo;
}