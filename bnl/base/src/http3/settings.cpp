#include <bnl/http3/settings.hpp>

namespace bnl {
namespace http3 {

std::array<std::pair<uint64_t, uint64_t>, settings::size>
settings::array() const noexcept
{
  {
    return {
      std::make_pair(setting::max_header_list_size, max_header_list_size),
      std::make_pair(setting::num_placeholders, num_placeholders),
      std::make_pair(setting::qpack_max_table_capacity,
                     qpack_max_table_capacity),
      std::make_pair(setting::qpack_blocked_streams, qpack_blocked_streams)
    };
  }
}

}
}
