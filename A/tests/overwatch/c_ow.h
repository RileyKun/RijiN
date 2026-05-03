#pragma once

struct Matrix {
	float m11, m12, m13, m14,
		m21, m22, m23, m24,
		m31, m32, m33, m34,
		m41, m42, m43, m44;

	Vector GetCameraVec()
	{
		float	A = m22 * m33 - m32 * m23,
				B = m32 * m13 - m12 * m33,
				C = m12 * m23 - m22 * m13,
				Z = m11 * A + m21 * B + m31 * C;

		if (abs(Z) < 0.0001) { 
			return Vector();
			 };

		float	D = m31 * m23 - m21 * m33,
				E = m11 * m33 - m31 * m13,
				F = m21 * m13 - m11 * m23,
				G = m21 * m32 - m31 * m22,
				H = m31 * m12 - m11 * m32,
				K = m11 * m22 - m21 * m12;

		return Vector(-(A * m41 + D * m42 + G * m43) / Z, -(B * m41 + E * m42 + H * m43) / Z, -(C * m41 + F * m42 + K * m43) / Z);
	}

	bool WorldToScreen(Vector worldPos, Vector *OutPos) {
		float screenX = (m11 * worldPos.x) + (m21 * worldPos.y) + (m31 * worldPos.z) + m41;
		float screenY = (m12 * worldPos.x) + (m22 * worldPos.y) + (m32 * worldPos.z) + m42;
		float screenW = (m14 * worldPos.x) + (m24 * worldPos.y) + (m34 * worldPos.z) + m44;

		float camX = 1366.0f / 2.0f;
		float camY = 768.0f / 2.0f;

		float x = camX + (camX * screenX / screenW);
		float y = camY - (camY * screenY / screenW);

		OutPos->x = x;
		OutPos->y = y;

		return (screenW > 0.001f);
	}
};

struct cmp_singleton {
	uint64_t vtable;
	uint64_t component;
	uint64_t parent;
};

#define OW_TICK_INTERVAL 0.016f

class c_ow {
public:
	NTSTATUS NtOpenThread(PHANDLE ThreadHandle, ACCESS_MASK AccessMask, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId);
	NTSTATUS NtClose(HANDLE Handle);
	NTSTATUS NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
	NTSTATUS NtQueryVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength);
	NTSTATUS NtReadVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PULONG NumberOfBytesRead);
	
	bool init();
	void unload();
	
	void* alloc(int size);
	void free(void* ptr);
	bool read(void* dest, uint64_t source, uint32_t size);
	
	void think();
	
	void frame_think();
	
	bool update_state();
	void add_objects();
	
	void check_cache();
	
	bool is_valid_component(uint64_t component, uint8_t id);
	bool is_valid_parent(uint64_t parent);
	bool is_valid_page(uint64_t address);
	
	bool get_main_thread(CLIENT_ID* client_id);
	
	uint64_t get_decrypted_parent(uint64_t parent);
	uint64_t lookup_entry(uint64_t parent, uint64_t id);
	uint64_t decrypt_entry(uint64_t* entry);
	uint64_t lookup_component(uint64_t parent, uint64_t id);
	
	bool _init;
	bool _failed;
	
	HANDLE _thread_id;
	
	uint64_t _base_address;
	
	uint64_t _user_cmd_func_address;
	
	uint64_t _key_func_address;
	uint64_t _key_func2_address;
  
  uint64_t _glow_handle_player;
  
  uint64_t _mouse_move_func_address;
  
  uint64_t _sens_func_address;
  
  uint64_t _ray_trace;
  uint64_t _ray_add_filter;
  
  uint64_t _skill_struct;
	
	uint64_t _test_key1;
	uint64_t _test_key2;
	uint64_t _test_parent_key;
	
	game_manager_t* _gm;
	bool _thread_lock;
	
	bool _unload_user_cmd;
	
	cmp_singleton _singleton_input;
  
	bool _state_updated;
	bool _reset;
	
	int _game_tickcount;
	
	LARGE_INTEGER _process_time;
	uint32_t _sample_time;
	
	input_cmd* _input_cmd;
	server_cmd* _server_cmd;
	pseudo_cmd _cmd;
	uint32_t _last_buttons;
	
	uint32_t _move_seed;
	
	uint64_t _test_address;
	
	bool _pool_id_valid[65535];
	
	int _black;
	
	bool _weed_boss;
  
  int _call_count;
  int _call_incr;
};

extern c_ow* g_ow;