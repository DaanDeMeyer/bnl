#include <fstream>
#include <vector>

#include <h3c/log/fprintf.hpp>
#include <h3c/qpack.hpp>

#include <util.hpp>

static h3c::logger logger{ h3c::log::impl::fprintf() }; // NOLINT

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

uint64_t id_decode(h3c::buffer &encoded, std::error_code &ec)
{
  if (encoded.size() < sizeof(uint64_t)) {
    THROW(h3c::error::incomplete);
  }

  uint64_t id = static_cast<uint64_t>(encoded[0]) << 56U |
                static_cast<uint64_t>(encoded[1]) << 48U |
                static_cast<uint64_t>(encoded[2]) << 40U |
                static_cast<uint64_t>(encoded[3]) << 32U |
                static_cast<uint64_t>(encoded[4]) << 24U |
                static_cast<uint64_t>(encoded[5]) << 16U |
                static_cast<uint64_t>(encoded[6]) << 8U |
                static_cast<uint64_t>(encoded[7]) << 0U;

  encoded.advance(sizeof(uint64_t));

  return id;
}

size_t size_decode(h3c::buffer &encoded, std::error_code &ec)
{
  if (encoded.size() < sizeof(uint32_t)) {
    THROW(h3c::error::incomplete);
  }

  size_t encoded_size = static_cast<uint32_t>(encoded[0]) << 24U |
                        static_cast<uint32_t>(encoded[1]) << 16U |
                        static_cast<uint32_t>(encoded[2]) << 8U |
                        static_cast<uint32_t>(encoded[3]) << 0U;

  encoded.advance(sizeof(uint32_t));

  return encoded_size;
}

void write(std::ostream &dest, const std::vector<h3c::header> &headers)
{
  for (const auto &header : headers) {
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

  h3c::mutable_buffer encoded;

  try {
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    encoded = h3c::mutable_buffer(static_cast<size_t>(size));
    input.read(reinterpret_cast<char *>(encoded.data()), size);
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

  std::error_code ec;
  h3c::qpack::decoder qpack(&logger);

  while (!encoded.empty()) {
    TRY(id_decode(encoded, ec));
    size_t encoded_size = TRY(size_decode(encoded, ec));

    std::vector<h3c::header> headers;

    while (qpack.count() != encoded_size) {
      h3c::header header = TRY(qpack.decode(encoded, ec));
      headers.emplace_back(std::move(header));
    }

    // Write output

    try {
      write(output, headers);
    } catch (std::ios_base::failure &e) {
      H3C_LOG_ERROR(&logger, "Error writing output: {}", e.what());
      return 1;
    }
  }

  return 0;
}

#pragma GCC diagnostic pop
