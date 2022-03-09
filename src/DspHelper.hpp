#pragma once

namespace mck {
    static const double MIN_VALUE_LIN = 1e-10;
    static const double MIN_VALUE_DB = -200.0;

    double RoundValue(double value, int numberOfPlaces = 0);
    
    double LinToDb(double lin);
    double DbToLin(double db);
    double DbToLog(double db, double min = -110.0, double max = 0.0);
    double LogToDb(double val, double min = -110.0, double max = 0.0);

    double Undenormal(double in);
    float Undenormal(float in);

    float CalcMeterLin(float *in, unsigned len);

    float CalcMeterDb(float *in, unsigned len);
}