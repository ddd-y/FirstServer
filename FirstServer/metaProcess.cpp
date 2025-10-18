#include "metaProcess.h"

void metaProcess::addLoad(int newLoad)
{
    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - LastUpdateTime).count();
    LastUpdateTime = current_time;
    //简单处理一下网络拥塞
    if (duration < UPDATE_DURATION_MIN)
        duration *= 2.0;
    double sub_Load = newLoad - LastLoad;
    LastLoad = newLoad;
    PredictLoadChange = sub_Load / duration * PRE_PERCENTAGE * 1000.0 + PredictLoadChange * (1.0- PRE_PERCENTAGE);
    if (std::abs(PredictLoadChange) < MIN_PREDICT)
        PredictLoadChange = 0.0;
    double middle_duration = UPDATE_DURATION / 2;
    PredictLoad = std::max(0.0, middle_duration * PredictLoadChange + static_cast<double>(LastLoad));
}
