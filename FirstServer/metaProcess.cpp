#include "metaProcess.h"

void metaProcess::addLoad(int newLoad)
{
    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - LastUpdateTime).count();
    if (duration<UPDATE_DURATION_MIN) 
    {
        LastLoad = newLoad;
        PredictLoad = newLoad;
        PredictLoadChange = 0.0;
        LastUpdateTime = current_time;
        return;
    }
    LastUpdateTime = current_time;
    double sub_Load = newLoad - LastLoad;
    LastLoad = newLoad;
    PredictLoadChange = sub_Load / duration * PRE_PERCENTAGE * 1000.0 + PredictLoadChange * (1.0- PRE_PERCENTAGE);
    if (std::abs(PredictLoadChange) < MIN_PREDICT)
        PredictLoadChange = 0.0;
    double middle_duration = UPDATE_DURATION / 2;
    PredictLoad = std::max(0.0, middle_duration * PredictLoadChange + static_cast<double>(LastLoad));
}
