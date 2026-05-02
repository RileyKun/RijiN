#pragma once

class c_procedure_gfxtest_dx9 : public c_procedure_base{
public:

  bool setup() override{
    if(global->dxgi_offset_data == nullptr)
      return false;

    return true;
  }

  bool write_signatures() override{
    return true;
  }

  bool write_pointers() override{
    return true;
  }

  bool write_hooks() override{
    set_section(HASH(".text"));

    set_module(HASH("d3d9.dll"));
    {
      add_hook(HASH("d3d9_hook"), global->dxgi_offset_data->d3d9_present);
    }

    return true;
  }
};