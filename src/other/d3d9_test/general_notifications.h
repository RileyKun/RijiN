#pragma once

class c_general_logs{
public:
   c_gcs_component_log_window* player_log = nullptr;
   c_gcs_component_log_window* server_log = nullptr;
};

CLASS_EXTERN(c_general_logs, logs);