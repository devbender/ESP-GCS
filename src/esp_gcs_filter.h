#pragma once

#include <math.h>
#include <vector>
#include <algorithm>

class esp_gcs_filter_t {

  private:
    uint8_t samples = 5;
    float d_raw, d_maverage, d_median, d_mixed, d_wavg, d_kalmanFilter;
    std::vector<float> data, dataCopy;    
    float wp1=0.4, wp2=0.4, wp3=0.2;

    // Kalman filer parameters
    float _err_measure = 1;
    float _err_estimate = 1;
    float _q = 0.01;
    float _current_estimate;
    float _last_estimate;
    float _kalman_gain;


  public:
    esp_gcs_filter_t(){};
    esp_gcs_filter_t(uint8_t _samples);

    void setSamples(uint8_t _samples);
    void setKFParams(float mea_e, float est_e, float q);
    void setWAParams(float _wp1, float _wp2, float _wp3);

    void add(float measurement);
    float updateKFEstimate(float mea);

    float raw();
    float mavg();
    float wavg();
    float median();
    float kalman();
};