#include <bnl/http3/error.hpp>

#include <bnl/http3/codec/qpack.hpp>

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

static buffer id_encode(uint64_t id)
{
  buffer encoded(sizeof(uint64_t));

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

static buffer size_encode(uint32_t encoded_size)
{
  buffer encoded(sizeof(uint32_t));

  encoded[0] = static_cast<uint8_t>(encoded_size >> 24U);
  encoded[1] = static_cast<uint8_t>(encoded_size >> 16U);
  encoded[2] = static_cast<uint8_t>(encoded_size >> 8U);
  encoded[3] = static_cast<uint8_t>(encoded_size >> 0U);

  return encoded;
}

void write(std::ostream &dest, const buffer &encoded)
{
  dest.write(reinterpret_cast<const char *>(encoded.data()),
             static_cast<int32_t>(encoded.size()));
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

  // `std::getline` sets `failbit` when it doesn't read any characters.
  input.exceptions(std::ifstream::badbit);

  std::ofstream output;
  output.exceptions(std::ofstream::failbit | std::ofstream::badbit);

  // Open output file

  try {
    output.open(argv[2], std::ios::trunc | std::ios::binary);
  } catch (const std::ios_base::failure &e) {
    LOG_E("Error opening output file: {}", e.what());
    return 1;
  }

  // Read input

  std::string line;
  uint64_t id = 1;
  std::vector<http3::header> headers;

  http3::qpack::encoder qpack(logger_.get());
  std::error_code ec;

  while (std::getline(input, line)) {
    if (line.empty()) {
      if (headers.empty()) {
        continue;
      }

      // Encode header block and write to output

      try {
        std::queue<buffer> buffers;

        for (const http3::header &header : headers) {
          buffer encoded = TRY(qpack.encode(header, ec));
          buffers.emplace(std::move(encoded));
        }

        if (qpack.count() > UINT32_MAX) {
          LOG_E("Headers encoded size does not fit in an unsigned 32-bit "
                "integer");
        }

        write(output, id_encode(id));
        write(output, size_encode(static_cast<uint32_t>(qpack.count())));

        while (!buffers.empty()) {
          buffer encoded = std::move(buffers.front());
          buffers.pop();
          write(output, encoded);
        }
      } catch (const std::ios_base::failure &e) {
        LOG_E("Error writing to output file: {}", e.what());
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
      LOG_E("Missing TAB character");
      return 1;
    }

    buffer name(i);
    std::copy_n(line.substr(0, i).data(), name.size(), name.data());

    buffer value(line.size() - (i + 1));
    std::copy_n(line.substr(i + 1).data(), value.size(), value.data());

    headers.emplace_back(http3::header{ std::move(name), std::move(value) });
  }

  return 0;
}

#pragma GCC diagnostic pop
