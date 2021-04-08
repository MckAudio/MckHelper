#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace mck
{
    class WaveInfo
    {
    public:
        bool valid;
        unsigned packIdx;
        unsigned sampleIdx;
        unsigned sampleRate;
        unsigned numChans;
        unsigned lengthMs;
        unsigned lengthSamps;
        std::string name;
        std::string relPath;
        std::string path;
        WaveInfo()
            : valid(false),
              packIdx(0),
              sampleIdx(0),
              sampleRate(0),
              numChans(1),
              lengthMs(0),
              lengthSamps(0),
              name(""),
              relPath(""),
              path(""){}
    };
    void to_json(nlohmann::json &j, const WaveInfo &w);
    void from_json(const nlohmann::json &j, WaveInfo &w);

    class WaveInfoDetail
    {
    public:
        bool valid;
        unsigned packIdx;
        unsigned sampleIdx;
        unsigned sampleRate;
        unsigned numChans;
        unsigned lengthMs;
        unsigned lengthSamps;
        std::string name;
        std::string relPath;
        std::string path;
        std::vector<std::vector<float>> waveForm;
        unsigned waveResolutionUs;
        WaveInfoDetail()
            : valid(false),
              packIdx(0),
              sampleIdx(0),
              sampleRate(0),
              numChans(1),
              lengthMs(0),
              lengthSamps(0),
              name(""),
              relPath(""),
              path(""),
              waveForm(),
              waveResolutionUs(500) {}
    };
    void to_json(nlohmann::json &j, const WaveInfoDetail &w);
    void from_json(const nlohmann::json &j, WaveInfoDetail &w);

    WaveInfo ConvertWaveInfo(WaveInfoDetail &w);
    WaveInfoDetail ConvertWaveInfo(WaveInfo &w);

    namespace helper
    {
        WaveInfo ImportWaveFile(std::string path, unsigned sampleRate, std::vector<std::vector<float>> &output);
        WaveInfoDetail ImportWaveForm(std::string path, unsigned sampleRate, std::vector<std::vector<float>> &output, unsigned resolutionUs = 500);
    }
}