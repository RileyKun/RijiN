#pragma once

class gcs_component_group_host : public gcs_component_base{
public:
  i32 padding = 12;

  void setup() override{
    padding = render->rescale(12);
  }

  void think(vec3i pos, vec3i size) override{
  }

  void draw(vec3i pos, vec3i size){
  }

  void update(){
    if(children().empty())
      return;

    assert(padding > 0);

    u32 number_of_groups  = children().size();
    u32 group_count       = 0;
    for(gcs_component_group* group : children()){
      if(group == nullptr)
        continue;

      if(number_of_groups <= 2){
        vec3i group_padding = vec3i(padding, padding);
        vec3i group_size = vec3i((size().x - (group_padding.x * 3)) / 2, size().y - (group_padding.x * 2));

        group->set_size(group_size);

        if(group_count == 0)
          group->set_pos(group_padding);
        else if(group_count == 1)
          group->set_pos(group_padding + vec3i(group_size.x + group_padding.x, 0));
      }
      else if(number_of_groups >= 3){
        vec3i group_padding = vec3i(padding, padding);
        vec3i group_size = vec3i((size().x - (group_padding.x * 3)) / 2, (size().y - (group_padding.y * 3)) / 2);

        group->set_size(group_size);

        if(group_count == 0)
          group->set_pos(group_padding);
        else if(group_count == 1)
          group->set_pos(group_padding + vec3i(0, group_size.y + group_padding.y));
        else if(group_count == 2){
          if(number_of_groups == 3){
            group->set_size(vec3i(group_size.x, size().y - (group_padding.x * 2)));
            group->set_pos(group_padding + vec3i(group_size.x + group_padding.x, 0));
          }
          else{
            group->set_pos(group_padding + vec3i(group_size.x + group_padding.x, 0));
          }
        }
        else if(group_count == 3)
          group->set_pos(group_padding + vec3i(group_size.x + group_padding.x, group_size.y + group_padding.y));
      }

      group->update();

      group_count++;
    }
  }

  gcs_component_group* new_group(std::wstring name){
    assert(children().size() <= 4);

    gcs_component_group* group = add<gcs_component_group>();
    group->set_name(name);
    group->info()->enable_auto_positioning();

    return group;
  }
};