#pragma once

class c_privacy_mode : public c_base_privacy_mode{
public:

  c_misc_privacy_mode_settings* get_settings() override;

};

CLASS_EXTERN(c_privacy_mode, privacy_mode);