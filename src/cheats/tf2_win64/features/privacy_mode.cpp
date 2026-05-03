#include "../link.h"

c_privacy_mode* privacy_mode = nullptr;

c_misc_privacy_mode_settings* c_privacy_mode::get_settings(){
  return (c_misc_privacy_mode_settings*)&config->privacy_mode.enabled;
}