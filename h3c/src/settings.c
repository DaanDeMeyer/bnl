#include <h3c/settings.h>

const h3c_settings_t h3c_settings_default = {
  .max_header_list_size = H3C_VARINT_MAX,
  .num_placeholders = 0,
  .qpack_max_table_capacity = 0,
  .qpack_blocked_streams = 0
};
