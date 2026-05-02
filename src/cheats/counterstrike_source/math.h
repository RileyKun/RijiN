#pragma once


namespace math{
  inline bool intersect_inf_ray_with_sphere(vec3& origin, vec3& fwd, vec3& pos, float radius){
    static float t1 = 0.f;
    static float t2 = 0.f;

    assert(global->intersect_inf_ray_with_sphere_addr != nullptr);
    return utils::call_fastcall64_raw<bool, const vec3&, const vec3&, const vec3&, float, float*, float*>(global->intersect_inf_ray_with_sphere_addr, origin, fwd, pos, radius, &t1, &t2);
  }
}