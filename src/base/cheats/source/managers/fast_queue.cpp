#include "../../../link.h"

u32 c_base_fast_queue::think(u32 ping, u32 pop_id){
  const i8*              pop_name = get_pop_name(pop_id);
  c_fast_queue_settings* settings = get_settings();

  if(settings == nullptr || pop_name == nullptr){
    assert(pop_name != nullptr);
    return ping;
  }

  cookie_block_check_return_val(ping);

  if(!settings->enabled)
    return ping;

  u32 name_hash = HASH_RT(pop_name);
  switch(name_hash){
    default:{
      DBG("[-] Unknown region: \"%s\" %i\n", pop_name, ping);
      break;
    }
    case HASH("ams"):
    case HASH("ams4"):
    {
      if(settings->block_netherlands)
        return get_blacklist_ping();

      break;
    }
    case HASH("fra"):
    {
      if(settings->block_frankfurt_germany)
        return get_blacklist_ping();

      break;
    }
    case HASH("lhr"):
    {
      if(settings->block_heathrow_uk)
        return get_blacklist_ping();

      break;
    }
    case HASH("mad"):
    {
      if(settings->block_madrid_spain)
        return get_blacklist_ping();

      break;
    }
    case HASH("par"):
    {
      if(settings->block_paris_france)
        return get_blacklist_ping();

      break;
    }
    case HASH("lux"):
    case HASH("lux1"):
    case HASH("lux2"):
    {
      if(settings->block_luxembourg)
        return get_blacklist_ping();

      break;
    }
    case HASH("sto"):
    case HASH("sto2"):
    {
      if(settings->block_stockholm_sweden)
        return get_blacklist_ping();

      break;
    }
    case HASH("waw"):
    {
      if(settings->block_warsaw_poland)
        return get_blacklist_ping();

      break;
    }
    case HASH("sof"):
    {
      if(settings->block_sofia_bulgaria)
        return get_blacklist_ping();

      break;
    }
    case HASH("hel"):
    {
      if(settings->block_helsinki_finland)
        return get_blacklist_ping();

      break;
    }
    case HASH("vie"):
    {
      if(settings->block_vienna_austria)
        return get_blacklist_ping();

      break;
    }
    case HASH("mln1"):
    {
      if(settings->block_milan_italy)
        return get_blacklist_ping();
    }

    // asia / middle east.
    case HASH("bom"):
    case HASH("bom2"):
    {
      if(settings->block_mumbai_india)
        return get_blacklist_ping();

      break;
    }
    case HASH("maa"):
    case HASH("maa2"):
    {
      if(settings->block_chennai_india)
        return get_blacklist_ping();

      break;
    }
    case HASH("dxb"):
    {
      if(settings->block_dubai_uae)
        return get_blacklist_ping();

      break;
    }
    case HASH("hkg"):
    case HASH("hkg4"):
    {
      if(settings->block_hong_kong)
        return get_blacklist_ping();

      break;
    }
    case HASH("sha"):
    {
      if(settings->block_shanghai_china)
        return get_blacklist_ping();

      break;
    }
    case HASH("can"):
    {
      if(settings->block_guangzhou_china)
        return get_blacklist_ping();

      break;
    }
    case HASH("tsn"):
    {
      if(settings->block_tianjin_china)
        return get_blacklist_ping();

      break;
    }
    case HASH("tyo"):
    case HASH("tyo1"):
    case HASH("tyo2"):
    {
      if(settings->block_tokyo_japan)
        return get_blacklist_ping();

      break;
    }
    case HASH("sgp"):
    {
      if(settings->block_singapore)
        return get_blacklist_ping();

      break;
    }
    case HASH("seo"):
    {
      if(settings->block_seoul_skorea)
        return get_blacklist_ping();

      break;
    }

    // USA
    case HASH("dfw"):
    case HASH("dfw2"):
    case HASH("dfwm"):
    case HASH("msa1"):
    {
      if(settings->block_us_texas)
        return get_blacklist_ping();

      break;
    }
    case HASH("jfk"):
    case HASH("mny1"):
    {
      if(settings->block_us_new_york)
        return get_blacklist_ping();

      break;
    }
    case HASH("atl"):
    {
      if(settings->block_us_atlanta)
        return get_blacklist_ping();

      break;
    }
    case HASH("iad"):
    {
      if(settings->block_us_washington)
        return get_blacklist_ping();

      break;
    }
    case HASH("ord"):
    {
      if(settings->block_us_chicago)
        return get_blacklist_ping();

      break;
    }
    case HASH("lax"):
    {
      if(settings->block_us_california)
        return get_blacklist_ping();

      break;
    }
    case HASH("sea"):
    case HASH("eat"):
    {
      if(settings->block_us_seattle_wshton)
        return get_blacklist_ping();

      break;
    }
    case HASH("okc"):
    {
      if(settings->block_us_oklahoma)
        return get_blacklist_ping();

      break;
    }
    case HASH("msy1"):
    {
      if(settings->block_us_new_orleans)
        return get_blacklist_ping();

      break;
    }
    case HASH("mat1"):
    {
      if(settings->block_us_block_virginia)
        return get_blacklist_ping();

      break;
    }
    case HASH("mmi1"):
    {
      if(settings->block_us_florida)
        return get_blacklist_ping();

      break;
    }
    case HASH("mas1"):
    {
      if(settings->block_us_boston)
        return get_blacklist_ping();

      break;
    }

    // south america!!
    case HASH("gru"):
    {
      if(settings->block_brazil)
        return get_blacklist_ping();

      break;
    }
    case HASH("lim"):
    {
      if(settings->block_peru)
        return get_blacklist_ping();

      break;
    }
    case HASH("scl"):
    {
      if(settings->block_chile)
        return get_blacklist_ping();

      break;
    }
    case HASH("eze"):
    {
      if(settings->block_argentina)
        return get_blacklist_ping();

      break;
    }
    case HASH("jnb"):
    {
      if(settings->block_africa)
        return get_blacklist_ping();

      break;
    }
    case HASH("syd"):
    {
      if(settings->block_australia)
        return get_blacklist_ping();

      break;
    }
  }

  return ping <= settings->max_ping ? math::smallest((i32)ping, (i32)get_whitelist_ping()) : ping;
}

i8* c_base_fast_queue::get_pop_name(u32 pop_id){
  if(!pop_id){
    DBG("[-] c_base_fast_queue::get_pop_name: %i (zero)\n", pop_id);
    return nullptr;
  }

  static i8 pop_str[5];

  pop_str[0] = pop_id >> 16;
  pop_str[1] = pop_id >> 8;
  pop_str[2] = pop_id;
  pop_str[3] = pop_id >> 24;
  pop_str[4] = '\0';

  return pop_str;
}

void c_base_fast_queue::create_menu_settings(gcs_component_group* group, c_fast_queue_settings* settings){
  assert(settings != nullptr && "somes gone fucked the fast queue");
  assert(group != nullptr)
  if(settings == nullptr)
    return;

  gcs::toggle(group, WXOR(L"Fast Queue"), &settings->enabled);
  gcs::slider(group, WXOR(L"Maximum ping"), WXOR(L"ms"), 1, 1000, GCS_SLIDER_TYPE_INT, &settings->max_ping);

  // EU
  gcs::dropdown(group, WXOR(L"Block Europe"), true)
    ->add_option(WXOR(L"Netherlands"), &settings->block_netherlands)
    ->add_option(WXOR(L"Germany"), &settings->block_frankfurt_germany)
    ->add_option(WXOR(L"UK"), &settings->block_heathrow_uk)
    ->add_option(WXOR(L"Spain"), &settings->block_madrid_spain)
    ->add_option(WXOR(L"France"), &settings->block_paris_france)
    ->add_option(WXOR(L"Luxembourg"), &settings->block_luxembourg)
    ->add_option(WXOR(L"Sweden"), &settings->block_stockholm_sweden)
    ->add_option(WXOR(L"Poland"), &settings->block_warsaw_poland)
    ->add_option(WXOR(L"Bulgaria"), &settings->block_sofia_bulgaria)
    ->add_option(WXOR(L"Finland"), &settings->block_helsinki_finland)
    ->add_option(WXOR(L"Austria"), &settings->block_vienna_austria)
    ->add_option(WXOR(L"Italy"), &settings->block_milan_italy);

  // Asia / M-E.
  gcs::dropdown(group, WXOR(L"Block Asia / Middle East"), true)
    ->add_option(WXOR(L"Mumbai, India"), &settings->block_mumbai_india)
    ->add_option(WXOR(L"Chennai, India"), &settings->block_chennai_india)
    ->add_option(WXOR(L"UAE"), &settings->block_dubai_uae)
    ->add_option(WXOR(L"Hong Kong"), &settings->block_hong_kong)
    ->add_option(WXOR(L"Shanghai, China"), &settings->block_shanghai_china)
    ->add_option(WXOR(L"Guangzhou, China"), &settings->block_guangzhou_china)
    ->add_option(WXOR(L"Tianjin, China"), &settings->block_tianjin_china)
    ->add_option(WXOR(L"Japan"), &settings->block_tokyo_japan)
    ->add_option(WXOR(L"Singapore"), &settings->block_singapore)
    ->add_option(WXOR(L"South Korea"), &settings->block_seoul_skorea);

  // USA
  gcs::dropdown(group, WXOR(L"Block North America"), true)
    ->add_option(WXOR(L"Texas"), &settings->block_us_texas)
    ->add_option(WXOR(L"New York"), &settings->block_us_new_york)
    ->add_option(WXOR(L"Atlanta"), &settings->block_us_atlanta)
    ->add_option(WXOR(L"Washington DC"), &settings->block_us_washington)
    ->add_option(WXOR(L"Washington Seattle"), &settings->block_us_seattle_wshton)
    ->add_option(WXOR(L"Chicago"), &settings->block_us_chicago)
    ->add_option(WXOR(L"California"), &settings->block_us_california)
    ->add_option(WXOR(L"Oklahoma"), &settings->block_us_oklahoma)
    ->add_option(WXOR(L"Louisiana"), &settings->block_us_new_orleans)
    ->add_option(WXOR(L"Virginia"), &settings->block_us_block_virginia)
    ->add_option(WXOR(L"Florida"), &settings->block_us_florida)
    ->add_option(WXOR(L"Massachusetts"), &settings->block_us_boston);

  gcs::dropdown(group, WXOR(L"Block South America"), true)
    ->add_option(WXOR(L"Brazil"), &settings->block_brazil)
    ->add_option(WXOR(L"Peru"), &settings->block_peru)
    ->add_option(WXOR(L"Chile"), &settings->block_chile)
    ->add_option(WXOR(L"Argentina"), &settings->block_argentina);

  gcs::dropdown(group, WXOR(L"Block Others"), true)
    ->add_option(WXOR(L"Africa"), &settings->block_africa)
    ->add_option(WXOR(L"Australia"), &settings->block_australia);
}





