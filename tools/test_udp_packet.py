#!/usr/bin/env python3
"""Host-side check of the packet_transport layout used by packet_rx.h."""
from __future__ import annotations

import struct
import sys

MAGIC = 0x4553
DATA_KEY, SENSOR_KEY, BINARY_SENSOR_KEY = 1, 2, 3
HI = " ACEHFJLPU987654"
LO = "0123456789AHCJEF"


def decode_error_code(b: int) -> str:
    return HI[(b >> 4) & 0x0F] + LO[b & 0x0F]


def add_str(buf: bytearray, s: str) -> None:
    raw = s.encode("ascii")
    buf.append(len(raw))
    buf.extend(raw)


def make_packet(host: str, lwt, ret, outdoor, freq, out_err, in_err, fault: bool) -> bytes:
    v = bytearray()
    v += struct.pack("<H", MAGIC)
    add_str(v, host)
    while len(v) & 3:
        v.append(0)
    v.append(DATA_KEY)

    def add_sensor(buf: bytearray, ident: str, f: float) -> None:
        buf.append(SENSOR_KEY)
        buf.extend(struct.pack("<f", f))
        add_str(buf, ident)

    add_sensor(v, "altherma_lwt", lwt)
    add_sensor(v, "altherma_return", ret)
    add_sensor(v, "altherma_outdoor_temp", outdoor)
    add_sensor(v, "altherma_comp_freq", freq)
    add_sensor(v, "altherma_outdoor_error", out_err)
    add_sensor(v, "altherma_indoor_error", in_err)
    v.append(BINARY_SENSOR_KEY)
    v.append(1 if fault else 0)
    add_str(v, "altherma_fault")
    while len(v) & 3:
        v.append(0)
    return bytes(v)


def parse(data: bytes):
    if not data or len(data) < 3:
        return None
    magic = struct.unpack_from("<H", data, 0)[0]
    if magic != MAGIC:
        return None
    i = 2
    n = data[i]
    i += 1
    host = data[i : i + n].decode("ascii", "replace")
    i += n
    while i & 3:
        i += 1
    if i >= len(data) or data[i] != DATA_KEY:
        return None
    i += 1
    sensors, binary = {}, {}
    while i < len(data):
        key = data[i]
        i += 1
        if key == 0:  # ZERO_FILL
            continue
        if key == SENSOR_KEY:
            if i + 4 >= len(data):
                return None
            val = struct.unpack_from("<f", data, i)[0]
            i += 4
            ln = data[i]
            i += 1
            ident = data[i : i + ln].decode("ascii", "replace")
            i += ln
            sensors[ident] = val
        elif key == BINARY_SENSOR_KEY:
            if i >= len(data):
                return None
            val = data[i]
            i += 1
            ln = data[i]
            i += 1
            ident = data[i : i + ln].decode("ascii", "replace")
            i += ln
            binary[ident] = val
        else:
            break
    return {"host": host, "sensors": sensors, "binary": binary}


def main() -> int:
    fails = 0

    def check(cond: bool, msg: str) -> None:
        nonlocal fails
        if not cond:
            print(f"FAIL: {msg}", file=sys.stderr)
            fails += 1

    pkt = make_packet("esphome-altherma-83545c", 35.5, 30.0, 8.5, 0.0, 55.0, 0.0, True)
    p = parse(pkt)
    check(p is not None, "parse sample")
    check(p and p["host"] == "esphome-altherma-83545c", "hostname")
    check(p and any(k.startswith("altherma_") for k in p["sensors"]), "altherma ids")
    check(p and abs(p["sensors"]["altherma_lwt"] - 35.5) < 0.01, "lwt")
    check(p and abs(p["sensors"]["altherma_outdoor_error"] - 55.0) < 0.01, "outdoor error byte")
    check(p and p["binary"].get("altherma_fault") == 1, "fault on")
    check(decode_error_code(55) == "E7", "55 -> E7")
    check(decode_error_code(147) == "U3", "147 -> U3")
    check(decode_error_code(0) == " 0", "0 -> ' 0'")
    bad = bytearray(pkt)
    bad[0] = 0
    check(parse(bytes(bad)) is None, "reject bad magic")
    check(parse(b"") is None, "reject empty")
    if fails:
        print(f"{fails} check(s) failed", file=sys.stderr)
        return 1
    print("packet_rx (python): all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
