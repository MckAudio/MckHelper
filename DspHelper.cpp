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