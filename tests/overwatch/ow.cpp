#include "link.h"
#include "link.h"

bool s_ow_init = false;

NtOpenThread_fn sys_NtOpenThread;
NtQueryInformationThread_fn sys_NtQueryInformationThread;
NtQuerySystemInformation_fn sys_NtQuerySystemInformation;
NtReadVirtualMemory_fn sys_NtReadVirtualMemory;
NtWriteVirtualMemory_fn sys_NtWriteVirtualMemory;
NtProtectVirtualMemory_fn sys_NtProtectVirtualMemory;
NtQueryVirtualMemory_fn sys_NtQueryVirtualMemory;
NtContinue_fn sys_NtContinue;

CONTEXT s_backup_context;

char s_reg_content[128];
char s_stack[512];

void __fastcall LdrParentRtlInitializeNtUserPfn_hook(EXCEPTION_RECORD* ExceptionRecord, CONTEXT* ContextFrame) {
	if (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {
		if (ExceptionRecord->ExceptionAddress == g_ow->_user_cmd_func_address) {
			// rbx = input component
			// R8 = sequence number
			// R10 = server cmd
			// input + b30 = server cmd
			// r8 = sequence number
			
			uint64_t rbx = (uint64_t)(ContextFrame->Rbx);
			uint64_t r8 = (uint64_t)(ContextFrame->R8);
			uint64_t r10 = (uint64_t)(ContextFrame->R10);
			
			g_ow->_game_tickcount = (int)r8;
			g_ow->_singleton_input.component = rbx;
			g_ow->_input_cmd = (input_cmd*)(rbx + 0x1020);
			g_ow->_server_cmd = (server_cmd*)(r10);
			
			g_ow->_thread_lock = true;
			g_ow->frame_think();
			g_ow->_thread_lock = false;
		}
		
		// doesnt change after game launched
		if (ExceptionRecord->ExceptionAddress == g_ow->_key_func_address) {
			//g_ow->_test_parent_key = (uint64_t)(ContextFrame->Rcx);
			
			ContextFrame->Dr1 = g_ow->_key_func2_address;
		}
		
		if (ExceptionRecord->ExceptionAddress == g_ow->_key_func2_address) {
			g_ow->_test_key1 = *(uint64_t*)((uint64_t)(ContextFrame->Rsp) + 0x38 + 0x8);
			g_ow->_test_key2 = *(uint64_t*)((uint64_t)(ContextFrame->Rsp) + 0x38 + 0x10);

			ContextFrame->Dr1 = g_ow->_sens_func_address;
		}
		
		if (ExceptionRecord->ExceptionAddress == g_ow->_glow_handle_player) {
			g_esp->run_glow(ContextFrame);
/*
			for (int i = 0; i < MAX_OBJECTS; ++i) {
				if (g_esp->_glow_pointers[i] == cmp) {
					uint32_t rgb = g_esp->_glow_rgb[i];
					uint32_t color = 0xFF000000;
					color |= rgb;
					
					ContextFrame->Rdx |= 1<<7;
					
					if (rgb != 0xFFFFFFFF)
						ContextFrame->R8 = color;
					
					break;
				}
			}
*/
		}
    
    if (ExceptionRecord->ExceptionAddress == g_ow->_mouse_move_func_address) {
      g_ow->_call_incr += 1;
      
      uint64_t rdi = (uint64_t)ContextFrame->Rdi;
      input_cmd* cmd = (input_cmd*)(rdi + 0x1020);
      Vector view = Vector(cmd->forward_x, cmd->forward_y, cmd->forward_z);
      
      //DBG("game cmd %i %i %i %i", (int)(view.x*100.0f), (int)(view.y*100.0f), (int)(view.z*100.0f), cmd->buttons);
      
     /* if (g_aim->_aiming && g_aim->_data.type != AIM_TYPE_RAGE && true) {
        g_aim->apply_smooth(view, false);
        Vector new_aim = g_aim->_new_dir.normal();
        
        cmd->forward_x = new_aim.x;
        cmd->forward_y = new_aim.y;
        cmd->forward_z = new_aim.z;
      }*/
    }
    
    if (ExceptionRecord->ExceptionAddress == g_ow->_sens_func_address) {
      float* sens = (float*)(&(ContextFrame->Xmm1));
      
      if (g_trig->_ghost_sens != 0.0f)
        *sens *= g_trig->_ghost_sens;
    }
		
		ContextFrame->EFlags |= (1<<16);
		sys_NtContinue(ContextFrame, FALSE);
	}
}

bool OW_Init_Hooks() { 
	//g_console->print("OW_Init_Hooks", 5.0f);
	
  HMODULE ntdll = (HMODULE)utils::get_module_handle(HASH("ntdll.dll"));
  
  sys_NtOpenThread = utils::get_proc_address(ntdll, HASH("NtOpenThread"));
  sys_NtQueryInformationThread = utils::get_proc_address(ntdll, HASH("NtQueryInformationThread"));
  sys_NtQuerySystemInformation = utils::get_proc_address(ntdll, HASH("NtQuerySystemInformation"));
  sys_NtReadVirtualMemory = utils::get_proc_address(ntdll, HASH("NtReadVirtualMemory"));
  sys_NtWriteVirtualMemory = utils::get_proc_address(ntdll, HASH("NtWriteVirtualMemory"));
  sys_NtProtectVirtualMemory = utils::get_proc_address(ntdll, HASH("NtProtectVirtualMemory"));
  sys_NtQueryVirtualMemory = utils::get_proc_address(ntdll, HASH("NtQueryVirtualMemory"));
  sys_NtContinue = utils::get_proc_address(ntdll, HASH("NtContinue"));
	
  uint64_t imp = (uint64_t)(utils::get_proc_address(ntdll, HASH("KiUserApcDispatcher")));
  
	uint64_t KiUserApcDispatcher = imp;
	
	// KiUserExceptionDispatcher
	uint16_t jz = *(uint16_t*)(KiUserApcDispatcher + 0x23);
	if ((jz & 0xFF) != 0x74)
		return false;
	
	uint64_t mov = KiUserApcDispatcher + 0x23 + 0x2 + (uint64_t)(jz >> 8) + 0x6;
	
	for (int i = 0; i < 16; ++i) {
		if (*(uint16_t*)mov == 0x8b48)
			break;
		
		mov += 1;
	}
	
	if (*(uint16_t*)mov != 0x8b48)
		return false;
	
	uint32_t rel32 = *(uint32_t*)(mov + 0x3);
	uint64_t LdrParentRtlInitializeNtUserPfn = (mov + 0x7 + (uint64_t)rel32) + 0x8;
	
	if (true) {
		DWORD old_protect;
		VirtualProtect(LdrParentRtlInitializeNtUserPfn, 8, PAGE_READWRITE, &old_protect);
		*(uint64_t*)(LdrParentRtlInitializeNtUserPfn) = (uint64_t)LdrParentRtlInitializeNtUserPfn_hook;
		
	//	g_console->print("OW_Init_Hooks: exceptions hooked", 5.0f);
	}
	
	//g_hooks->attach();
	
	//g_console->print("Press RCTRL+DELETE to uninject", 5.0f);
		
	return true;
}

bool OW_Init() {
	if (s_ow_init == false) {
  	if (!OW_Init_Hooks())
  		return false;
    
  	s_ow_init = true;
	}
  
	return true;
}

void OW_Unload() {
  g_ow->unload();
}

void OW_InitX(float a, matrix_t& m) {
	float s;
	float c;
	sincosf(a, &s, &c);
	
	m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = c;
    m[1][2] = s;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = -s;
    m[2][2] = c;
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void OW_InitY(float a, matrix_t& m) {
	float s;
	float c;
	sincosf(a, &s, &c);
	
	m[0][0] = c;
	m[0][1] = 0.0f;
	m[0][2] = -s;
	m[0][3] = 0.0f;
	
	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;
	
	m[2][0] = s;
	m[2][1] = 0.0f;
	m[2][2] = c;
	m[2][3] = 0.0f;
	
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void OW_Transform(Vector vec, matrix_t m, Vector* out) {
	Vector Z = Vector(vec.z, vec.z, vec.z);
	Vector Y = Vector(vec.y, vec.y, vec.y);
	Vector X = Vector(vec.x, vec.x, vec.x);
	
	Vector result = Vector(Z.x * m[2][0] + m[3][0], Z.y * m[2][1] + m[3][1], Z.z * m[2][2] + m[3][2]);
	result = Vector(Y.x * m[1][0] + result.x, Y.y * m[1][1] + result.y, Y.z * m[1][2] + result.z);
	result = Vector(X.x * m[0][0] + result.x, X.y * m[0][1] + result.y, X.z * m[0][2] + result.z);
	
	*out = result;
}