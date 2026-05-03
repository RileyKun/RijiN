#include "link.h"

EXPORT i32 main(i32 argc, const i8 *argv[]) {

  float observed        = 0.047619;
  float closest_delta   = FLT_MAX;
  i32   found_ranged    = 0;

  for(i32 ranged_crit_random = 1; ranged_crit_random < 10000; ranged_crit_random++){
    float normalized_damage = (float)ranged_crit_random / 3.f;
    float ratio             = normalized_damage + ((float)448 - (float)ranged_crit_random);
    float observed_sim      = normalized_damage / ratio;

    float delta = abs(observed_sim - observed);

    if(delta > closest_delta)
      continue;

    closest_delta = delta;
    found_ranged  = ranged_crit_random;

    if(observed_sim == observed)
      goto finish_loop;
  }

  finish_loop:
  printf("dmg: %i %f\n", found_ranged, closest_delta);

  system("pause");

  return 0;
}
