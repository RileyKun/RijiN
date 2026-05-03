#pragma once

class c_lag_compensation : public c_base_lag_compensation{
public:
  void call_entity_prediction_required(c_internal_base_player* player) override{
    // don't have the prediction class, should be a part of the base.
  }
};

CLASS_EXTERN(c_lag_compensation, lag_compensation);