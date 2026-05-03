#pragma once

class c_interpolation_manager : public c_base_interpolation_manager{
public:
  bool wants_no_interp(c_internal_base_entity* entity) override;
};

CLASS_EXTERN(c_interpolation_manager, interp_manager);