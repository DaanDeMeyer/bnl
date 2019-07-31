#include <bnl/http3/codec/qpack.hpp>
#include <bnl/log/console.hpp>

#include <fstream>
#include <vector>

using namespace bnl;

static http3::result<uint64_t>
id_decode(base::buffer &encoded)
{
  if (encoded.size() < sizeof(uint64_t)) {
    return http3::error::incomplete;
  };

  uint64_t id = static_cast<uint64_t>(encoded[0]) << 56U |
                static_cast<uint64_t>(encoded[1]) << 48U |
                static_cast<uint64_t>(encoded[2]) << 40U |
                static_cast<uint64_t>(encoded[3]) << 32U |
                static_cast<uint64_t>(encoded[4]) << 24U |
                static_cast<uint64_t>(encoded[5]) << 16U |
                static_cast<uint64_t>(encoded[6]) << 8U |
                static_cast<uint64_t>(encoded[7]) << 0U;

  encoded.consume(sizeof(uint64_t));

  return id;
}

static http3::result<size_t>
size_decode(base::buffer &encoded)
{
  if (encoded.size() < sizeof(uint32_t)) {
    return http3::error::incomplete;
  }

  size_t encoded_size = static_cast<uint32_t>(encoded[0]) << 24U |
                        static_cast<uint32_t>(encoded[1]) << 16U |
                        static_cast<uint32_t>(encoded[2]) << 8U |
                        static_cast<uint32_t>(encoded[3]) << 0U;

  encoded.consume(sizeof(uint32_t));

  return encoded_size;
}

static void
write(std::ostream &dest, const std::vector<http3::header> &headers)
{
  for (const http3::header &header : headers) {
    dest << header.name() << '\t' << header.value() << '\n';
  }

  dest << '\n';
}

static http3::result<void>
decode(base::buffer &encoded, std::ofstream &output)
{
  BNL_TRY(id_decode(encoded));
  size_t encoded_size = BNL_TRY(size_decode(encoded));

  http3::qpack::decoder qpack;
  std::vector<http3::header> headers;

  while (qpack.count() != encoded_size) {
    http3::header header = BNL_TRY(qpack.decode(encoded));
    headers.emplace_back(std::move(header));
  }

  write(output, headers);

  return base::success();
}

int
main(int argc, char *argv[])
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
    BNL_LOG_E("Error opening input file: {}", e.what());
    return 1;
  }

  // Read input

  base::buffer encoded;

  try {
    input.seekg(0, std::ios::end);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    encoded = base::buffer(static_cast<size_t>(size));
    input.read(reinterpret_cast<char *>(encoded.data()), size);
  } catch (const std::ios_base::failure &e) {
    BNL_LOG_E("Error reading input: {}", e.what());
    return 1;
  }

  std::ofstream output;
  output.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // Open output file

  try {
    output.open(argv[2], std::ios::trunc | std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    BNL_LOG_E("Error opening output file: {}", e.what());
    return 1;
  }

  // Decode input

  while (!encoded.empty()) {
    http3::result<void> r = decode(encoded, output);
    if (!r) {
      BNL_LOG_E("Error decoding headers");
      return 1;
    }
  }

  return 0;
}
