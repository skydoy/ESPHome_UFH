# Generates the wiring diagrams in docs/img/ (wiring-*-pt.svg / -en.svg). Run from the repo root.
PINS = [1, 2, 15, 16, 17, 18, 38, 39, 40, 41, 42, 47, 48]
COLORS = ['#e6194b', '#3cb44b', '#f58231', '#4363d8', '#911eb4', '#46a0a0', '#f032e6', '#9a6324',
          '#800000', '#008080', '#000075', '#808000', '#e6a100', '#1784F7', '#ff7a1a', '#555']
FONT = 'font-family="Segoe UI, Roboto, Helvetica, Arial, sans-serif"'

def T(lang, pt, en):
    return pt if lang == 'pt' else en

def head(w, h, title):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}" {FONT}>'
            f'<rect width="{w}" height="{h}" fill="#ffffff"/>'
            f'<text x="20" y="34" font-size="20" font-weight="700" fill="#0B1B3F">{title}</text>')

def board(x, y, w, h, title, sub, color='#0B1B3F'):
    return (f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="12" fill="#f4f7fb" stroke="{color}" stroke-width="2"/>'
            f'<text x="{x + 14}" y="{y + 26}" font-size="15" font-weight="700" fill="{color}">{title}</text>'
            f'<text x="{x + 14}" y="{y + 44}" font-size="12" fill="#555">{sub}</text>')

def pin(x, y, label, side='r', color='#333'):
    tx = x - 10 if side == 'r' else x + 10
    anchor = 'end' if side == 'r' else 'start'
    return (f'<circle cx="{x}" cy="{y}" r="5" fill="#fff" stroke="{color}" stroke-width="2"/>'
            f'<text x="{tx}" y="{y + 4}" font-size="12" text-anchor="{anchor}" fill="#222">{label}</text>')

def wire(x1, y1, x2, y2, c, w=3):
    mx = (x1 + x2) / 2
    return f'<path d="M{x1} {y1} C{mx} {y1} {mx} {y2} {x2} {y2}" fill="none" stroke="{c}" stroke-width="{w}"/>'

def direct(lang):
    rows = PINS
    W, top, step = 980, 150, 34
    H = top + (len(rows) + 1) * step + 150
    o = [head(W, H, T(lang, 'Relés diretamente nos pinos do ESP (até 13)', 'Relays straight on the ESP pins (up to 13)'))]
    o.append(f'<text x="20" y="58" font-size="13" fill="#555">{T(lang, "Fio a fio, do ESP às entradas da placa de relés (onde antes ligava o DAEnetIP4). Liga também o GND.", "Wire by wire, from the ESP to the relay board inputs (where the DAEnetIP4 used to plug in). Connect GND too.")}</text>')
    bx, bw = 40, 300
    o.append(board(bx, 80, bw, H - 190, 'Waveshare ESP32-S3-ETH', T(lang, 'com PoE Module (B) por cima', 'with PoE Module (B) on top')))
    rx, rw = 640, 300
    o.append(board(rx, 80, rw, H - 190, T(lang, 'Placa de relés', 'Relay board'), T(lang, 'ex.: Denkovi (entradas 3,3 V)', 'e.g. Denkovi (3.3 V inputs)')))
    for i, g in enumerate(rows):
        y = top + i * step
        c = COLORS[i]
        o.append(pin(bx + bw, y, f'GPIO{g}', 'r', c))
        o.append(pin(rx, y, f'IN{i + 1}  →  {T(lang, "relé", "relay")} {i + 1}', 'l', c))
        o.append(wire(bx + bw + 5, y, rx - 5, y, c))
    y = top + len(rows) * step
    o.append(pin(bx + bw, y, 'GND', 'r', '#000'))
    o.append(pin(rx, y, 'GND', 'l', '#000'))
    o.append(wire(bx + bw + 5, y, rx - 5, y, '#000', 4))
    # ethernet / power
    ey = H - 90
    o.append(f'<rect x="{bx}" y="{ey}" width="{bw}" height="56" rx="10" fill="#e8f3ff" stroke="#1784F7" stroke-width="2"/>'
             f'<text x="{bx + 14}" y="{ey + 24}" font-size="13" font-weight="700" fill="#1784F7">RJ45 → {T(lang, "switch PoE", "PoE switch")}</text>'
             f'<text x="{bx + 14}" y="{ey + 43}" font-size="12" fill="#555">{T(lang, "rede + alimentação do ESP num só cabo", "network + ESP power on one cable")}</text>')
    o.append(f'<rect x="{rx}" y="{ey}" width="{rw}" height="56" rx="10" fill="#fff4ec" stroke="#ff7a1a" stroke-width="2"/>'
             f'<text x="{rx + 14}" y="{ey + 24}" font-size="13" font-weight="700" fill="#ff7a1a">{T(lang, "Fonte da placa de relés", "Relay board supply")}</text>'
             f'<text x="{rx + 14}" y="{ey + 43}" font-size="12" fill="#555">{T(lang, "a mesma de antes (12/24 V)", "the same as before (12/24 V)")}</text>')
    o.append(f'<text x="{W / 2}" y="{H - 14}" font-size="12" text-anchor="middle" fill="#888">{T(lang, "Assistente → Relés → ligam com sinal ALTO", "Wizard → Relays → switch on with HIGH")}</text>')
    o.append('</svg>')
    return ''.join(o)

def mcp(lang):
    W, H = 1100, 760
    o = [head(W, H, T(lang, 'MCP23017 + placa de 16 relés', 'MCP23017 + 16-channel relay board'))]
    bx, bw = 30, 260
    o.append(board(bx, 80, bw, 220, 'Waveshare ESP32-S3-ETH', 'I²C'))
    mx, mw = 400, 260
    o.append(board(mx, 80, mw, 600, 'MCP23017', T(lang, 'endereço 0x20 (A0-A2 a GND)', 'address 0x20 (A0-A2 to GND)')))
    rx, rw = 790, 280
    o.append(board(rx, 80, rw, 600, T(lang, 'Placa de 16 relés', '16-channel relay board'), T(lang, 'com optoacopladores', 'opto-isolated')))
    pairs = [('3V3', 'VCC', '#e6194b'), ('GND', 'GND', '#000'), ('GPIO16', 'SDA', '#4363d8'), ('GPIO17', 'SCL', '#3cb44b')]
    for i, (a, b, c) in enumerate(pairs):
        y = 140 + i * 36
        o.append(pin(bx + bw, y, a, 'r', c)); o.append(pin(mx, y, b, 'l', c)); o.append(wire(bx + bw + 5, y, mx - 5, y, c))
    names = [f'GPA{i}' for i in range(8)] + [f'GPB{i}' for i in range(8)]
    for i, n in enumerate(names):
        y = 300 + i * 22
        c = COLORS[i]
        o.append(pin(mx + mw, y, n, 'r', c)); o.append(pin(rx, y, f'IN{i + 1}', 'l', c))
        o.append(wire(mx + mw + 5, y, rx - 5, y, c, 2.5))
    # power of the relay board
    o.append(pin(rx, 180, 'VCC  ←  3,3 V' if lang == 'pt' else 'VCC  ←  3.3 V', 'l', '#e6194b'))
    o.append(pin(rx, 214, 'JD-VCC  ←  ' + T(lang, 'fonte dos relés', 'relay supply'), 'l', '#ff7a1a'))
    o.append(pin(rx, 248, 'GND', 'l', '#000'))
    o.append(f'<text x="{rx + 14}" y="{276}" font-size="12" fill="#b55a00">{T(lang, "tira o jumper JD-VCC", "remove the JD-VCC jumper")}</text>')
    o.append(f'<text x="{W / 2}" y="{H - 30}" font-size="12" text-anchor="middle" fill="#888">{T(lang, "Assistente → Relés → ligam com sinal BAIXO", "Wizard → Relays → switch on with LOW")}</text>')
    o.append('</svg>')
    return ''.join(o)

def loads(lang):
    W, H = 1000, 560
    o = [head(W, H, T(lang, 'Lado dos 230 V (eletricista)', 'Mains side (electrician)'))]
    o.append(f'<text x="20" y="58" font-size="13" fill="#b00">{T(lang, "⚠ Desliga o disjuntor antes de mexer. Igual ao que já tinhas com o DAEnetIP4.", "⚠ Switch off the breaker first. Same as you had with the DAEnetIP4.")}</text>')
    # mains rails
    o.append('<line x1="60" y1="100" x2="60" y2="520" stroke="#8B4513" stroke-width="5"/><text x="48" y="96" font-size="14" font-weight="700" fill="#8B4513">L</text>')
    o.append('<line x1="940" y1="100" x2="940" y2="520" stroke="#1f5fbf" stroke-width="5"/><text x="932" y="96" font-size="14" font-weight="700" fill="#1f5fbf">N</text>')
    items = [
        (T(lang, 'Relés das zonas (COM → NA)', 'Zone relays (COM → NO)'), T(lang, 'Atuador térmico da zona (NF)', 'Zone thermal actuator (NC)'), '#ff7a1a', True),
        (T(lang, 'Relé da bomba (COM → NA)', 'Pump relay (COM → NO)'), T(lang, 'Bomba circuladora (contactor se for grande)', 'Circulation pump (contactor if large)'), '#1784F7', True),
        (T(lang, 'Relé do pedido de calor', 'Heat demand relay'), T(lang, 'Entrada de termóstato da bomba de calor / caldeira', 'Heat pump / boiler room-thermostat input'), '#3cb44b', False),
    ]
    for i, (rl, ld, c, mains) in enumerate(items):
        y = 150 + i * 130
        o.append(f'<rect x="180" y="{y - 30}" width="230" height="60" rx="10" fill="#f4f7fb" stroke="#0B1B3F" stroke-width="2"/>'
                 f'<text x="295" y="{y + 5}" font-size="13" text-anchor="middle" font-weight="700">{rl}</text>')
        o.append(f'<rect x="560" y="{y - 30}" width="320" height="60" rx="10" fill="#fff" stroke="{c}" stroke-width="2"/>'
                 f'<text x="720" y="{y + 5}" font-size="13" text-anchor="middle">{ld}</text>')
        if mains:
            o.append(f'<line x1="60" y1="{y}" x2="180" y2="{y}" stroke="#8B4513" stroke-width="3"/>')
            o.append(f'<line x1="410" y1="{y}" x2="560" y2="{y}" stroke="#8B4513" stroke-width="3"/>')
            o.append(f'<line x1="880" y1="{y}" x2="940" y2="{y}" stroke="#1f5fbf" stroke-width="3"/>')
        else:
            o.append(f'<path d="M410 {y - 10} H560 M410 {y + 10} H560" stroke="#3cb44b" stroke-width="3"/>'
                     f'<text x="485" y="{y + 32}" font-size="11" text-anchor="middle" fill="#3cb44b">{T(lang, "sem 230 V", "no mains")}</text>')
    o.append('</svg>')
    return ''.join(o)

if __name__ == '__main__':
    for lang in ('pt', 'en'):
        for name, fn in (('direct', direct), ('mcp23017', mcp), ('mains', loads)):
            open(f'docs/img/wiring-{name}-{lang}.svg', 'w').write(fn(lang))
    print('ok')
