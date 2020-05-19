#include "DspHelper.h"

double mck::LinToDb(double lin)
{
    return std::max(MIN_VALUE_DB, 20.0 * std::log10(lin));
}
double mck::DbToLin(double db)
{
    db = std::pow(10.0, db / 20.0);
    db = db <= MIN_VALUE_LIN ? 0.0 : db;
    return db;
}
double mck::Undenormal(double in)
{
    if (std::fpclassify(in) != FP_NORMAL && std::fpclassify(in) != FP_ZERO)
    {
        return 0;
    }
    return in;
}
float mck::Undenormal(float in)
{
    if (std::fpclassify(in) != FP_NORMAL && std::fpclassify(in) != FP_ZERO)
    {
        return 0;
    }
    return in;
}
float mck::CalcMeterLin(float *in, unsigned len)
{
    float sum = 0.0;
    float tmp = 0.0;

    for (unsigned i = 0; i < len; i++)
    {
        tmp = mck::Undenormal(in[i]);
        sum += tmp * tmp;
    }
    return std::sqrt(sum / len);
}
float mck::CalcMeterDb(float *in, unsigned len)
{
    return LinToDb(CalcMeterLin(in, len));
}