#pragma once
// Build a clear-text ESPHome packet_transport UDP datagram (no encryption).
// Pair of packet_rx.h; no ESPHome dependency, so it can be compiled on the host for tests.
#include "packet_rx.h"
#include <cstring>

namespace esphome {
namespace ufh {
namespace packet_rx {

struct TxSensor {
  const char *id;
  float value;
};
struct TxBinary {
  const char *id;
  uint8_t value;
};

// Write a packet into out[0..out_len). Returns false if it would not fit or the name is empty.
inline bool build(uint8_t *out, size_t out_len, size_t &written, const char *hostname,
                  const TxSensor *sensors, uint8_t n_sensors, const TxBinary *binary, uint8_t n_binary) {
  written = 0;
  if (out == nullptr || hostname == nullptr || hostname[0] == 0)
    return false;
  size_t name_len = std::strlen(hostname);
  if (name_len == 0 || name_len >= MAX_NAME)
    return false;
  auto put8 = [&](uint8_t v) -> bool {
    if (written >= out_len)
      return false;
    out[written++] = v;
    return true;
  };
  auto put16 = [&](uint16_t v) -> bool { return put8((uint8_t) (v & 0xFF)) && put8((uint8_t) (v >> 8)); };
  auto put32 = [&](uint32_t v) -> bool {
    return put8((uint8_t) v) && put8((uint8_t) (v >> 8)) && put8((uint8_t) (v >> 16)) && put8((uint8_t) (v >> 24));
  };
  auto put_str = [&](const char *s) -> bool {
    size_t n = std::strlen(s);
    if (n == 0 || n >= MAX_ID || !put8((uint8_t) n))
      return false;
    if (written + n > out_len)
      return false;
    std::memcpy(out + written, s, n);
    written += n;
    return true;
  };

  if (!put16(MAGIC_NUMBER) || !put8((uint8_t) name_len))
    return false;
  if (written + name_len > out_len)
    return false;
  std::memcpy(out + written, hostname, name_len);
  written += name_len;
  while (written & 3u) {
    if (!put8(0))
      return false;
  }
  if (!put8(DATA_KEY))
    return false;
  for (uint8_t i = 0; i < n_sensors; i++) {
    if (sensors[i].id == nullptr || sensors[i].id[0] == 0)
      return false;
    if (!put8(SENSOR_KEY))
      return false;
    union {
      uint32_t u;
      float f;
    } u;
    u.f = sensors[i].value;
    if (!put32(u.u) || !put_str(sensors[i].id))
      return false;
  }
  for (uint8_t i = 0; i < n_binary; i++) {
    if (binary[i].id == nullptr || binary[i].id[0] == 0)
      return false;
    if (!put8(BINARY_SENSOR_KEY) || !put8(binary[i].value) || !put_str(binary[i].id))
      return false;
  }
  return written > 0 && (n_sensors + n_binary) > 0;
}

// Grundfos PWM profile A (heating): speed 0–100 % maps to duty 91–10 % (higher duty = slower).
// Direct (invert=false): duty equals speed. Values are clamped to 0–100.
inline float speed_to_duty(float speed_pct, bool invert_profile_a) {
  if (speed_pct < 0)
    speed_pct = 0;
  if (speed_pct > 100)
    speed_pct = 100;
  if (!invert_profile_a)
    return speed_pct;
  return 91.0f - speed_pct * (91.0f - 10.0f) / 100.0f;
}

}  // namespace packet_rx
}  // namespace ufh
}  // namespace esphome
