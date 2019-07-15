#include <bnl/http3/codec/qpack.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/log/console.hpp>
#include <bnl/util/error.hpp>

#include <algorithm>
#include <fstream>
#include <memory>
#include <queue>
#include <vector>

using namespace bnl;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

// Allocate logger on the heap because `THROW` and `LOG_E` macro expect a
// pointer.
static std::unique_ptr<log::api> logger_(new log::console()); // NOLINT

static base::buffer
id_encode(uint64_t id)
{
  base::buffer encoded(sizeof(uint64_t));

  encoded[0] = static_cast<uint8_t>(id >> 56U);
  encoded[1] = static_cast<uint8_t>(id >> 48U);
  encoded[2] = static_cast<uint8_t>(id >> 40U);
  encoded[3] = static_cast<uint8_t>(id >> 32U);
  encoded[4] = static_cast<uint8_t>(id >> 24U);
  encoded[5] = static_cast<uint8_t>(id >> 16U);
  encoded[6] = static_cast<uint8_t>(id >> 8U);
  encoded[7] = static_cast<uint8_t>(id >> 0U);

  return encoded;
}

static base::buffer
size_encode(uint32_t encoded_size)
{
  base::buffer encoded(sizeof(uint32_t));

  encoded[0] = static_cast<uint8_t>(encoded_size >> 24U);
  encoded[1] = static_cast<uint8_t>(encoded_size >> 16U);
  encoded[2] = static_cast<uint8_t>(encoded_size >> 8U);
  encoded[3] = static_cast<uint8_t>(encoded_size >> 0U);

  return encoded;
}

void
write(std::ostream& dest, const base::buffer& encoded)
{
  dest.write(reinterpret_cast<const char*>(encoded.data()),
             static_cast<int32_t>(encoded.size()));
}

static std::error_code
encode(uint64_t id,
       const std::vector<http3::header>& headers,
       std::ofstream& output)
{
  http3::qpack::encoder qpack(logger_.get());

  std::queue<base::buffer> buffers;

  for (const http3::header& header : headers) {
    base::result<base::buffer> result = TRY(qpack.encode(header));
    buffers.emplace(std::move(result.value()));
  }

  if (qpack.count() > UINT32_MAX) {
    LOG_E("Headers encoded size does not fit in an unsigned 32-bit integer");
  }

  try {
    write(output, id_encode(id));
    write(output, size_encode(static_cast<uint32_t>(qpack.count())));

    while (!buffers.empty()) {
      base::buffer encoded = std::move(buffers.front());
      buffers.pop();
      write(output, encoded);
    }
  } catch (const std::ios_base::failure& e) {
    LOG_E("Error writing to output file: {}", e.what());
    return e.code();
  }

  return {};
}

int
main(int argc, char* argv[])
{
  if (argc < 3) {
    return 1;
  }

  std::ifstream input;
  input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // Open input file

  try {
    input.open(argv[1], std::ios::binary);
  } catch (const std::ios_base::failure& e) {
    LOG_E("Error opening input file: {}", e.what());
    return 1;
  }

  // `std::getline` sets `failbit` when it doesn't read any characters.
  input.exceptions(std::ifstream::badbit);

  std::ofstream output;
  output.exceptions(std::ofstream::failbit | std::ofstream::badbit);

  // Open output file

  try {
    output.open(argv[2], std::ios::trunc | std::ios::binary);
  } catch (const std::ios_base::failure& e) {
    LOG_E("Error opening output file: {}", e.what());
    return 1;
  }

  // Read input

  base::string line;
  uint64_t id = 1;
  std::vector<http3::header> headers;

  std::error_code ec;

  while (std::getline(input, line)) {
    if (line.empty()) {
      if (headers.empty()) {
        continue;
      }

      std::error_code ec = encode(id, headers, output);
      if (ec) {
        LOG_E("Error encoding headers");
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
    if (i == base::string::npos) {
      LOG_E("Missing TAB character");
      return 1;
    }

    http3::header header(line.substr(0, i), line.substr(i + 1));
    headers.emplace_back(std::move(header));
  }

  return 0;
}

#pragma GCC diagnostic pop
