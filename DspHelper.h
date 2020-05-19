#pragma once

#include <cmath>

namespace mck {
    static const double MIN_VALUE_LIN = 1e-10;
    static const double MIN_VALUE_DB = -200.0;
    
    double LinToDb(double lin);
    double DbToLin(double db);

    double Undenormal(double in);
    float Undenormal(float in);

    float CalcMeterLin(float *in, unsigned len);

    float CalcMeterDb(float *in, unsigned len);
}