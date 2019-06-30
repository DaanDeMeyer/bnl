#include <bnl/http3/error.hpp>

#include <bnl/http3/codec/qpack.hpp>

#include <bnl/log/console.hpp>

#include <bnl/util/error.hpp>

#include <fstream>
#include <vector>

using namespace bnl;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

uint64_t
id_decode(buffer &encoded, std::error_code &ec, const log::api *logger_)
{
  CHECK(encoded.size() >= sizeof(uint64_t), http3::error::incomplete);

  uint64_t id = static_cast<uint64_t>(encoded[0]) << 56U |
                static_cast<uint64_t>(encoded[1]) << 48U |
                static_cast<uint64_t>(encoded[2]) << 40U |
                static_cast<uint64_t>(encoded[3]) << 32U |
                static_cast<uint64_t>(encoded[4]) << 24U |
                static_cast<uint64_t>(encoded[5]) << 16U |
                static_cast<uint64_t>(encoded[6]) << 8U |
                static_cast<uint64_t>(encoded[7]) << 0U;

  encoded += sizeof(uint64_t);

  return id;
}

size_t
size_decode(buffer &encoded, std::error_code &ec, const log::api *logger_)
{
  CHECK(encoded.size() >= sizeof(uint32_t), http3::error::incomplete);

  size_t encoded_size = static_cast<uint32_t>(encoded[0]) << 24U |
                        static_cast<uint32_t>(encoded[1]) << 16U |
                        static_cast<uint32_t>(encoded[2]) << 8U |
                        static_cast<uint32_t>(encoded[3]) << 0U;

  encoded += sizeof(uint32_t);

  return encoded_size;
}

void write(std::ostream &dest, const std::vector<http3::header> &headers)
{
  for (const http3::header &header : headers) {
    std::string name(reinterpret_cast<const char *>(header.name.data()),
                     header.name.size());
    std::string value(reinterpret_cast<const char *>(header.value.data()),
                      header.value.size());
    dest << name << '\t' << value << '\n';
  }

  dest << '\n';
}

int main(int argc, char *argv[])
{
  if (argc < 3) {
    return 1;
  }

  // Allocate logger on the heap because `THROW` and `LOG_E` macro expect a
  // pointer.
  std::unique_ptr<log::api> logger_(new log::impl::console());

  std::ifstream input;
  input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // Open input file

  try {
    input.open(argv[1], std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    LOG_E("Error opening input file: {}", e.what());
    return 1;
  }

  // Read input

  buffer encoded;

  try {
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    buffer_mut buffer(static_cast<size_t>(size));
    input.read(reinterpret_cast<char *>(buffer.data()), size);

    encoded = std::move(buffer);
  } catch (const std::ios_base::failure &e) {
    LOG_E("Error reading input: {}", e.what());
    return 1;
  }

  std::ofstream output;
  output.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // Open output file

  try {
    output.open(argv[2], std::ios::trunc | std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    LOG_E("Error opening output file: {}", e.what());
    return 1;
  }

  // Decode input

  std::error_code ec;
  http3::qpack::decoder qpack(logger_.get());

  while (!encoded.empty()) {
    TRY(id_decode(encoded, ec, logger_.get()));
    size_t encoded_size = TRY(size_decode(encoded, ec, logger_.get()));

    std::vector<http3::header> headers;

    while (qpack.count() != encoded_size) {
      http3::header header = TRY(qpack.decode(encoded, ec));
      headers.emplace_back(std::move(header));
    }

    // Write output

    try {
      write(output, headers);
    } catch (std::ios_base::failure &e) {
      LOG_E("Error writing output: {}", e.what());
      return 1;
    }
  }

  return 0;
}

#pragma GCC diagnostic pop
