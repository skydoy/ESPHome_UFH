#pragma once
#include <cstdint>
#include <cstring>

namespace esphome {
namespace ufh_tank {

static constexpr uint32_t TANK_MAGIC = 0x55465432;  // "UFT2"

struct TankCfg {
  uint32_t magic;
  uint8_t configured;
  uint8_t relay_active_low;
  uint8_t pwm_invert;  // 1 = Grundfos PWM profile A (default)
  uint8_t reserved0;
  int16_t tank_target_x10;  // default 40.0
  int16_t tank_cut_x10;     // default 45.0
  int16_t tank_hyst_x10;    // default 3.0
  uint16_t comp_zero_min;   // minutes, default 15
  uint8_t pwm_min_speed;    // 0-100, default 30
  uint8_t pwm_max_speed;    // 0-100, default 100
  uint8_t pwm_zones_max;    // open zones that map to max speed, default 8
  uint8_t reserved1;
  char ufh_provider[32];
  char altherma_provider[32];
  char tank_probe[22];
  uint8_t reserved[42];
};
static_assert(sizeof(TankCfg) == 148, "TankCfg size");

inline void default_tank(TankCfg &c) {
  memset(&c, 0, sizeof(c));
  c.magic = TANK_MAGIC;
  c.pwm_invert = 1;
  c.tank_target_x10 = 400;
  c.tank_cut_x10 = 450;
  c.tank_hyst_x10 = 30;
  c.comp_zero_min = 15;
  c.pwm_min_speed = 30;
  c.pwm_max_speed = 100;
  c.pwm_zones_max = 8;
}

}  // namespace ufh_tank
}  // namespace esphome
