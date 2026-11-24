// Host-side test for components/ufh/packet_rx.h (no ESPHome).
#include "packet_rx.h"
#include "packet_tx.h"
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>
#include <cmath>

using namespace esphome::ufh::packet_rx;

static void add16(std::vector<uint8_t> &v, uint16_t x) {
  v.push_back((uint8_t) x);
  v.push_back((uint8_t) (x >> 8));
}
static void add32(std::vector<uint8_t> &v, uint32_t x) {
  v.push_back((uint8_t) x);
  v.push_back((uint8_t) (x >> 8));
  v.push_back((uint8_t) (x >> 16));
  v.push_back((uint8_t) (x >> 24));
}
static void add_str(std::vector<uint8_t> &v, const char *s) {
  auto n = std::strlen(s);
  v.push_back((uint8_t) n);
  v.insert(v.end(), s, s + n);
}
static std::vector<uint8_t> make_packet(const char *host, float lwt, float ret, float outdoor, float freq,
                                        float out_err, float in_err, bool fault) {
  std::vector<uint8_t> v;
  add16(v, MAGIC_NUMBER);
  add_str(v, host);
  while (v.size() & 3)
    v.push_back(0);
  v.push_back(DATA_KEY);
  auto add_sensor = [&](const char *id, float f) {
    v.push_back(SENSOR_KEY);
    union {
      uint32_t u;
      float f;
    } u;
    u.f = f;
    add32(v, u.u);
    add_str(v, id);
  };
  add_sensor("altherma_lwt", lwt);
  add_sensor("altherma_return", ret);
  add_sensor("altherma_outdoor_temp", outdoor);
  add_sensor("altherma_comp_freq", freq);
  add_sensor("altherma_outdoor_error", out_err);
  add_sensor("altherma_indoor_error", in_err);
  v.push_back(BINARY_SENSOR_KEY);
  v.push_back(fault ? 1 : 0);
  add_str(v, "altherma_fault");
  while (v.size() & 3)
    v.push_back(0);
  return v;
}

static int fails = 0;
#define CHECK(cond, msg) \
  do { \
    if (!(cond)) { \
      std::fprintf(stderr, "FAIL: %s\n", msg); \
      fails++; \
    } \
  } while (0)

int main() {
  auto pkt = make_packet("esphome-altherma-83545c", 35.5f, 30.0f, 8.5f, 0.0f, 55.0f, 0.0f, true);
  Packet p{};
  CHECK(parse(pkt.data(), pkt.size(), p), "parse sample");
  CHECK(std::strcmp(p.hostname, "esphome-altherma-83545c") == 0, "hostname");
  CHECK(!p.is_ping, "not ping");
  CHECK(looks_like_altherma(p), "altherma ids");
  const SensorVal *s = find_sensor(p, "altherma_lwt");
  CHECK(s && std::fabs(s->value - 35.5f) < 0.01f, "lwt");
  s = find_sensor(p, "altherma_outdoor_error");
  CHECK(s && std::fabs(s->value - 55.0f) < 0.01f, "outdoor error byte");
  const BinaryVal *b = find_binary(p, "altherma_fault");
  CHECK(b && b->value == 1, "fault on");
  char code[3];
  decode_error_code(55, code);
  CHECK(std::strcmp(code, "E7") == 0, "55 -> E7");
  decode_error_code(147, code);
  CHECK(std::strcmp(code, "U3") == 0, "147 -> U3");
  decode_error_code(0, code);
  CHECK(std::strcmp(code, " 0") == 0 || std::strcmp(code, "00") == 0 || code[0] == ' ', "0 decode");
  // 0 = " ACEHFJLPU987654"[0] + "0123456789AHCJEF"[0] = ' ' and '0' -> " 0"
  CHECK(std::strcmp(code, " 0") == 0, "0 -> ' 0'");

  // bad magic
  pkt[0] = 0;
  CHECK(!parse(pkt.data(), pkt.size(), p), "reject bad magic");

  // empty
  CHECK(!parse(nullptr, 0, p), "reject empty");

  // build() round-trip: UFH demand + open zones
  {
    TxSensor sensors[1] = {{"ufh_open_zones", 3.0f}};
    TxBinary binary[1] = {{"ufh_heat_demand", 1}};
    uint8_t buf[256];
    size_t n = 0;
    CHECK(build(buf, sizeof(buf), n, "esphome-ufh-aabbcc", sensors, 1, binary, 1), "build ufh");
    Packet q{};
    CHECK(parse(buf, n, q), "parse built ufh");
    CHECK(looks_like_ufh(q) && !looks_like_altherma(q) && !looks_like_tank(q), "ufh ids");
    const SensorVal *zs = find_sensor(q, "ufh_open_zones");
    CHECK(zs && std::fabs(zs->value - 3.0f) < 0.01f, "open zones");
    const BinaryVal *d = find_binary(q, "ufh_heat_demand");
    CHECK(d && d->value == 1, "heat demand");
  }
  {
    TxSensor sensors[1] = {{"tank_temp", 38.5f}};
    TxBinary binary[2] = {{"tank_pump", 1}, {"tank_heater", 0}};
    uint8_t buf[256];
    size_t n = 0;
    CHECK(build(buf, sizeof(buf), n, "esphome-ufh-tank-ccbbaa", sensors, 1, binary, 2), "build tank");
    Packet q{};
    CHECK(parse(buf, n, q), "parse built tank");
    CHECK(looks_like_tank(q) && !looks_like_ufh(q), "tank ids");
    const SensorVal *tt = find_sensor(q, "tank_temp");
    CHECK(tt && std::fabs(tt->value - 38.5f) < 0.01f, "tank temp");
  }
  CHECK(std::fabs(speed_to_duty(100.0f, true) - 10.0f) < 0.01f, "profile A max speed -> 10% duty");
  CHECK(std::fabs(speed_to_duty(0.0f, true) - 91.0f) < 0.01f, "profile A min speed -> 91% duty");
  CHECK(std::fabs(speed_to_duty(50.0f, false) - 50.0f) < 0.01f, "direct duty equals speed");

  if (fails) {
    std::fprintf(stderr, "%d check(s) failed\n", fails);
    return 1;
  }
  std::printf("packet_rx: all checks passed\n");
  return 0;
}
