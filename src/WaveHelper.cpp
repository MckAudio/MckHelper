#include <MckHelper/WaveHelper.hpp>
#include <filesystem>
#include <sndfile.h>
#include <samplerate.h>
/*
mck::WaveInfo &mck::WaveInfo::operator=(const WaveInfo &w)
{
    if (this == &w)
    {
        return *this;
    }
    this->valid = w.valid;
    this->packIdx = w.packIdx;
    this->sampleIdx = w.sampleIdx;
    this->sampleRate = w.sampleRate;
    this->numChans = w.numChans;
    this->lengthMs = w.lengthMs;
    this->lengthSamps = w.lengthSamps;
    this->path = w.path;

    return *this;
}*/

void mck::to_json(nlohmann::json &j, const WaveInfo &w)
{
    j["valid"] = w.valid;
    j["packIdx"] = w.packIdx;
    j["sampleIdx"] = w.sampleRate;
    j["sampleRate"] = w.sampleRate;
    j["numChans"] = w.numChans;
    j["lengthMs"] = w.lengthMs;
    j["lengthSamps"] = w.lengthSamps;
    j["name"] = w.name;
    j["path"] = w.path;
    j["relPath"] = w.relPath;
}
void mck::from_json(const nlohmann::json &j, WaveInfo &w)
{
    w.valid = j.at("valid").get<bool>();
    w.packIdx = j.at("packIdx").get<unsigned>();
    w.sampleRate = j.at("sampleIdx").get<unsigned>();
    w.sampleRate = j.at("sampleRate").get<unsigned>();
    w.numChans = j.at("numChans").get<unsigned>();
    w.lengthMs = j.at("lengthMs").get<unsigned>();
    w.lengthSamps = j.at("lengthSamps").get<unsigned>();
    w.name = j.at("name").get<std::string>();
    w.path = j.at("path").get<std::string>();
    w.relPath = j.at("relPath").get<std::string>();
}
/*
mck::WaveInfoDetail &mck::WaveInfoDetail::operator=(const WaveInfoDetail &w)
{
    if (this == &w)
    {
        return *this;
    }
    (WaveInfo) *this = (WaveInfo)w;
    this->waveForm = w.waveForm;
    this->waveResolutionUs = w.waveResolutionUs;

    return *this;
}*/
void mck::to_json(nlohmann::json &j, const WaveInfoDetail &w)
{
    j["valid"] = w.valid;
    j["packIdx"] = w.packIdx;
    j["sampleIdx"] = w.sampleRate;
    j["sampleRate"] = w.sampleRate;
    j["numChans"] = w.numChans;
    j["lengthMs"] = w.lengthMs;
    j["lengthSamps"] = w.lengthSamps;
    j["name"] = w.name;
    j["path"] = w.path;
    j["relPath"] = w.relPath;
    j["waveForm"] = w.waveForm;
    j["waveResolutionUs"] = w.waveResolutionUs;
}
void mck::from_json(const nlohmann::json &j, WaveInfoDetail &w)
{
    w.valid = j.at("valid").get<bool>();
    w.packIdx = j.at("packIdx").get<unsigned>();
    w.sampleRate = j.at("sampleIdx").get<unsigned>();
    w.sampleRate = j.at("sampleRate").get<unsigned>();
    w.numChans = j.at("numChans").get<unsigned>();
    w.lengthMs = j.at("lengthMs").get<unsigned>();
    w.lengthSamps = j.at("lengthSamps").get<unsigned>();
    w.name = j.at("name").get<std::string>();
    w.path = j.at("path").get<std::string>();
    w.relPath = j.at("relPath").get<std::string>();
    w.waveForm = j.at("waveForm").get<std::vector<std::vector<float>>>();
    w.waveResolutionUs = j.at("waveResolutionUs").get<unsigned>();
}

mck::WaveInfo mck::ConvertWaveInfo(WaveInfoDetail &w)
{
    WaveInfo r;
    r.valid = w.valid;
    r.packIdx = w.packIdx;
    r.sampleIdx = w.sampleIdx;
    r.sampleRate = w.sampleRate;
    r.numChans = w.numChans;
    r.lengthMs = w.lengthMs;
    r.lengthSamps = w.lengthSamps;
    r.name = w.name;
    r.path = w.path;
    r.relPath = w.relPath;

    return r;
}
mck::WaveInfoDetail mck::ConvertWaveInfo(WaveInfo &w)
{
    WaveInfoDetail r;
    r.valid = w.valid;
    r.packIdx = w.packIdx;
    r.sampleIdx = w.sampleIdx;
    r.sampleRate = w.sampleRate;
    r.numChans = w.numChans;
    r.lengthMs = w.lengthMs;
    r.lengthSamps = w.lengthSamps;
    r.name = w.name;
    r.path = w.path;
    r.relPath = w.relPath;
    r.waveForm.clear();
    r.waveResolutionUs = 500;

    return r;
}

mck::WaveInfo mck::helper::ImportWaveFile(std::string path, unsigned sampleRate, std::vector<std::vector<float>> &output)
{
    mck::WaveInfo info;
    info.valid = false;

    SNDFILE *snd;
    SF_INFO sndInfo;

    snd = sf_open(path.c_str(), SFM_READ, &sndInfo);
    if (snd == nullptr)
    {
        return info;
    }

    info.numChans = sndInfo.channels;
    info.lengthSamps = sndInfo.frames;
    info.sampleRate = sndInfo.samplerate;
    info.path = path;

    // Malloc Interleaved Buffer
    float *buffer = (float *)malloc(info.lengthSamps * info.numChans * sizeof(float));
    memset(buffer, 0, info.lengthSamps * info.numChans * sizeof(float));

    // Read Samples from File
    unsigned numSrcFrames = sf_readf_float(snd, buffer, sndInfo.frames);

    // Close File
    sf_close(snd);

    // Samplerate conversion
    if (info.sampleRate != sampleRate)
    {
        // Sample Rate Conversion
        double convCoeff = (double)sampleRate / (double)info.sampleRate;
        numSrcFrames = (unsigned)std::ceil((double)info.lengthSamps * convCoeff);
        float *tmpSrcBuf = (float *)malloc(info.numChans * numSrcFrames * sizeof(float));
        memset(tmpSrcBuf, 0, info.numChans * numSrcFrames * sizeof(float));
        SRC_DATA tmpSrc;
        tmpSrc.data_in = buffer;
        tmpSrc.data_out = tmpSrcBuf;
        tmpSrc.input_frames = info.lengthSamps;
        tmpSrc.output_frames = numSrcFrames;
        numSrcFrames = tmpSrc.output_frames;
        tmpSrc.src_ratio = convCoeff;

        int err = src_simple(&tmpSrc, SRC_SINC_BEST_QUALITY, info.numChans);
        if (err != 0)
        {
            std::fprintf(stderr, "Failed to apply samplerate conversion to sample %s:\n%s\nExiting...", path.c_str(), src_strerror(err));
            free(buffer);
            free(tmpSrcBuf);
            return mck::WaveInfo();
        }
        info.lengthSamps = tmpSrc.output_frames_gen;
        free(buffer);
        buffer = tmpSrcBuf;
    }
    info.lengthMs = (unsigned)std::floor(1000.0 * (double)info.lengthSamps / (double)sampleRate);

    // Resize Output Buffer
    output.clear();
    output.resize(info.numChans);
    for (unsigned i = 0; i < info.numChans; i++)
    {
        output[i].resize(info.lengthSamps);
    }

    // Copy Samples
    for (unsigned c = 0; c < info.numChans; c++)
    {
        for (unsigned s = 0; s < info.lengthSamps; s++)
        {
            output[c][s] = buffer[s * info.numChans + c];
        }
    }

    // Free Interleaved Buffer
    free(buffer);
    buffer = nullptr;

    info.valid = true;
    return info;
}
mck::WaveInfoDetail mck::helper::ImportWaveForm(std::string path, unsigned sampleRate, std::vector<std::vector<float>> &output, unsigned resolutionUs)
{
    WaveInfoDetail info;
    if (resolutionUs < 100 || resolutionUs > 10e6)
    {
        return info;
    }

    WaveInfo tmpInfo = ImportWaveFile(path, sampleRate, output);
    info = ConvertWaveInfo(tmpInfo);
    if (info.valid == false)
    {
        return info;
    }
    // 500us
    double divider = (double)10e6 / (double)resolutionUs;
    unsigned sampsPerMs = (unsigned)std::ceil((double)info.sampleRate / divider);
    unsigned lengthMs = (unsigned)std::floor(divider * (double)info.lengthSamps / (double)sampleRate);

    // Read Waveform
    info.waveForm.resize(info.numChans);
    for (unsigned c = 0; c < info.numChans; c++)
    {
        info.waveForm[c].resize(lengthMs, 0.0);
    }
    for (unsigned c = 0; c < info.numChans; c++)
    {
        for (unsigned i = 0; i < lengthMs; i++)
        {
            float wMax = 0.0f;
            float wMin = 0.0f;
            for (unsigned s = i * sampsPerMs; s < std::min((i + 1) * sampsPerMs, info.lengthSamps); s++)
            {
                if (output[c][s] > wMax)
                {
                    wMax = output[c][s];
                }
                else if (output[c][s] < wMin)
                {
                    wMin = output[c][s];
                }
            }
            info.waveForm[c][i] = std::abs(wMin) > wMax ? wMin : wMax;
        }
    }
    info.waveResolutionUs = resolutionUs;
    return info;
}