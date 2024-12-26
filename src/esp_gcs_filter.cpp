#include "esp_gcs_filter.h"


esp_gcs_filter_t::esp_gcs_filter_t(uint8_t _samples) {
      samples = _samples;
}


void esp_gcs_filter_t::setSamples(uint8_t _samples) {
  samples = _samples;
}


void esp_gcs_filter_t::setKFParams(float mea_e, float est_e, float q) {
  _err_measure=mea_e;
  _err_estimate=est_e;
  _q = q;
}


void esp_gcs_filter_t::setWAParams(float _wp1, float _wp2, float _wp3) {
  wp1=_wp1;
  wp2=_wp2;
  wp3=_wp3;
}


float esp_gcs_filter_t::updateKFEstimate(float mea) {

  _kalman_gain = _err_estimate/(_err_estimate + _err_measure);
  _current_estimate = _last_estimate + _kalman_gain * (mea - _last_estimate);
  _err_estimate =  (1.0 - _kalman_gain)*_err_estimate + fabs(_last_estimate-_current_estimate)*_q;
  _last_estimate=_current_estimate;

  return _current_estimate;
}


void esp_gcs_filter_t::add(float newSample) {

  // store raw sample
  d_raw = newSample;

  // add sample to pool
  data.push_back( newSample );

  // delete oldest
  if( data.size() > samples )
    data.erase( data.begin() );

  
  // Average ----------------------------------
  float sumSamples = 0;

  for(auto& sample : data)
    sumSamples+=sample;

  d_maverage = round( sumSamples / data.size() );

  // Median -----------------------------------
  if(data.size() >= samples) {       

    dataCopy = data;
    std::sort(dataCopy.begin(), dataCopy.end());
    d_median = dataCopy[ round(dataCopy.size()/2) ];
  }
  
  // Kalman -----------------------------------
  d_kalmanFilter = updateKFEstimate( newSample );

  // Weighted Avg -----------------------------
  d_wavg = round( (wp1)*d_maverage + (wp2)*d_median + (wp3)*d_kalmanFilter  );
}


float esp_gcs_filter_t::raw() {
  return d_raw;
}


float esp_gcs_filter_t::mavg() {
  return d_maverage;
}


float esp_gcs_filter_t::wavg() {      
  return d_wavg;
}


float esp_gcs_filter_t::median() {
  return d_median;
}


float esp_gcs_filter_t::kalman() {
  return d_kalmanFilter;
}