#include "../../../link.h"

NEVERINLINE void c_render_context::set_stencil_state(s_stencil_state* stencil_state){
  if(stencil_state == nullptr){
    assert(false && "set_stencil_state nullptr");
    return;
  }

  global_internal->is_setting_stencils = true;
#if defined(SOURCE_2018)
  STACK_CHECK_START;
  utils::internal_virtual_call<void, s_stencil_state*>(base_interface->set_stencil_state_index, this, stencil_state);
  STACK_CHECK_END;
#else
  set_stencil_enable(stencil_state->enable);
  set_stencil_fail_operation(stencil_state->fail_op);
  set_stencil_zfail_operation(stencil_state->zfail_op);
  set_stencil_pass_operation(stencil_state->pass_op);
  set_stencil_compare_function(stencil_state->compare_func);
  set_stencil_reference_value(stencil_state->reference_value);
  set_stencil_test_mask(stencil_state->test_mask);
  set_stencil_write_mask(stencil_state->write_mask);
#endif
  global_internal->is_setting_stencils  = false;
}