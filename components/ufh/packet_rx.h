#pragma once
// Parse a clear-text ESPHome packet_transport UDP datagram (2026.10.1 / 2026.11.0).
// No encryption, no rolling code, no ping-pong — the format Altherma 1ee8199 sends.
// This header has no ESPHome dependency so it can be compiled on the host for tests.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace esphome {
namespace ufh {
namespace packet_rx {

static constexpr uint16_t MAGIC_NUMBER = 0x4553;
static constexpr uint16_t MAGIC_PING = 0x5048;
enum DataKey : uint8_t {
  ZERO_FILL_KEY = 0,
  DATA_KEY = 1,
  SENSOR_KEY = 2,
  BINARY_SENSOR_KEY = 3,
  PING_KEY = 4,
  ROLLING_CODE_KEY = 5,
};

static constexpr size_t MAX_NAME = 32;
static constexpr size_t MAX_ID = 32;

struct SensorVal {
  char id[MAX_ID];
  float value;
};
struct BinaryVal {
  char id[MAX_ID];
  uint8_t value;
};

struct Packet {
  char hostname[MAX_NAME];
  bool is_ping;
  uint8_t n_sensors;
  uint8_t n_binary;
  SensorVal sensors[16];
  BinaryVal binary[8];
};

inline bool looks_like_altherma(const Packet &p) {
  for (uint8_t i = 0; i < p.n_sensors; i++) {
    if (std::strncmp(p.sensors[i].id, "altherma_", 9) == 0)
      return true;
  }
  for (uint8_t i = 0; i < p.n_binary; i++) {
    if (std::strncmp(p.binary[i].id, "altherma_", 9) == 0)
      return true;
  }
  return false;
}

inline bool looks_like_prefix(const Packet &p, const char *prefix) {
  size_t n = std::strlen(prefix);
  for (uint8_t i = 0; i < p.n_sensors; i++) {
    if (std::strncmp(p.sensors[i].id, prefix, n) == 0)
      return true;
  }
  for (uint8_t i = 0; i < p.n_binary; i++) {
    if (std::strncmp(p.binary[i].id, prefix, n) == 0)
      return true;
  }
  return false;
}

inline bool looks_like_tank(const Packet &p) { return looks_like_prefix(p, "tank_"); }
inline bool looks_like_ufh(const Packet &p) { return looks_like_prefix(p, "ufh_"); }


inline const SensorVal *find_sensor(const Packet &p, const char *id) {
  for (uint8_t i = 0; i < p.n_sensors; i++) {
    if (std::strcmp(p.sensors[i].id, id) == 0)
      return &p.sensors[i];
  }
  return nullptr;
}

inline const BinaryVal *find_binary(const Packet &p, const char *id) {
  for (uint8_t i = 0; i < p.n_binary; i++) {
    if (std::strcmp(p.binary[i].id, id) == 0)
      return &p.binary[i];
  }
  return nullptr;
}

// Decode a raw Altherma error byte to the 2-character service code (e.g. 55 -> "E7").
// 0 returns " 0" (space + 0). buf must have room for 3 bytes.
inline void decode_error_code(uint8_t b, char *buf) {
  static const char HI[] = " ACEHFJLPU987654";
  static const char LO[] = "0123456789AHCJEF";
  buf[0] = HI[b >> 4];
  buf[1] = LO[b & 0x0F];
  buf[2] = 0;
}

inline bool parse(const uint8_t *data, size_t len, Packet &out) {
  std::memset(&out, 0, sizeof(out));
  if (data == nullptr || len < 4)
    return false;
  size_t pos = 0;
  auto get16 = [&](uint16_t &v) -> bool {
    if (pos + 2 > len)
      return false;
    v = (uint16_t) (data[pos] | (data[pos + 1] << 8));
    pos += 2;
    return true;
  };
  auto get8 = [&](uint8_t &v) -> bool {
    if (pos >= len)
      return false;
    v = data[pos++];
    return true;
  };
  auto get32 = [&](uint32_t &v) -> bool {
    if (pos + 4 > len)
      return false;
    v = (uint32_t) data[pos] | ((uint32_t) data[pos + 1] << 8) | ((uint32_t) data[pos + 2] << 16) |
        ((uint32_t) data[pos + 3] << 24);
    pos += 4;
    return true;
  };
  auto get_str = [&](char *dst, size_t dst_len) -> bool {
    if (pos >= len)
      return false;
    uint8_t n = data[pos++];
    if (n == 0 || pos + n > len || n >= dst_len)
      return false;
    std::memcpy(dst, data + pos, n);
    dst[n] = 0;
    pos += n;
    return true;
  };

  uint16_t magic = 0;
  if (!get16(magic))
    return false;
  if (magic != MAGIC_NUMBER && magic != MAGIC_PING)
    return false;
  if (!get_str(out.hostname, sizeof(out.hostname)))
    return false;
  if (magic == MAGIC_PING) {
    out.is_ping = true;
    return true;
  }
  // pad header to 4 bytes: 2 (magic) + 1 (len) + namelen
  size_t header_len = 2 + 1 + std::strlen(out.hostname);
  size_t padded = (header_len + 3) & ~3u;
  if (padded > len)
    return false;
  pos = padded;

  if (pos >= len)
    return false;
  uint8_t key = 0;
  if (!get8(key))
    return false;
  // Clear-text Altherma packets start with DATA_KEY. Rolling-code / other keys are ignored.
  if (key != DATA_KEY)
    return false;

  while (pos < len) {
    if (data[pos] == ZERO_FILL_KEY) {
      pos++;
      continue;
    }
    if (data[pos] == PING_KEY) {
      pos++;
      uint32_t dummy = 0;
      if (!get32(dummy))
        return false;
      continue;
    }
    if (data[pos] == BINARY_SENSOR_KEY) {
      pos++;
      if (out.n_binary >= 8)
        return false;
      uint8_t v = 0;
      if (!get8(v))
        return false;
      BinaryVal &b = out.binary[out.n_binary];
      b.value = v;
      if (!get_str(b.id, sizeof(b.id)))
        return false;
      out.n_binary++;
      continue;
    }
    if (data[pos] == SENSOR_KEY) {
      pos++;
      if (out.n_sensors >= 16)
        return false;
      uint32_t bits = 0;
      if (!get32(bits))
        return false;
      union {
        uint32_t u;
        float f;
      } u;
      u.u = bits;
      SensorVal &s = out.sensors[out.n_sensors];
      s.value = u.f;
      if (!get_str(s.id, sizeof(s.id)))
        return false;
      out.n_sensors++;
      continue;
    }
    // unknown key or padding: stop
    break;
  }
  return out.n_sensors > 0 || out.n_binary > 0;
}

}  // namespace packet_rx
}  // namespace ufh
}  // namespace esphome
