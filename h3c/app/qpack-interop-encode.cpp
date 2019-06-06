#include <algorithm>
#include <fstream>
#include <vector>

#include <h3c/error.hpp>
#include <h3c/log/fprintf.hpp>
#include <h3c/qpack.hpp>

#include <util.hpp>

static h3c::buffer id_encode(uint64_t id)
{
  h3c::mutable_buffer dest(sizeof(uint64_t));

  dest[0] = static_cast<uint8_t>(id >> 56U);
  dest[1] = static_cast<uint8_t>(id >> 48U);
  dest[2] = static_cast<uint8_t>(id >> 40U);
  dest[3] = static_cast<uint8_t>(id >> 32U);
  dest[4] = static_cast<uint8_t>(id >> 24U);
  dest[5] = static_cast<uint8_t>(id >> 16U);
  dest[6] = static_cast<uint8_t>(id >> 8U);
  dest[7] = static_cast<uint8_t>(id >> 0U);

  return std::move(dest);
}

static h3c::buffer size_encode(const std::vector<h3c::header> &headers,
                               const h3c::qpack::encoder &qpack,
                               std::error_code &ec)
{
  size_t encoded_size = qpack.prefix_encoded_size();

  for (const auto &header : headers) {
    encoded_size += qpack.encoded_size(header, ec);
    if (ec) {
      return {};
    }
  }

  h3c::mutable_buffer dest(sizeof(uint32_t));

  dest[0] = static_cast<uint8_t>(encoded_size >> 24U);
  dest[1] = static_cast<uint8_t>(encoded_size >> 16U);
  dest[2] = static_cast<uint8_t>(encoded_size >> 8U);
  dest[3] = static_cast<uint8_t>(encoded_size >> 0U);

  return std::move(dest);
}

void write(std::ostream &dest, const h3c::buffer &src)
{
  dest.write(reinterpret_cast<const char *>(src.data()),
             static_cast<int32_t>(src.size()));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

int main(int argc, char *argv[])
{
  if (argc < 3) {
    return 1;
  }

  h3c::logger logger{ h3c::log::impl::fprintf() };

  std::ifstream input;
  input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // Open input file

  try {
    input.open(argv[1], std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    H3C_LOG_ERROR(&logger, "Error opening input file: {}", e.what());
    return 1;
  }

  // `std::getline` sets `failbit` when it doesn't read any characters.
  input.exceptions(std::ifstream::badbit);

  std::ofstream output;
  output.exceptions(std::ofstream::failbit | std::ofstream::badbit);

  // Open output file

  try {
    output.open(argv[2], std::ios::trunc | std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    H3C_LOG_ERROR(&logger, "Error opening output file: {}", e.what());
    return 1;
  }

  // Read input

  std::string line;
  uint64_t id = 1;
  std::vector<h3c::header> headers;

  h3c::qpack::encoder qpack(&logger);
  std::error_code ec;

  while (std::getline(input, line)) {
    if (line.empty()) {
      if (headers.empty()) {
        continue;
      }

      // Encode header block and write to output

      try {
        write(output, id_encode(id));
        h3c::buffer size_encoded = TRY(size_encode(headers, qpack, ec));
        write(output, size_encoded);
        write(output, qpack.prefix_encode());

        for (const h3c::header &header : headers) {
          h3c::buffer encoded = TRY(qpack.encode(header, ec));
          write(output, encoded);
        }
      } catch (const std::ios_base::failure &e) {
        H3C_LOG_ERROR(&logger, "Error writing to output file: {}", e.what());
        return 1;
      }

      id++;
      headers.clear();

      continue;
    }

    if (line[0] == '#') {
      continue;
    }

    size_t i = line.find('\t');
    if (i == std::string::npos) {
      H3C_LOG_ERROR(&logger, "Missing TAB character");
      return 1;
    }

    h3c::mutable_buffer name(i);
    std::copy_n(line.substr(0, i).begin(), name.size(), name.begin());

    h3c::mutable_buffer value(line.size() - (i + 1));
    std::copy_n(line.substr(i + 1).begin(), value.size(), value.begin());

    headers.emplace_back(h3c::header{ std::move(name), std::move(value) });
  }

  return 0;
}

#pragma GCC diagnostic pop
