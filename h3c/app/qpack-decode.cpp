#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <h3c/log/fprintf.hpp>
#include <h3c/qpack.hpp>

#include <util.hpp>

std::error_code
decode(uint8_t *src,
       size_t size,
       uint64_t *stream_id,
       std::vector<std::pair<std::string, std::string>> *headers,
       size_t *encoded_size,
       const h3c::logger *logger)
{
  *encoded_size = 0;

  if (sizeof(uint64_t) > size) {
    THROW(h3c::error::incomplete);
  }

  *stream_id = static_cast<uint64_t>(src[0]) << 56U |
               static_cast<uint64_t>(src[1]) << 48U |
               static_cast<uint64_t>(src[2]) << 40U |
               static_cast<uint64_t>(src[3]) << 32U |
               static_cast<uint64_t>(src[4]) << 24U |
               static_cast<uint64_t>(src[5]) << 16U |
               static_cast<uint64_t>(src[6]) << 8U |
               static_cast<uint64_t>(src[7]) << 0U;
  src += sizeof(uint64_t);
  size -= sizeof(uint64_t);
  *encoded_size += sizeof(uint64_t);

  if (sizeof(uint32_t) > size) {
    THROW(h3c::error::incomplete);
  }

  size_t header_block_encoded_size = static_cast<uint32_t>(src[0]) << 24U |
                                     static_cast<uint32_t>(src[1]) << 16U |
                                     static_cast<uint32_t>(src[2]) << 8U |
                                     static_cast<uint32_t>(src[3]) << 0U;
  src += sizeof(uint32_t);
  size -= sizeof(uint32_t);
  *encoded_size += sizeof(uint32_t);

  if (header_block_encoded_size > size) {
    THROW(h3c::error::incomplete);
  }

  size_t prefix_encoded_size = 0;
  std::error_code error = h3c::qpack::prefix::decode(src, size,
                                                     &prefix_encoded_size,
                                                     logger);
  if (error) {
    return error;
  }

  src += prefix_encoded_size;
  size -= prefix_encoded_size;
  header_block_encoded_size -= prefix_encoded_size;
  *encoded_size += prefix_encoded_size;

  h3c::qpack::decoder qpack;
  error = qpack.init(logger);
  if (error) {
    return error;
  }

  while (header_block_encoded_size > 0) {
    size_t header_encoded_size = 0;
    h3c::header header = {};

    error = qpack.decode(src, size, &header, &header_encoded_size, logger);
    if (error) {
      return error;
    }

    std::string name(header.name.data, header.name.size);
    std::string value(header.value.data, header.value.size);

    headers->emplace_back(std::pair<std::string, std::string>{ name, value });

    src += header_encoded_size;
    size -= header_encoded_size;
    header_block_encoded_size -= header_encoded_size;
    *encoded_size += header_encoded_size;
  }

  return {};
}

void write(std::ostream &dest,
           const std::vector<std::pair<std::string, std::string>> &headers)
{
  for (const auto &entry : headers) {
    dest << entry.first << '\t' << entry.second << '\n';
  }

  dest << '\n';
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

int main(int argc, char *argv[])
{
  if (argc < 3) {
    return 1;
  }

  h3c::logger logger{ h3c::log::impl::fprintf() };

  std::vector<uint8_t> encoded;

  std::ifstream input;
  input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // Open input file

  try {
    input.open(argv[1], std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    H3C_LOG_ERROR(&logger, "Error opening input file: {}", e.what());
    return 1;
  }

  // Read input

  try {
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    encoded = std::vector<uint8_t>(static_cast<size_t>(size));
    input.read(reinterpret_cast<char *>(encoded.data()), size); // NOLINT
  } catch (const std::ios_base::failure &e) {
    H3C_LOG_ERROR(&logger, "Error reading input: {}", e.what());
    return 1;
  }

  std::ofstream output;
  output.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // Open output file

  try {
    output.open(argv[2], std::ios::trunc | std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    H3C_LOG_ERROR(&logger, "Error opening output file: {}", e.what());
    return 1;
  }

  // Decode input

  while (!encoded.empty()) {
    uint64_t stream_id = 0;
    std::vector<std::pair<std::string, std::string>> headers;
    size_t encoded_size = 0;

    std::error_code error = decode(encoded.data(), encoded.size(), &stream_id,
                                   &headers, &encoded_size, &logger);
    if (error) {
      return error.value();
    }

    // Write output

    try {
      write(output, headers);
    } catch (std::ios_base::failure &e) {
      H3C_LOG_ERROR(&logger, "Error writing output: {}", e.what());
      return 1;
    }

    encoded.erase(encoded.begin(),
                  encoded.begin() + static_cast<int32_t>(encoded_size));
  }

  return 0;
}

#pragma GCC diagnostic pop
