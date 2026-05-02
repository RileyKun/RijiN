#include "link.h"

#if defined(DEV_MODE)
EXPORT u32 gen_flag_debug_mode = 0;
#endif

#if defined(STAGING_MODE)
EXPORT u32 gen_flag_staging_mode = 0;
#endif