#include "Metronome.h"

mck::Metronome::Metronome() : m_tempo(120.0f), m_initialized(false), m_isRunning(false), m_clkSet(false), m_clkLen(24), m_clkCount(0), m_clkIdx(0), m_clkBuf(0), m_print(false), m_numBeats(4), m_sampleIdx(0), m_startIdx(0), m_start(false)
{
    m_clkBuffer = new unsigned[m_clkLen];
    m_beat.beat = 0;
    m_beat.total = 4;
}

mck::Metronome::~Metronome()
{
    delete m_clkBuffer;
}

bool mck::Metronome::Init(unsigned samplerate, unsigned buffersize)
{
    if (m_initialized)
    {
        return false;
    }

    m_bufferSize = buffersize;
    m_sampleRate = samplerate;

    m_initialized = true;

    return true;
}

void mck::Metronome::ProcessSysEx(jack_midi_event_t *event)
{
    if (m_initialized == false)
    {
        return;
    }

    unsigned char type = (event->buffer[0] & 0xff);

    if (type == 0xf8)
    {
        if (m_clkSet)
        {
            m_clkBuffer[m_clkIdx] = event->time - m_clkCount;
        }
        else
        {
            m_clkBuffer[m_clkIdx] = m_clkBuf + event->time - m_clkCount;
        }
        m_clkSet = true;
        m_clkCount = event->time;

        if (m_clkIdx == 0)
        {
            m_startIdx = event->time;
            m_start = true;
        }

        m_clkIdx += 1;

        if (m_clkIdx >= m_clkLen)
        {
            // Calculate Tempo
            unsigned sum = 0;
            for (unsigned j = 0; j < m_clkLen; j++)
            {
                sum += m_clkBuffer[j];
            }
            printf("Sum: %d, SR: %d, BS: %d, ClkBuf: %d\n", sum, m_sampleRate, m_bufferSize, m_clkBuf);
            m_tempo = 60.0 * (double)m_sampleRate / (double)sum;
            printf("Current Tempo is: %f BPM\n", m_tempo);
            m_print = true;
            m_beat.beat = (m_beat.beat + 1) % m_beat.total;
            m_beat.time = event->time;
            printf("Beat is: %d / %d\n", m_beat.beat+1, m_beat.total);

            m_clkIdx = 0;
        }
    }
    else if (type == 0xfa)
    {
        m_isRunning = true;
        m_clkSet = true;
        m_clkIdx = 0;
        m_beat.beat = 0;
        m_beat.time = event->time;
        m_clkCount = event->time;
        printf("Beat is: %d / %d\n", m_beat.beat+1, m_beat.total);
    }
    else if (type == 0xfb)
    {
        m_isRunning = true;
    }
    else if (type == 0xfc)
    {
        m_isRunning = false;
    }
}

void mck::Metronome::EndProcess()
{

    if (m_isRunning)
    {
        if (m_clkSet == false)
        {
            m_clkBuf += m_bufferSize;
        }
        else
        {
            m_clkBuf = m_bufferSize;
        }
    }
    m_clkSet = false;
}

void mck::Metronome::GetRTData(MetroData &mrt)
{
    mrt.bpm = m_tempo;
    mrt.sync = m_isRunning;
    mrt.beat = m_beat.beat;
}