#pragma once

// NOTE:
// It would be real nice to just call localplayer->PhysicsSimulate since its doing exactly what we want in here anyway
// Edit: better to rebuild it instead since the command context will always be 1 tick behind, we could copy it but I just dont want to bother
// https://github.com/TheAlePower/TeamFortress2/blob/1b81dded673d49adebf4d0958e52236ecc28a956/tf2_src/game/client/c_baseplayer.cpp#L2304

class c_engine_prediction{
public:
  void run();
  
  void finish(){
    if(!global->did_run_prediction)
      return;

    // Restore the globaldata after createmove
    memcpy(globaldata, &global->old_global_data, sizeof(s_global_data));

    // Restore prediction seed.
    *global->prediction_random_seed = global->old_prediction_seed;

    global->did_run_prediction = false;
  }
};

CLASS_EXTERN(c_engine_prediction, engine_prediction);