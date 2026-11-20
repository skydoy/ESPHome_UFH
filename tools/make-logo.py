# Generates the ESPHome UFH logo (original artwork: underfloor heating loop + heat waves).
# Usage: pip install fonttools resvg_py; ROBOTO=/path/Roboto[wdth,wght].ttf python make-logo.py
# Run from docs/img/ (writes the SVGs and PNGs there). Brand gradient shared with ESPManiacs.
# Font: Roboto (Apache-2.0 / OFL); text is converted to paths so the SVGs need no font.
import os, resvg_py
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer
from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.transformPen import TransformPen
G1, G2 = '#0FE3FA', '#1784F7'
# Underfloor loop (serpentine pipe) in the lower half
x0, x1, ys, r = 128, 472, (318, 394, 470), 38
loop = f'M{x0} {ys[0]}H{x1-r}A{r} {r} 0 0 1 {x1-r} {ys[1]}H{x0+r}A{r} {r} 0 0 0 {x0+r} {ys[2]}H{x1}'
# Heat waves rising from the floor
def wave(x, y0=250, y1=96, a=22):
    h = (y0 - y1) / 4
    return (f'M{x} {y0}C{x+a} {y0-h*0.5} {x+a} {y0-h*1.5} {x} {y0-2*h}'
            f'S{x-a} {y0-h*3.5} {x} {y1}')
waves = ''.join(wave(x) for x in (210, 300, 390))
body = f'''<defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="{G1}"/><stop offset="1" stop-color="{G2}"/></linearGradient></defs>
<rect width="600" height="600" rx="128" fill="url(#g)"/>
<g fill="none" stroke="#fff" stroke-linecap="round" stroke-linejoin="round">
<path stroke-width="30" d="{loop}"/><path stroke-width="24" d="{waves}"/></g>'''
icon = f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 600 600" width="600" height="600">{body}</svg>'
open('icon.svg', 'w').write(icon)
open('favicon.svg', 'w').write(icon)
src = os.environ.get('ROBOTO', 'Roboto-VariableFont_wdth,wght.ttf')
def mk(w):
    f = instancer.instantiateVariableFont(TTFont(src), {'wght': w, 'wdth': 100})
    return f.getGlyphSet(), f.getBestCmap(), f['head'].unitsPerEm
FB = mk(700); FR = mk(400)
def tp(F, s, size, x0, base):
    gs, cmap, upm = F; sc = size / upm; x = 0; pen = SVGPathPen(gs)
    for ch in s:
        g = cmap[ord(ch)]; gs[g].draw(TransformPen(pen, (sc, 0, 0, -sc, x0 + x * sc, base))); x += gs[g].width
    return pen.getCommands(), x * sc
NAME = 'ESPHome UFH'
_, TW = tp(FB, NAME, 140, 0, 0)
def full(color):
    W = max(860, TW + 40); ox = (W - 600) / 2; tx = (W - TW) / 2
    t, _ = tp(FB, NAME, 140, tx, 745)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W:.0f} 780" width="{W:.0f}" height="780">'
            f'<g transform="translate({ox:.1f} 0)">{body}</g><path fill="{color}" d="{t}"/></svg>')
open('logo-light.svg', 'w').write(full('#0B1B3F'))
open('logo-dark.svg', 'w').write(full('#FFFFFF'))
W, H = 1280, 640; s = 0.55; ix = 70; iy = (H - 600 * s) / 2; tx = ix + 600 * s + 60
t, _ = tp(FB, NAME, 86, tx, 280)
l1, _ = tp(FR, 'Underfloor heating in Home Assistant', 36, tx, 350)
l2, _ = tp(FR, 'Zones, pump and heat demand. Ethernet + PoE.', 36, tx, 398)
l3, _ = tp(FR, 'M5Stack Unit PoE-P4  ·  ESPHome  ·  Local', 26, tx, 470)
soc = f'''<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">
<defs><linearGradient id="bg" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#06203A"/><stop offset="1" stop-color="#06304D"/></linearGradient></defs>
<rect width="{W}" height="{H}" fill="url(#bg)"/><g transform="translate({ix} {iy}) scale({s})">{body}</g>
<path fill="#FFFFFF" d="{t}"/><path fill="#CFEFFF" d="{l1}{l2}"/><path fill="{G1}" d="{l3}"/></svg>'''
open('social-preview.svg', 'w').write(soc)
def png(svgtxt, out, w=None, h=None):
    data = resvg_py.svg_to_bytes(svg_string=svgtxt, width=w, height=h) if w else resvg_py.svg_to_bytes(svg_string=svgtxt)
    open(out, 'wb').write(bytes(data))
png(icon, 'icon-512.png', 512, 512); png(icon, 'icon.png', 256, 256); png(icon, 'apple-touch-icon.png', 180, 180)
png(icon, 'favicon-32.png', 32, 32)
LH = round(512 * 780 / max(860, TW + 40)); png(full('#0B1B3F'), 'logo-light.png', 512, LH); png(full('#FFFFFF'), 'logo-dark.png', 512, LH)
png(soc, 'social-preview.png', W, H)
print('ok', TW)
