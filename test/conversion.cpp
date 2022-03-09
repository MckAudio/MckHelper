#include "doctest/doctest.h"

#include "../src/DspHelper.hpp"

#include <vector>

TEST_CASE("Rounding")
{
    std::vector<std::vector<double>> tests = {
        {0.1, 0, 0.0},
        {0.9, 0, 1.0},
        {0.5, 0, 1.0},
        {2.94, 1, 2.9},
        {2.95, 1, 3.0},
        {2.95, 2, 2.95}
    };

    for(auto &test : tests) {
        CHECK(mck::RoundValue(test[0], test[1]) == test[2]);
    }
}

TEST_CASE("Lin to dB")
{
    std::vector<std::vector<double>> tests = {
        {10.0, 20.0},
        {1.0, 0.0},
        {0.0, -200.0},
        {0.5, -6.021},
        {0.1, -20.0}
    };

    for(auto &test : tests) {
        double ret = mck::RoundValue(mck::LinToDb(test[0]), 3);
        CHECK(ret == test[1]);
    }
}