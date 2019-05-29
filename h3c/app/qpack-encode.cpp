#include <array>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <h3c/error.hpp>
#include <h3c/log/fprintf.hpp>
#include <h3c/qpack.hpp>

#include <util.hpp>

std::error_code
encode(uint8_t *dest,
       size_t size,
       uint64_t stream_id,
       const std::vector<std::pair<std::string, std::string>> &headers,
       size_t *encoded_size,
       const h3c::logger *logger)
{
  *encoded_size = 0;

  if (size < sizeof(uint64_t)) {
    THROW(h3c::error::incomplete);
  }

  dest[0] = static_cast<uint8_t>(stream_id >> 56U);
  dest[1] = static_cast<uint8_t>(stream_id >> 48U);
  dest[2] = static_cast<uint8_t>(stream_id >> 40U);
  dest[3] = static_cast<uint8_t>(stream_id >> 32U);
  dest[4] = static_cast<uint8_t>(stream_id >> 24U);
  dest[5] = static_cast<uint8_t>(stream_id >> 16U);
  dest[6] = static_cast<uint8_t>(stream_id >> 8U);
  dest[7] = static_cast<uint8_t>(stream_id >> 0U);

  dest += sizeof(uint64_t);
  size -= sizeof(uint64_t);
  *encoded_size += sizeof(uint64_t);

  if (size < sizeof(uint32_t)) {
    THROW(h3c::error::incomplete);
  }

  // We don't know the exact header block size yet so we store the offset where
  // it starts so we can fill it in later.
  uint8_t *header_block_offset = dest;

  dest += sizeof(uint32_t);
  size -= sizeof(uint32_t);
  *encoded_size += sizeof(uint32_t);

  size_t header_block_encoded_size = 0;

  size_t prefix_encoded_size = 0;
  std::error_code error = h3c::qpack::prefix::encode(dest, size,
                                                     &prefix_encoded_size,
                                                     logger);
  if (error) {
    return error;
  }

  dest += prefix_encoded_size;
  size -= prefix_encoded_size;
  *encoded_size += prefix_encoded_size;
  header_block_encoded_size += prefix_encoded_size;

  for (const auto &header : headers) {
    h3c::header h3c_header = { { header.first.data(), header.first.size() },
                               { header.second.data(), header.second.size() } };

    size_t header_encoded_size = 0;
    error = h3c::qpack::encode(dest, size, h3c_header, &header_encoded_size,
                               logger);
    if (error) {
      return error;
    }

    dest += header_encoded_size;
    size -= header_encoded_size;
    header_block_encoded_size += header_encoded_size;
    *encoded_size += header_encoded_size;
  }

  // Now that we know the header block encoded size, we fill it in at the offset
  // we stored earlier.
  dest = header_block_offset;

  dest[0] = static_cast<uint8_t>(header_block_encoded_size >> 24U);
  dest[1] = static_cast<uint8_t>(header_block_encoded_size >> 16U);
  dest[2] = static_cast<uint8_t>(header_block_encoded_size >> 8U);
  dest[3] = static_cast<uint8_t>(header_block_encoded_size >> 0U);

  return {};
}

void write(std::ostream &dest, uint8_t *src, size_t size)
{
  dest.write(reinterpret_cast<char *>(src), // NOLINT
             static_cast<int32_t>(size));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

int main(int argc, char *argv[])
{
  if (argc < 3) {
    return 1;
  }

  std::string line;
  uint64_t stream_id = 1;
  std::vector<std::pair<std::string, std::string>> headers;

  std::array<uint8_t, 64000> buffer = {};

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

  while (std::getline(input, line)) {
    if (line.empty()) {
      if (headers.empty()) {
        continue;
      }

      // Encode header block

      size_t encoded_size = 0;
      std::error_code error = encode(buffer.data(), buffer.size(), stream_id,
                                     headers, &encoded_size, &logger);
      if (error) {
        return error.value();
      }

      // Write output

      try {
        write(output, buffer.data(), encoded_size);
      } catch (const std::ios_base::failure &e) {
        H3C_LOG_ERROR(&logger, "Error writing to output file: {}", e.what());
        return 1;
      }

      stream_id++;
      headers.clear();

      continue;
    }

    if (line[0] == '#') {
      continue;
    }

    size_t i = line.find('\t');
    if (i == std::string::npos) {
      std::cerr << "Missing TAB character" << std::endl;
      return 1;
    }

    std::string name = line.substr(0, i);
    std::string value = line.substr(i + 1, line.size());

    headers.emplace_back(std::pair<std::string, std::string>{ name, value });
  }

  return 0;
}

#pragma GCC diagnostic pop
