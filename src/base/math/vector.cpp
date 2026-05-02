#include "../link.h"

float c_vec3::distance(c_vec3 v, bool fast = false){
  return math::abs((*this - v).length(fast));
}

float c_vec3::distance2d(c_vec3 v, bool fast = false){
  return math::abs((*this - v).length_2d(fast));
}

i32 c_vec3::distance_meter(c_vec3 v, bool fast = false){
  return (i32)(distance(v, fast) * 0.0254f);
}

void c_vec3::normalize_angle(){
  x = math::normalize_angle(x);
  y = math::normalize_angle(y);
  z = math::normalize_angle(z);
}

void c_vec4::normalize_angle(){
  x = math::normalize_angle(x);
  y = math::normalize_angle(y);
  z = math::normalize_angle(z);
  w = math::normalize_angle(w);
}