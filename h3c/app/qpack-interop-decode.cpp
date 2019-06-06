#include <fstream>
#include <vector>

#include <h3c/log/fprintf.hpp>
#include <h3c/qpack.hpp>

#include <util.hpp>

static h3c::logger logger{ h3c::log::impl::fprintf() }; // NOLINT

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

uint64_t id_decode(h3c::buffer &src, std::error_code &ec)
{
  if (src.size() < sizeof(uint64_t)) {
    THROW(h3c::error::incomplete);
  }

  uint64_t id = static_cast<uint64_t>(src[0]) << 56U |
                static_cast<uint64_t>(src[1]) << 48U |
                static_cast<uint64_t>(src[2]) << 40U |
                static_cast<uint64_t>(src[3]) << 32U |
                static_cast<uint64_t>(src[4]) << 24U |
                static_cast<uint64_t>(src[5]) << 16U |
                static_cast<uint64_t>(src[6]) << 8U |
                static_cast<uint64_t>(src[7]) << 0U;

  src.advance(sizeof(uint64_t));

  return id;
}

size_t size_decode(h3c::buffer &src, std::error_code &ec)
{
  if (src.size() < sizeof(uint32_t)) {
    THROW(h3c::error::incomplete);
  }

  size_t encoded_size = static_cast<uint32_t>(src[0]) << 24U |
                        static_cast<uint32_t>(src[1]) << 16U |
                        static_cast<uint32_t>(src[2]) << 8U |
                        static_cast<uint32_t>(src[3]) << 0U;

  src.advance(sizeof(uint32_t));

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

  h3c::mutable_buffer src;

  try {
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    src = h3c::mutable_buffer(static_cast<size_t>(size));
    input.read(reinterpret_cast<char *>(src.data()), size);
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

  while (!src.empty()) {
    TRY(id_decode(src, ec));
    size_t encoded_size = TRY(size_decode(src, ec));

    size_t begin = src.position();

    qpack.prefix_decode(src, ec);
    if (ec) {
      return ec.value();
    }

    std::vector<h3c::header> headers;

    while (src.position() - begin != encoded_size) {
      h3c::header header = TRY(qpack.decode(src, ec));
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
