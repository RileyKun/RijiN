#pragma once

class c_game_event;
class c_game_event_listener{
public:
  virtual void snb(){};

  // FireEvent is called by EventManager if event just occurred
  // KeyValue memory will be freed by manager if not needed anymore
  virtual void fire_game_event(c_game_event* event);
};

class c_interface_game_event_manager{
public:

  bool add_listener(std::string name);
  bool find_listener(std::string name);
  void remove_listener();
};