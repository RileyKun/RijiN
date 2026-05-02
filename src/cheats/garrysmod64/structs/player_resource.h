class c_player_resource{
public:
  VIRTUAL_TABLE_RW_IMPLEMENTS()

  i32 get_ping(i32 index){
    if(index <= 0 || index > MAXPLAYERS){
      assert(index > 0 && index <= MAXPLAYERS);
      return 0;
    }

    return *(i32*)((uptr)this + (DT_PlayerResource_m_iPing + (sizeof(i32) * index)));
  }

  i32 get_score(i32 index){
    if(index <= 0 || index > MAXPLAYERS){
      assert(index > 0 && index <= MAXPLAYERS);
      return 0;
    }

    return *(i32*)((uptr)this + (DT_PlayerResource_m_iScore + (sizeof(i32) * index)));
  }

  i32 get_deaths(i32 index){
    if(index <= 0 || index > MAXPLAYERS){
      assert(index > 0 && index <= MAXPLAYERS);
      return 0;
    }

    return *(i32*)((uptr)this + (DT_PlayerResource_m_iDeaths + (sizeof(i32) * index)));
  }

  i32 get_armor(i32 index){
    if(index <= 0 || index > MAXPLAYERS){
      assert(index > 0 && index <= MAXPLAYERS);
      return 0;
    }

    return *(i32*)((uptr)this + (DT_PlayerResource_m_iArmor + (sizeof(i32) * index)));
  }

};