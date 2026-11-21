// ESPHome_UFH web_server v3 add-ons (the only js_include allowed):
//  1) project favicon (heating loop) via data: URI;
//  2) cap the debug log panel height;
//  3) PT/EN language toggle + entity/group/option name translation (MutationObserver);
//  4) UFH panel: live diagram (manifold, zones, pump, heat pump), setup wizard, zone editor
//     and schedules. Talks to the device through /ufh/state and /ufh/config.
// Display only: option values sent to the device (Pump Mode: Auto / On / Off) stay in English.

(function () {
  var HREF = 'data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA2MDAgNjAwIiB3aWR0aD0iNjAwIiBoZWlnaHQ9IjYwMCI+PGRlZnM+PGxpbmVhckdyYWRpZW50IGlkPSJnIiB4MT0iMCIgeTE9IjAiIHgyPSIxIiB5Mj0iMSI+PHN0b3Agb2Zmc2V0PSIwIiBzdG9wLWNvbG9yPSIjMEZFM0ZBIi8+PHN0b3Agb2Zmc2V0PSIxIiBzdG9wLWNvbG9yPSIjMTc4NEY3Ii8+PC9saW5lYXJHcmFkaWVudD48L2RlZnM+CjxyZWN0IHdpZHRoPSI2MDAiIGhlaWdodD0iNjAwIiByeD0iMTI4IiBmaWxsPSJ1cmwoI2cpIi8+CjxnIGZpbGw9Im5vbmUiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj4KPHBhdGggc3Ryb2tlLXdpZHRoPSIzMCIgZD0iTTEyOCAzMThINDM0QTM4IDM4IDAgMCAxIDQzNCAzOTRIMTY2QTM4IDM4IDAgMCAwIDE2NiA0NzBINDcyIi8+PHBhdGggc3Ryb2tlLXdpZHRoPSIyNCIgZD0iTTIxMCAyNTBDMjMyIDIzMC43NSAyMzIgMTkyLjI1IDIxMCAxNzMuMFMxODggMTE1LjI1IDIxMCA5Nk0zMDAgMjUwQzMyMiAyMzAuNzUgMzIyIDE5Mi4yNSAzMDAgMTczLjBTMjc4IDExNS4yNSAzMDAgOTZNMzkwIDI1MEM0MTIgMjMwLjc1IDQxMiAxOTIuMjUgMzkwIDE3My4wUzM2OCAxMTUuMjUgMzkwIDk2Ii8+PC9nPjwvc3ZnPg==';
  function applyFav() {
    var head = document.head || document.getElementsByTagName('head')[0];
    if (!head) return;
    var links = head.querySelectorAll('link[rel="icon"],link[rel="shortcut icon"]');
    if (!links.length) {
      var l = document.createElement('link');
      l.rel = 'icon'; l.type = 'image/svg+xml'; l.href = HREF; head.appendChild(l);
    } else {
      for (var i = 0; i < links.length; i++) { links[i].type = 'image/svg+xml'; links[i].href = HREF; }
    }
  }
  if (document.head) applyFav(); else document.addEventListener('DOMContentLoaded', applyFav);
})();

(function () {
  function root() { var a = document.querySelector('esp-app'); return a && a.shadowRoot; }
  function fixLog() {
    var sr = root(), log = sr && sr.querySelector('esp-log'), lr = log && log.shadowRoot;
    if (!lr) return false;
    if (!lr.getElementById('web-log-css')) {
      var st = document.createElement('style');
      st.id = 'web-log-css';
      st.textContent = '.logs{max-height:min(45vh,420px);overflow-y:auto;overscroll-behavior:contain}';
      lr.appendChild(st);
    }
    return true;
  }
  if (!fixLog()) { var n = 0, t = setInterval(function () { if (fixLog() || ++n > 40) clearInterval(t); }, 250); }
})();

(function () {
  var ENT_PT = {"House Mode": "Modo da casa", "Home": "Casa", "Away": "Ausente", "Holiday": "Férias", "Summer": "Verão", "Pump Mode": "Modo da bomba", "Auto": "Automático", "On": "Ligada", "Off": "Desligada", "Pump": "Bomba", "Heat Demand": "Pedido de calor", "Problem": "Problema", "Warnings": "Avisos", "Active Zones": "Zonas ativas", "All Zones Off": "Desligar todas as zonas", "Pump Runtime Today": "Bomba: horas hoje", "Pump Runtime Total": "Bomba: horas total", "Status LED": "LED de estado", "Firmware Update": "Atualização do firmware", "Auto Update": "Atualização automática", "Update Status": "Estado da atualização", "Check for Update": "Verificar atualizações", "Install Update": "Instalar atualização", "Latest Version": "Última versão", "Update Available": "Atualização disponível", "Firmware Version": "Versão do firmware", "ESPHome Version": "Versão do ESPHome", "Board": "Placa", "Uptime": "Tempo ligado", "Reset Reason": "Motivo do reinício", "IP Address": "Endereço IP", "MAC Address": "Endereço MAC", "Restart": "Reiniciar", "Restart (Safe Mode)": "Reiniciar (modo seguro)", "Zone 1": "Zona 1", "Zone 2": "Zona 2", "Zone 3": "Zona 3", "Zone 4": "Zona 4", "Zone 5": "Zona 5", "Zone 6": "Zona 6", "Zone 7": "Zona 7", "Zone 8": "Zona 8", "Zone 9": "Zona 9", "Zone 10": "Zona 10", "Zone 11": "Zona 11", "Zone 12": "Zona 12", "Zone 13": "Zona 13", "Zone 14": "Zona 14", "Zone 15": "Zona 15", "Zone 16": "Zona 16", "Main Thermostat": "Termóstato geral", "Pump Start Delay": "Atraso a ligar", "Pump Stop Delay": "Atraso a desligar", "Heat Pump Min On Time": "Bomba de calor: mínimo ligada", "Heat Pump Min Off Time": "Bomba de calor: mínimo desligada"};
  var GROUP_PT = {"Pump": "Bomba", "Main": "Geral", "Zones": "Zonas", "House": "Casa", "Statistics": "Estatísticas", "Maintenance": "Manutenção", "Configuration": "Configuração", "Diagnostic": "Diagnóstico", "Undefined": "Outros"};
  var KEY = 'esphome_ufh_lang', lang;
  try { lang = localStorage.getItem(KEY); } catch (e) {}
  if (lang !== 'pt' && lang !== 'en') lang = /^pt/i.test(navigator.language || '') ? 'pt' : 'en';
  window.__ufhLang = {
    get: function () { return lang; },
    set: function (l) {
      lang = l === 'pt' ? 'pt' : 'en';
      try { localStorage.setItem(KEY, lang); } catch (e) {}
      if (window.__ufhOnLang) window.__ufhOnLang(lang);
      try { window.dispatchEvent(new Event('ufh-lang')); } catch (e) {}
    },
    t: function (pt, en) { return lang === 'pt' ? pt : en; }
  };

  function root() { var a = document.querySelector('esp-app'); return a && a.shadowRoot; }
  function tableEl() {
    var sr = root(), t = sr && sr.querySelector('esp-entity-table');
    return t && t.shadowRoot ? t : null;
  }
  // Display-only value translations (never change option/button values sent to the device).
  function uiString(v) {
    if (lang !== 'pt' || v == null) return v;
    var s = String(v);
    var m = /^New version (.+) available, install from Device Builder$/.exec(s);
    if (m) return 'Há uma versão nova ' + m[1] + ' disponível — instala-a no Device Builder';
    m = /^Up to date \((.+)\)$/.exec(s);
    if (m) return 'Atualizado (' + m[1] + ')';
    if (/^Up to date$/i.test(s)) return 'Atualizado';
    m = /^Update available: (.+)$/.exec(s);
    if (m) return 'Atualização disponível: ' + m[1];
    // Firmware Update entity states (web_server v3 shows hyphenated "Up-to-date")
    if (/^Up-to-date$/i.test(s) || /^Up to date$/i.test(s) || /^NO UPDATE$/i.test(s)) return 'Atualizado';
    if (/^Update available$/i.test(s) || /^UPDATE AVAILABLE$/i.test(s)) return 'Atualização disponível';
    if (/^Installing(\u2026|\.\.\.|…)?$/i.test(s) || /^INSTALLING$/i.test(s)) return 'A instalar';
    if (/^Checking(\u2026|\.\.\.|…)$/i.test(s) || /^Checking for updates$/i.test(s)) return 'A procurar\u2026';
    if (/^Update check failed/i.test(s)) return s.replace(/^Update check failed/i, 'Falha ao procurar atualização');
    var CONTENT_PT = {
      'Install requested': 'Instalação pedida',
      'Downloading firmware (do not power off)': 'A transferir o firmware (não desligues)',
      'Installed, rebooting': 'Instalado, a reiniciar',
      // Reset Reason — ESP8266 (Arduino)
      'Power On': 'Ligado',
      'Power on': 'Ligado',
      'Hardware Watchdog': 'Watchdog de hardware',
      'Exception': 'Exceção',
      'Software Watchdog': 'Watchdog de software',
      'Software/System restart': 'Reinício por software',
      'Software reset': 'Reinício por software',
      'Deep-Sleep Wake': 'Acordar do deep sleep',
      'External System': 'Sistema externo',
      // Reset Reason — ESP32 (ESP-IDF / ESPHome debug)
      'unknown source': 'origem desconhecida',
      'power-on event': 'Ligado',
      'external pin': 'Pino externo',
      'software via esp_restart': 'Reinício por software',
      'exception/panic': 'Exceção/pânico',
      'interrupt watchdog': 'Watchdog de interrupção',
      'task watchdog': 'Watchdog de tarefa',
      'other watchdogs': 'Outros watchdogs',
      'exiting deep sleep mode': 'A sair do deep sleep',
      'brownout': 'Quebra de tensão',
      'Brownout': 'Quebra de tensão',
      'SDIO': 'SDIO',
      'USB peripheral': 'Periférico USB',
      'JTAG': 'JTAG',
      'efuse error': 'Erro de eFuse',
      'power glitch detected': 'Oscilação de alimentação',
      'CPU lock up': 'CPU bloqueada'
    };
    if (CONTENT_PT[s]) return CONTENT_PT[s];
    if (/^Update failed \(/.test(s)) return s.replace('Update failed', 'Falha na atualização');
    m = /^Reboot request from (.+)$/.exec(s);
    if (m) return 'Pedido de reinício de ' + m[1];
    return s;
  }
  var SUFFIX_PT = [[' Heating', ' · a aquecer'], [' Temperature', ' · temperatura'], [' Runtime Today', ' · horas hoje'],
                   [' Runtime Total', ' · horas total'], [' Duty 24h', ' · ocupação 24 h']];
  function suffixPT(en) {
    for (var i = 0; i < SUFFIX_PT.length; i++) {
      var sf = SUFFIX_PT[i][0];
      if (en.length > sf.length && en.slice(-sf.length) === sf) {
        var base = en.slice(0, -sf.length);
        return (ENT_PT[base] || base) + SUFFIX_PT[i][1];
      }
    }
    return null;
  }

  // ---- one collapsible group per zone ("Zone N" sorting groups) ----
  // Header: "▸ <zone name>" + live summary (temperature → target · heating), click opens/closes.
  // Inside a group the rows lose the repeated zone name ("Thermostat", "Heating", ...).
  var OPEN_KEY = 'esphome_ufh_open_zones';
  function openZones() { try { return JSON.parse(localStorage.getItem(OPEN_KEY) || '[]'); } catch (e) { return []; } }
  function setOpen(n, on) {
    var o = openZones().filter(function (x) { return x !== n; });
    if (on) o.push(n);
    try { localStorage.setItem(OPEN_KEY, JSON.stringify(o)); } catch (e) {}
  }
  var ROW_LBL = {'': ['Termóstato', 'Thermostat'], 'Follows Main': ['Segue a geral', 'Follows main'], 'Last Reading': ['Última leitura', 'Last reading'], 'Heating': ['A aquecer', 'Heating'], 'Temperature': ['Temperatura', 'Temperature'],
                 'Runtime Today': ['Horas hoje', 'Runtime today'], 'Runtime Total': ['Horas total', 'Runtime total'],
                 'Duty 24h': ['Ocupação 24 h', 'Duty 24 h']};
  function TT(pt, en) { return lang === 'pt' ? pt : en; }
  function ago(m) { return m < 60 ? m + ' min' : (m < 2880 ? Math.round(m / 60) + ' h' : Math.round(m / 1440) + TT(' dias', ' days')); }
  function fmt(v) { return (v === null || v === undefined || isNaN(v)) ? '—' : (+v).toFixed(1) + ' °C'; }
  function zoneGroups(sr) {
    var st = window.__ufhState, zs = (st && st.zones) || [], open = openZones();
    sr.querySelectorAll('.tab-header').forEach(function (h) {
      if (h.__zone === undefined) {
        var ht = h.textContent.trim(), m = /^Zon[ea] (\d+)$/.exec(ht);
        h.__zone = m ? +m[1] : (ht === 'Pump' || ht === 'Bomba' ? 100 : (ht === 'Main' || ht === 'Geral' ? 101 : 0));
      }
      var n = h.__zone;
      if (!n) return;
      var box = h.nextElementSibling, z = zs[n - 1] || {}, name = z.n || ((lang === 'pt' ? 'Zona ' : 'Zone ') + n);
      var isOpen = open.indexOf(n) >= 0;
      var sum;
      if (n === 100) {
        // pump / heat pump group
        name = TT('Bomba', 'Pump');
        z = {};
        sum = st ? ((st.pump ? '🟢 ' + TT('ligada', 'on') : TT('desligada', 'off')) +
          (st.pump_in > 0 ? ' · ' + TT('liga em ', 'starts in ') + st.pump_in + ' s' : '') +
          ' · ' + [TT('automático', 'auto'), TT('sempre ligada', 'always on'), TT('desligada', 'off')][st.pump_mode || 0] +
          (st.ps != null ? ' · ⏱ ' + st.ps + ' s / ' + st.pe + ' s' : '')) : '';
      } else if (n === 101) {
        // main thermostat group
        name = TT('Geral', 'Main');
        var M = st && st.main;
        z = {};
        if (M) {
          var nf = zs.filter(function (q) { return q.fm; }).length;
          sum = M.m === 0 ? TT('desligada', 'off') :
            ((M.c ? TT('Conforto', 'Comfort') : 'Eco') + ' ' + fmt(M.tg) + (M.ovr ? TT(' (à mão)', ' (manual)') : '') + (M.m === 2 ? TT(' · fixo', ' · fixed') : ''));
          sum += ' · ' + nf + (nf === 1 ? TT(' zona', ' zone') : TT(' zonas', ' zones'));
        } else sum = '';
      } else if (z.lead) {
        var L = zs[z.lead - 1];
        sum = TT('segue ', 'follows ') + (L && L.n ? L.n : 'Z' + z.lead);
      } else if (z.n !== undefined) {
        sum = (z.fm ? TT('geral · ', 'main · ') : '') + (z.age >= 0 ? TT('há ', '') + ago(z.age) + TT('', ' ago') + ' · ' : '') + fmt(z.t) + ' → ' + (z.m === 0 || z.tg === null || isNaN(z.tg) ? TT('desligada', 'off') : fmt(z.tg));
      } else sum = '';
      if (z.r) sum += (sum ? ' · ' : '') + '🔥 ' + TT('a aquecer', 'heating');
      if (z.w) sum += ' ⚠️';
      var key = (isOpen ? 1 : 0) + '|' + name + '|' + sum;
      if (h.__key !== key) {
        h.__key = key;
        h.textContent = '';
        h.style.cssText = 'cursor:pointer;display:flex;gap:8px;align-items:baseline;user-select:none;max-width:none;width:100%;box-sizing:border-box;border-radius:' + (isOpen ? '12px 12px 0 0' : '12px');
        var c = document.createElement('span'); c.textContent = isOpen ? '▾' : '▸'; c.style.cssText = 'width:1em;display:inline-block';
        var t = document.createElement('span'); t.textContent = name;
        var r = document.createElement('span'); r.textContent = sum; r.style.cssText = 'margin-left:auto;font-weight:400;opacity:.8;font-size:.9em';
        h.appendChild(c); h.appendChild(t); h.appendChild(r);
      }
      if (box && box.classList.contains('tab-container')) {
        box.style.display = isOpen ? '' : 'none';
        // shorter row names inside the group
        if (isOpen && z.n) box.querySelectorAll('.entity-row').forEach(function (row) {
          var nb = row.children && row.children[1];
          if (!nb) return;
          for (var c2 = 0; c2 < nb.childNodes.length; c2++) {
            var tn = nb.childNodes[c2];
            if (tn.nodeType !== 3 || !tn.nodeValue.trim()) continue;
            if (tn.__en === undefined) tn.__en = tn.nodeValue.trim();
            var en = tn.__en, rest = en === z.n ? '' : (en.indexOf(z.n + ' ') === 0 ? en.slice(z.n.length + 1) : null);
            if (rest !== null && ROW_LBL[rest]) tn.nodeValue = ROW_LBL[rest][lang === 'pt' ? 0 : 1];
            var val = row.children[2];
            // last reading: epoch -> local time
            if (rest === 'Last Reading' && val) {
              var w2 = document.createTreeWalker(val, NodeFilter.SHOW_TEXT), vn;
              while ((vn = w2.nextNode())) {
                var num = parseFloat(vn.nodeValue);
                if (/^\s*\d{9,}(\.\d+)?\s*$/.test(vn.nodeValue) && num > 1e9) {
                  var d = new Date(num * 1000);
                  vn.nodeValue = d.toLocaleString(lang === 'pt' ? 'pt-PT' : 'en-GB', {day: '2-digit', month: '2-digit', hour: '2-digit', minute: '2-digit'});
                }
              }
            }
            // thermostat of a zone that follows the main one: no slider, only a note
            if (rest === '' && val) {
              val.querySelectorAll('input[type=range]').forEach(function (r) {
                r.style.display = z.fm ? 'none' : '';
                var note = r.parentNode && r.parentNode.querySelector('.ufh-fm');
                if (z.fm && !note) {
                  note = document.createElement('span'); note.className = 'ufh-fm'; note.style.cssText = 'opacity:.8;margin-right:8px';
                  r.parentNode.insertBefore(note, r);
                }
                if (note) { note.textContent = z.fm ? '🏠 ' + TT('segue a geral', 'follows main') : ''; }
              });
            }
          }
        });
      }
      if (!h.__ufhClick) {
        h.__ufhClick = true;
        h.addEventListener('click', function () {
          setOpen(h.__zone, openZones().indexOf(h.__zone) < 0);
          h.__key = '';
          zoneGroups(sr);
        });
      }
    });
  }
  window.__ufhGroups = function () { var tb = tableEl(); if (tb) zoneGroups(tb.shadowRoot); };
  function translateTable() {
    var tb = tableEl();
    if (!tb) return false;
    var sr = tb.shadowRoot;
    if (tb.entities) {
      var rows = sr.querySelectorAll('.entity-row');
      for (var i = 0; i < tb.entities.length && i < rows.length; i++) {
        var box = rows[i].children && rows[i].children[1];
        if (!box) continue;
        var tn = null;
        for (var c = 0; c < box.childNodes.length; c++)
          if (box.childNodes[c].nodeType === 3 && box.childNodes[c].nodeValue.trim()) tn = box.childNodes[c];
        if (tn) {
          if (tn.__en === undefined) tn.__en = tn.nodeValue.trim();
          var en = tn.__en, pt = ENT_PT[en] || suffixPT(en);
          tn.nodeValue = lang === 'pt' && pt ? pt : en;
        }
      }
    }
    sr.querySelectorAll('.entity-row').forEach(function (row) {
      var nameEl = row.children && row.children[1], name = nameEl && nameEl.textContent;
      if (!name) return;
      var bare = name.trim().replace(/^\[[^\]]+\]\s*/, '').trim();
      var isStatus = (bare === 'Update Status' || bare === 'Estado da atualização');
      var isReset = (bare === 'Reset Reason' || bare === 'Motivo do reinício');
      var isFwUpdate = (bare === 'Firmware Update' || bare === 'Atualização do firmware');
      if (!isStatus && !isReset && !isFwUpdate) return;
      var walker = document.createTreeWalker(row, NodeFilter.SHOW_TEXT), nodes = [], n;
      while ((n = walker.nextNode())) nodes.push(n);
      nodes.forEach(function (node) {
        var txt = node.nodeValue.trim();
        if (!txt || node.parentNode === nameEl) return;
        if (node.__valEn === undefined) {
          // Capture English once (before we rewrite for PT)
          if ((isStatus || isFwUpdate) && /^(New version|Up to date|Up-to-date|Update available|Installing|Checking|Install requested|Downloading firmware|Installed, rebooting|Update failed|Update check failed|NO UPDATE|UPDATE AVAILABLE|INSTALLING|UNKNOWN)/i.test(txt))
            node.__valEn = txt;
          else if (isReset) node.__valEn = txt;
        }
        if (node.__valEn !== undefined) node.nodeValue = uiString(node.__valEn);
      });
    });
    sr.querySelectorAll('.tab-header').forEach(function (h) {
      var v = h.textContent.trim();
      Object.keys(GROUP_PT).forEach(function (en) {
        if (lang === 'pt' && v === en) h.textContent = GROUP_PT[en];
        if (lang === 'en' && v === GROUP_PT[en]) h.textContent = en;
      });
    });
    sr.querySelectorAll('.abutton,.abuttonIsState').forEach(function (b) {
      var v = b.textContent.trim();
      if (!/^(PRESS|PREMIR|CARREGAR)$/.test(v)) return;
      b.textContent = lang === 'pt' ? 'CARREGAR' : 'PRESS';
    });
    // <option>: translate label only; keep value attribute in English (device receives English)
    sr.querySelectorAll('option').forEach(function (el) {
      var ov = el.getAttribute('value');
      if (el.__enOpt === undefined) el.__enOpt = (el.textContent || '').trim();
      var en = el.__enOpt;
      var pt = ENT_PT[en];
      el.textContent = (lang === 'pt' && pt) ? pt : en;
      if (ov != null) el.setAttribute('value', ov);
    });
    zoneGroups(sr);
    if (!tb.__langObs) {
      tb.__langObs = new MutationObserver(function () { translateTableSoon(); });
      tb.__langObs.observe(sr, { childList: true, subtree: true });
    }
    return true;
  }
  var tTimer = null;
  function translateTableSoon() {
    if (tTimer) return;
    tTimer = setTimeout(function () { tTimer = null; translateTable(); }, 30);
  }
  function placeLangBtn() {
    var sr = root();
    if (!sr || sr.getElementById('ufh-lang')) return !!sr;
    var b = document.createElement('button');
    b.id = 'ufh-lang';
    b.type = 'button';
    b.style.cssText = 'margin:8px 16px;padding:6px 12px;border-radius:999px;border:1px solid #c9ccd1;background:#fff;font-weight:700;cursor:pointer';
    function paint() { b.textContent = lang === 'pt' ? 'PT · EN' : 'EN · PT'; b.title = lang === 'pt' ? 'Mudar língua (PT/EN)' : 'Switch language (PT/EN)'; }
    paint();
    b.onclick = function () {
      window.__ufhLang.set(lang === 'pt' ? 'en' : 'pt');
      paint();
      translateTable();
    };
    var main = sr.querySelector('esp-entity-table') || sr.firstChild;
    if (main && main.parentNode) main.parentNode.insertBefore(b, main);
    else sr.appendChild(b);
    window.__ufhOnLang = function () { paint(); translateTable(); };
    return true;
  }
  function bootI18n() {
    placeLangBtn();
    translateTable();
  }
  if (!bootI18n()) {
    var n = 0, t = setInterval(function () { if (bootI18n() || ++n > 60) clearInterval(t); }, 250);
  }
})();

// ---------------------------------------------------------------------------------------------
// 4) UFH panel: live diagram + setup wizard + zone editor / schedules
// ---------------------------------------------------------------------------------------------
(function () {
  function L() { return (window.__ufhLang && window.__ufhLang.get()) || (/^pt/i.test(navigator.language || '') ? 'pt' : 'en'); }
  function T(pt, en) { return L() === 'pt' ? pt : en; }
  function esc(s) { return String(s == null ? '' : s).replace(/[&<>"']/g, function (c) { return {'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'}[c]; }); }
  function root() { var a = document.querySelector('esp-app'); return a && a.shadowRoot; }
  function fmt(v, d) { return v == null || isNaN(v) ? '–' : Number(v).toFixed(d == null ? 1 : d); }
  function slotToTime(s) { s = Math.max(0, Math.min(144, s | 0)) % 144; var m = s * 10; return ('0' + Math.floor(m / 60) % 25).slice(-2) + ':' + ('0' + m % 60).slice(-2); }
  function timeToSlot(t) { if (!t) return 0; var p = t.split(':'); var v = Math.round((+p[0] * 60 + +p[1]) / 10); return Math.max(0, Math.min(144, v)); }
  function getJSON(u) { return fetch(u, {cache: 'no-store'}).then(function (r) { if (!r.ok) throw new Error(r.status); return r.json(); }); }
  function postJSON(u, body) {
    return fetch(u, {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(body)})
      .then(function (r) { if (!r.ok) throw new Error(r.status); return r.json(); });
  }

  var SRC = function () { return [
    [0, T('Sem sensor (só horário)', 'No sensor (schedule only)')],
    [1, T('Shelly H&T / webhook (Wi-Fi)', 'Shelly H&T / webhook (Wi-Fi)')],
    [2, T('Sensor do Home Assistant', 'Home Assistant sensor')],
    [3, 'MQTT'],
    [4, T('Bluetooth BTHome (Shelly BLU, Xiaomi…)', 'Bluetooth BTHome (Shelly BLU, Xiaomi…)')],
    [5, T('Sonda DS18B20 com fio', 'Wired DS18B20 probe')]].filter(function (s) {
      // BTHome needs Bluetooth: hidden on boards without a radio (ESP32-P4)
      return s[0] !== 4 || !cfg || !cfg.features || cfg.features.ble !== false; }); };
  var ROLES = function () { return [[0, T('Livre', 'Unused')], [1, T('Bomba circuladora', 'Circulation pump')],
    [2, T('Pedido de calor (bomba de calor / caldeira)', 'Heat demand (heat pump / boiler)')], [3, T('Bomba + pedido de calor', 'Pump + heat demand')]]; };
  var DAYS = function () { return L() === 'pt' ? ['Seg', 'Ter', 'Qua', 'Qui', 'Sex', 'Sáb', 'Dom'] : ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']; };
  var HOUSE = function () { return [T('Casa', 'Home'), T('Ausente', 'Away'), T('Férias', 'Holiday'), T('Verão', 'Summer')]; };

  var CSS = [
    '.ufh{margin:8px 16px 16px;padding:12px;border:1px solid rgba(127,127,127,.3);border-radius:12px;font:14px/1.35 system-ui,Segoe UI,Roboto,sans-serif}',
    '.ufh h3{margin:0 0 8px;font-size:15px;display:flex;gap:8px;align-items:center;flex-wrap:wrap}',
    '.ufh .chip{padding:2px 9px;border-radius:999px;background:rgba(23,132,247,.12);font-weight:600;font-size:12px}',
    '.ufh .warn{background:rgba(230,120,0,.15);color:#b55a00}',
    '.ufh .bar{display:flex;gap:6px;flex-wrap:wrap;margin-top:8px}',
    '.ufh button,.ufhm button{padding:6px 12px;border-radius:999px;border:1px solid #c9ccd1;background:#fff;color:#111;font-weight:600;cursor:pointer}',
    '.ufh button.pri,.ufhm button.pri{background:linear-gradient(135deg,#0FE3FA,#1784F7);color:#fff;border:0}',
    '.ufh svg{width:100%;height:auto;display:block}',
    '.ufh .z{cursor:pointer}',
    '.ufh .z:hover rect.box{stroke:#1784F7}',
    '@keyframes ufhspin{to{transform:rotate(360deg)}}',
    '.ufh .spin{animation:ufhspin 1.2s linear infinite;transform-box:fill-box;transform-origin:center}',
    '@keyframes ufhflow{to{stroke-dashoffset:-24}}',
    '.ufh .flow{stroke-dasharray:8 4;animation:ufhflow .8s linear infinite}',
    '.ufh .lbl{fill:#555}',
    '.ufh .warn{color:#b55a00}',
    '@media (prefers-color-scheme: dark){.ufh .lbl{fill:#c3ccd6}.ufh .chip{background:rgba(79,179,255,.18)}.ufh .warn{background:rgba(255,150,40,.18);color:#ffb35c}.ufh button{background:#2a2d33;color:#eee;border-color:#444}}'
  ].join('\n');
  var MCSS = [
    '.ufhm{position:fixed;inset:0;background:rgba(0,0,0,.45);z-index:9999;display:flex;align-items:flex-start;justify-content:center;overflow:auto;padding:24px 8px}',
    '.ufhm .card{background:#fff;color:#111;max-width:760px;width:100%;border-radius:14px;padding:18px 18px 14px;font:14px/1.4 system-ui,Segoe UI,Roboto,sans-serif;box-shadow:0 10px 40px rgba(0,0,0,.3)}',
    '.ufhm h2{margin:0 0 4px;font-size:18px}.ufhm .sub{color:#666;margin:0 0 12px}',
    '.ufhm button{padding:7px 14px;border-radius:999px;border:1px solid #c9ccd1;background:#fff;color:#111;font-weight:600;cursor:pointer;font:inherit;font-weight:600}',
    '.ufhm button.pri{background:linear-gradient(135deg,#0FE3FA,#1784F7);color:#fff;border:0}',
    '.ufhm .hint button{padding:3px 10px;font-size:12px}',
    '.ufhm label{display:block;margin:8px 0 2px;font-weight:600;font-size:13px}',
    '.ufhm input,.ufhm select{padding:6px 8px;border:1px solid #c9ccd1;border-radius:8px;font:inherit;max-width:100%}',
    '.ufhm input[type=text],.ufhm select.w{width:100%;box-sizing:border-box}',
    '.ufhm input[type=number]{width:80px}',
    '.ufhm .row{display:flex;gap:10px;flex-wrap:wrap;align-items:flex-end}',
    '.ufhm .zone{border:1px solid #e3e5e8;border-radius:10px;padding:10px;margin:8px 0}',
    '.ufhm .zone h4{margin:0 0 4px}',
    '.ufhm .hint{color:#666;font-size:12px;margin:4px 0}',
    '.ufhm code{background:#f2f4f7;padding:2px 5px;border-radius:5px;word-break:break-all;font-size:12px}',
    '.ufhm .nav{display:flex;justify-content:space-between;gap:8px;margin-top:14px;flex-wrap:wrap}',
    '.ufhm .steps{display:flex;gap:4px;margin:6px 0 12px;flex-wrap:wrap}',
    '.ufhm .steps span{font-size:12px;padding:2px 8px;border-radius:999px;background:#eef1f5}',
    '.ufhm .steps span.on{background:#1784F7;color:#fff}',
    '.ufhm table{border-collapse:collapse;width:100%}.ufhm td{padding:3px 4px;vertical-align:middle}',
    '.ufhm .opt{border-top:1px solid #eee;padding:8px 0}',
    '@media (prefers-color-scheme: dark){.ufhm .card{background:#1d1f23;color:#eee}.ufhm input,.ufhm select{background:#2a2d33;color:#eee;border-color:#444}.ufhm .zone{border-color:#333}.ufhm code{background:#2a2d33}.ufhm .steps span{background:#2a2d33}.ufhm .hint,.ufhm .sub{color:#aaa}.ufhm button{background:#2a2d33;color:#eee;border-color:#444}}'
  ].join('\n');

  var state = null, cfg = null, panel = null;
  function warnPT(w) {
    if (L() !== 'pt') return w;
    return w.replace(/^setup(; )?/, 'por configurar$1').replace(/: sensor/g, ': sensor sem dados').replace(/: battery/g, ': pilha fraca')
      .replace(/: not heating/g, ': não aquece').replace(/: floor limit/g, ': limite do chão').replace('pump on, no zone open', 'bomba ligada sem zonas abertas');
  }

  // ---------- live diagram ----------
  function diagram(st) {
    var zs = st.zones || [], n = zs.length;
    var cols = n <= 6 ? n : Math.ceil(n / 2), rows = n <= 6 ? 1 : 2;
    var zw = 118, zh = 92, gap = 10, left = 150;
    var W = Math.max(640, left + cols * (zw + gap) + 20), H = 70 + rows * (zh + 34) + 20;
    var o = ['<svg viewBox="0 0 ' + W + ' ' + H + '" xmlns="http://www.w3.org/2000/svg" font-family="system-ui,Segoe UI,Roboto,sans-serif">'];
    // manifold (flow on top, return below)
    var mx0 = left - 10, mx1 = left + cols * (zw + gap);
    var hot = st.any;
    o.push('<rect x="' + mx0 + '" y="20" width="' + (mx1 - mx0) + '" height="10" rx="5" fill="' + (hot ? '#ff7a1a' : '#9aa3ad') + '"/>');
    o.push('<rect x="' + mx0 + '" y="36" width="' + (mx1 - mx0) + '" height="10" rx="5" fill="' + (hot ? '#1784F7' : '#b9c1ca') + '"/>');
    o.push('<text x="' + (mx1 - 4) + '" y="16" text-anchor="end" font-size="11" class="lbl">' + T('ida', 'flow') + ' / ' + T('retorno', 'return') + '</text>');
    // pump + heat pump on the left
    var pumpOn = st.pump;
    o.push('<line x1="40" y1="25" x2="' + mx0 + '" y2="25" stroke="' + (pumpOn ? '#ff7a1a' : '#9aa3ad') + '" stroke-width="6"' + (pumpOn ? ' class="flow"' : '') + '/>');
    o.push('<line x1="40" y1="41" x2="' + mx0 + '" y2="41" stroke="' + (pumpOn ? '#1784F7' : '#b9c1ca') + '" stroke-width="6"' + (pumpOn ? ' class="flow"' : '') + '/>');
    o.push('<g transform="translate(95,33)"><circle r="17" fill="' + (pumpOn ? '#e8f7ff' : '#f1f3f5') + '" stroke="' + (pumpOn ? '#1784F7' : '#9aa3ad') + '" stroke-width="3"/>' +
      '<g' + (pumpOn ? ' class="spin"' : '') + '><path d="M0-11 L4-2 L-4-2Z M11 0 L2 4 L2-4Z M0 11 L-4 2 L4 2Z M-11 0 L-2-4 L-2 4Z" fill="' + (pumpOn ? '#1784F7' : '#9aa3ad') + '"/></g></g>');
    var pumpTxt = pumpOn ? T('Bomba a trabalhar', 'Pump running') : (st.pump_in != null ? T('Bomba arranca em ', 'Pump starts in ') + st.pump_in + ' s' : T('Bomba parada', 'Pump stopped'));
    o.push('<text x="95" y="68" text-anchor="middle" font-size="11" class="lbl">' + esc(pumpTxt) + '</text>');
    var hasDemand = (st.roles || []).some(function (r) { return r === 2 || r === 3; });
    var hp = st.demand;
    o.push('<g transform="translate(6,6)"><rect width="30" height="52" rx="6" fill="' + (hp ? '#fff1e6' : '#f1f3f5') + '" stroke="' + (hp ? '#ff7a1a' : '#9aa3ad') + '" stroke-width="2"/>' +
      (hp ? '<path d="M15 14c4 5 7 8 7 13a7 7 0 0 1-14 0c0-3 2-5 3-7 0 3 1 4 3 4-1-4 0-7 1-10z" fill="#ff7a1a"/>' : '<circle cx="15" cy="26" r="7" fill="none" stroke="#9aa3ad" stroke-width="2"/>') +
      '<text x="15" y="47" text-anchor="middle" font-size="8" fill="#666">' + (hasDemand ? T('BC', 'HP') : '–') + '</text></g>');
    if (st.altherma && (st.altherma.flow != null || st.altherma.ret != null))
      o.push('<text x="95" y="84" text-anchor="middle" font-size="11" class="lbl">' + fmt(st.altherma.flow) + '° / ' + fmt(st.altherma.ret) + '°</text>');
    // zones: pipes first (behind), then the boxes
    var pos = zs.map(function (z, i) { var c = i % cols, r = Math.floor(i / cols); return [left + c * (zw + gap), 64 + r * (zh + 34)]; });
    zs.forEach(function (z, i) {
      var x = pos[i][0], y = pos[i][1], on = z.r;
      o.push('<line x1="' + (x + zw / 2 - 8) + '" y1="30" x2="' + (x + zw / 2 - 8) + '" y2="' + y + '" stroke="' + (on ? '#ff7a1a' : '#c8cdd3') + '" stroke-width="3"/>');
      o.push('<line x1="' + (x + zw / 2 + 8) + '" y1="46" x2="' + (x + zw / 2 + 8) + '" y2="' + y + '" stroke="' + (on ? '#1784F7' : '#c8cdd3') + '" stroke-width="3"/>');
    });
    zs.forEach(function (z, i) {
      var x = pos[i][0], y = pos[i][1];
      var on = z.r, off = z.m === 0;
      var col = on ? '#ff7a1a' : (off ? '#b9c1ca' : '#1784F7');
      o.push('<g class="z" data-z="' + i + '">');
      o.push('<rect class="box" x="' + x + '" y="' + y + '" width="' + zw + '" height="' + zh + '" rx="10" fill="' + (on ? '#fff4ec' : '#f7f9fb') + '" stroke="' + col + '" stroke-width="2"/>');
      // serpentine loop (4 runs)
      var sx = x + 12, ex = x + zw - 12, sy = y + 56;
      var p = 'M' + sx + ' ' + sy + ' H' + ex + ' v7 H' + sx + ' v7 H' + ex + ' v7 H' + sx;
      o.push('<path d="' + p + '" fill="none" stroke="' + col + '" stroke-width="2.5" stroke-linejoin="round" stroke-linecap="round"' + (on ? ' class="flow"' : '') + '/>');
      o.push('<text x="' + (x + 8) + '" y="' + (y + 17) + '" font-size="12" font-weight="700" fill="#222">' + esc((z.n || '').slice(0, 15)) + '</text>');
      var t1 = z.lead ? T('segue ', 'follows ') + esc(((st.zones[z.lead - 1] || {}).n || ('#' + z.lead)).slice(0, 10))
        : (fmt(z.t) + '° → ' + (z.tg == null ? (off ? 'off' : '–') : fmt(z.tg) + '°'));
      o.push('<text x="' + (x + 8) + '" y="' + (y + 34) + '" font-size="12" fill="#333">' + t1 + '</text>');
      var badge = [];
      if (z.w & 1) badge.push('⚠ ' + T('sensor', 'sensor'));
      if (z.w & 2) badge.push('🔋');
      if (z.w & 4) badge.push('⚠ ' + T('não aquece', 'not heating'));
      if (z.w & 8) badge.push(T('chão', 'floor') + ' ⛔');
      if (z.w & 16) badge.push('🪟');
      if (z.early) badge.push('⏰');
      if (z.ovr) badge.push('✋');
      o.push('<text x="' + (x + 8) + '" y="' + (y + 48) + '" font-size="10" fill="#b55a00">' + esc(badge.join(' ')) + '</text>');
      o.push('<text x="' + (x + zw - 6) + '" y="' + (y + 17) + '" text-anchor="end" font-size="10" fill="#888">' + (i + 1) + '</text>');
      o.push('</g>');
    });
    o.push('</svg>');
    return o.join('');
  }

  function renderPanel() {
    if (!panel || !state) return;
    var st = state;
    var head = '<h3>🌡️ ' + T('Piso radiante', 'Underfloor heating') +
      ' <span class="chip">' + esc(HOUSE()[st.house] || '') + '</span>' +
      (st.time ? ' <span class="chip">' + esc(DAYS()[st.dow] || '') + ' ' + esc(st.time) + '</span>' : '') +
      (st.exercise ? ' <span class="chip">' + T('anti-bloqueio', 'anti-seize') + '</span>' : '') +
      (st.warnings && st.warnings !== 'setup' ? ' <span class="chip warn">⚠ ' + esc(warnPT(st.warnings)) + '</span>' : '') + '</h3>';
    var setup = !st.configured ? '<p><b>' + T('Primeira utilização: configura a instalação.', 'First run: set up your installation.') +
      '</b> ' + T('As zonas ficam desligadas até terminares o assistente.', 'Zones stay off until you finish the wizard.') + '</p>' : '';
    panel.innerHTML = head + setup + diagram(st) +
      '<div class="bar"><button class="pri" data-a="wizard">' + (st.configured ? T('⚙️ Definições', '⚙️ Settings') : T('▶ Começar a configuração', '▶ Start setup')) + '</button>' +
      (st.configured ? '<button data-a="sched">🗓️ ' + T('Horários e temperaturas', 'Schedules & temperatures') + '</button>' : '') +
      '<button data-a="balance">📊 ' + T('Equilíbrio', 'Balance') + '</button></div>';
  }

  function poll() {
    getJSON('/ufh/state').then(function (s) { state = s; window.__ufhState = s; renderPanel(); if (window.__ufhGroups) window.__ufhGroups(); }).catch(function () {});
  }

  // ---------- modal helpers ----------
  function modal(html) {
    closeModal();
    var m = document.createElement('div');
    m.className = 'ufhm';
    m.id = 'ufh-modal';
    m.innerHTML = '<div class="card">' + html + '</div>';
    m.addEventListener('click', function (e) { if (e.target === m) closeModal(); });
    document.body.appendChild(m);
    return m;
  }
  function closeModal() { var m = document.getElementById('ufh-modal'); if (m) m.remove(); }
  function val(m, sel) { var e = m.querySelector(sel); return e ? e.value : null; }
  function chk(m, sel) { var e = m.querySelector(sel); return !!(e && e.checked); }
  function opts(list, cur) { return list.map(function (o) { return '<option value="' + o[0] + '"' + (String(o[0]) === String(cur) ? ' selected' : '') + '>' + esc(o[1]) + '</option>'; }).join(''); }

  // ---------- wizard ----------
  var W = null;  // working copy of the config
  var step = 0;
  function stepsBar(names) { return '<div class="steps">' + names.map(function (n, i) { return '<span class="' + (i === step ? 'on' : '') + '">' + (i + 1) + '. ' + esc(n) + '</span>'; }).join('') + '</div>'; }
  function wizNames() { return [T('Relés', 'Relays'), T('Zonas e sensores', 'Zones & sensors'), T('Temperaturas e horários', 'Temperatures & schedules'), T('Opções', 'Options'), T('Gravar', 'Save')]; }

  function openWizard() {
    getJSON('/ufh/config').then(function (c) { cfg = c; W = JSON.parse(JSON.stringify(c)); step = 0; showStep(); })
      .catch(function () { alert(T('Não consegui ler a configuração.', 'Could not read the configuration.')); });
  }

  function collect(m) {
    if (step === 0) {
      W.zone_count = +val(m, '#zc');
      W.active_low = val(m, '#al') === '1';
      W.roles = W.roles.map(function (r, k) { var e = m.querySelector('#role' + k); return e ? +e.value : 0; });
    } else if (step === 1) {
      for (var i = 0; i < W.zone_count; i++) {
        var z = W.zones[i];
        z.name = (val(m, '#zn' + i) || '').trim() || ('Zone ' + (i + 1));
        z.leader = +val(m, '#zl' + i);
        z.room.type = +val(m, '#zs' + i);
        var p = m.querySelector('#zp' + i); if (p) z.room.param = p.value.trim();
        var k = m.querySelector('#zk' + i); if (k) z.room.key = k.value.trim();
        z.floor_max = +(val(m, '#zf' + i) || 0);
        z.floor.type = +(val(m, '#zfs' + i) || 0);
        var fp = m.querySelector('#zfp' + i); if (fp) z.floor.param = fp.value.trim();
        var fk = m.querySelector('#zfk' + i); if (fk) z.floor.key = fk.value.trim();
      }
    } else if (step === 2) {
      for (var j = 0; j < W.zone_count; j++) {
        var zz = W.zones[j];
        if (zz.leader) continue;
        zz.comfort = +val(m, '#zc' + j); zz.eco = +val(m, '#ze' + j);
        zz.mode = +val(m, '#zm' + j);
        zz.sched = readSched(m, 's' + j + '_', zz.sched);
      }
    } else if (step === 3) {
      W.opt = W.opt || {};
      ['pump_exercise', 'valve_exercise', 'window', 'optimum_start', 'pv', 'tariff', 'altherma'].forEach(function (k) { W.opt[k] = chk(m, '#o_' + k); });
      W.pump_start_s = +val(m, '#ps'); W.pump_stop_s = +val(m, '#pe');
      W.stale_min = Math.round(+val(m, '#stale') * 60);
      W.holiday = +val(m, '#hol');
      W.pv.delta = +val(m, '#pvd'); W.pv.topic = val(m, '#pvt').trim(); W.pv.key = val(m, '#pvk').trim(); W.pv.threshold_w = +val(m, '#pvw');
      var endSlot = function (a, b) { var e = timeToSlot(b); return e === 0 && timeToSlot(a) !== 0 ? 144 : e; };
      W.tariff.s1 = timeToSlot(val(m, '#ts1')); W.tariff.e1 = endSlot(val(m, '#ts1'), val(m, '#te1'));
      W.tariff.s2 = timeToSlot(val(m, '#ts2')); W.tariff.e2 = endSlot(val(m, '#ts2'), val(m, '#te2'));
      W.tariff.boost = +val(m, '#tb'); W.tariff.cut = +val(m, '#tc');
      W.altherma.host = val(m, '#ah').trim(); W.altherma.flow = val(m, '#af').trim(); W.altherma.ret = val(m, '#ar').trim();
      W.mqtt.host = val(m, '#mh').trim(); W.mqtt.port = +val(m, '#mp') || 1883; W.mqtt.user = val(m, '#mu').trim(); W.mqtt.pass = val(m, '#mw');
    }
  }

  function schedEditor(prefix, sc) {
    var d = DAYS(), h = '<table>';
    for (var k = 0; k < 7; k++) {
      var a = sc[k] || [0, 0, 0, 0];
      h += '<tr><td><b>' + d[k] + '</b></td>' +
        '<td><input type="time" step="600" id="' + prefix + k + 'a" value="' + slotToTime(a[0]) + '"> – <input type="time" step="600" id="' + prefix + k + 'b" value="' + slotToTime(a[1]) + '"></td>' +
        '<td><input type="time" step="600" id="' + prefix + k + 'c" value="' + slotToTime(a[2]) + '"> – <input type="time" step="600" id="' + prefix + k + 'd" value="' + slotToTime(a[3]) + '"></td></tr>';
    }
    return h + '</table><div class="hint">' + T('Dois períodos de Conforto por dia; fora deles é Eco. Para não usar um período, põe o início igual ao fim. Fim às 00:00 = meia-noite.',
      'Two Comfort periods per day; Eco outside them. To skip a period, set start = end. An end of 00:00 means midnight.') +
      ' <button type="button" data-copy="' + prefix + '">' + T('Copiar segunda para todos os dias úteis', 'Copy Monday to all weekdays') + '</button></div>';
  }
  function readSched(m, prefix, old) {
    var out = [];
    for (var k = 0; k < 7; k++) {
      var g = function (s) { var e = m.querySelector('#' + prefix + k + s); return e ? e.value : null; };
      if (g('a') == null) { out.push(old[k]); continue; }
      var e1 = g('b') === '00:00' && g('a') !== '00:00' ? 144 : timeToSlot(g('b'));
      var e2 = g('d') === '00:00' && g('c') !== '00:00' ? 144 : timeToSlot(g('d'));
      out.push([timeToSlot(g('a')), e1, timeToSlot(g('c')), e2]);
    }
    return out;
  }

  function sourceFields(i, which, src) {
    var idp = which === 'room' ? 'zp' : 'zfp', idk = which === 'room' ? 'zk' : 'zfk';
    var host = location.host, h = '';
    var zone = i + 1;
    if (src.type === 1) {
      h += '<div class="hint">' + T('Na app Shelly: sensor → Ações (Webhooks) → nova ação, evento “Temperatura mudou” e este URL:',
        'In the Shelly app: sensor → Actions (Webhooks) → new action, event “Temperature changed” and this URL:') +
        '<br><code>http://' + esc(host) + '/ufh/temp?zone=' + zone + (which === 'floor' ? '&amp;floor=1' : '') + '&amp;t=${ev.tC}</code></div>';
    } else if (src.type === 2) {
      h += '<div class="hint">' + T('Usa o blueprint “ESPHome UFH: temperatura do Home Assistant” (zona ', 'Use the blueprint “ESPHome UFH: temperature from Home Assistant” (zone ') + zone + ').</div>';
    } else if (src.type === 3) {
      h += '<div class="row"><div style="flex:2"><label>' + T('Tópico MQTT', 'MQTT topic') + '</label><input type="text" id="' + idp + i + '" value="' + esc(src.param) + '" placeholder="shellyplusht-xxxx/status/temperature:0"></div>' +
        '<div style="flex:1"><label>' + T('Campo JSON', 'JSON field') + '</label><input type="text" id="' + idk + i + '" value="' + esc(src.key) + '" placeholder="tC"></div></div>' +
        '<div class="hint">' + T('Shelly: campo “tC”. Zigbee2MQTT: “temperature”. Número simples: deixa vazio. Configura o broker em Opções.', 'Shelly: field “tC”. Zigbee2MQTT: “temperature”. Plain number: leave empty. Set the broker in Options.') + '</div>';
    } else if (src.type === 4) {
      var seen = (state && state.ble) || [];
      h += '<label>' + T('Endereço Bluetooth (MAC)', 'Bluetooth address (MAC)') + '</label><input type="text" id="' + idp + i + '" value="' + esc(src.param) + '" placeholder="AA:BB:CC:DD:EE:FF" list="ufh-ble">' +
        '<datalist id="ufh-ble">' + seen.map(function (b) { return '<option value="' + esc(b.mac) + '">' + fmt(b.t) + '° ' + b.rssi + ' dBm</option>'; }).join('') + '</datalist>' +
        '<div class="hint">' + T('Encontrados agora: ', 'Seen now: ') + (seen.length ? seen.map(function (b) { return esc(b.mac) + ' (' + fmt(b.t) + '°)'; }).join(', ') : T('nenhum (carrega no botão do sensor)', 'none (press the sensor button)')) + '</div>';
    } else if (src.type === 5) {
      var ow = (state && state.ow) || [];
      h += '<label>' + T('Sonda', 'Probe') + '</label><select class="w" id="' + idp + i + '">' + opts([['', '–']].concat(ow.map(function (o) { return [o.a, o.a + ' (' + fmt(o.t) + '°)']; })), src.param) + '</select>';
    }
    return h;
  }

  function showStep() {
    var names = wizNames(), h = '<h2>' + (W.configured ? T('Definições', 'Settings') : T('Configurar o piso radiante', 'Set up the underfloor heating')) + '</h2>' +
      '<p class="sub">' + esc(W.board) + '</p>' + stepsBar(names);
    var nR = W.relays;
    if (step === 0) {
      var zc = [];
      for (var k = 1; k <= nR; k++) zc.push([k, k]);
      h += '<label>' + T('Quantas zonas (circuitos) tens?', 'How many zones (loops) do you have?') + '</label><select id="zc">' + opts(zc, W.zone_count) + '</select>' +
        '<div class="hint">' + T('A zona N é o relé N. Os relés a seguir às zonas podem comandar a bomba e/ou pedir calor.', 'Zone N is relay N. The relays after the zones can drive the pump and/or ask for heat.') + '</div>' +
        '<label>' + T('Os relés ligam com o sinal…', 'The relays switch on with…') + '</label><select id="al">' +
        opts([[0, T('ALTO (Denkovi, maioria das placas com transístor)', 'HIGH (Denkovi, most transistor boards)')], [1, T('BAIXO (placas com optoacoplador “low level trigger”)', 'LOW (opto boards, “low level trigger”)')]], W.active_low ? 1 : 0) + '</select>';
      h += '<div id="roles"></div>';
    } else if (step === 1) {
      for (var i = 0; i < W.zone_count; i++) {
        var z = W.zones[i];
        var leaders = [[0, T('Própria (tem sensor/termóstato)', 'Own (has a sensor/thermostat)')]];
        for (var j = 0; j < W.zone_count; j++) if (j !== i) leaders.push([j + 1, T('Mesma divisão que ', 'Same room as ') + (j + 1) + ' · ' + (W.zones[j].name || '')]);
        h += '<div class="zone"><h4>' + T('Zona ', 'Zone ') + (i + 1) + ' · ' + T('relé ', 'relay ') + (i + 1) + '</h4>' +
          '<div class="row"><div style="flex:1;min-width:160px"><label>' + T('Nome', 'Name') + '</label><input type="text" maxlength="22" id="zn' + i + '" value="' + esc(z.name) + '"></div>' +
          '<div style="flex:1;min-width:160px"><label>' + T('Termóstato', 'Thermostat') + '</label><select class="w" id="zl' + i + '" data-re="1">' + opts(leaders, z.leader) + '</select></div></div>';
        if (!z.leader) {
          h += '<label>' + T('Sensor de temperatura da divisão', 'Room temperature sensor') + '</label><select class="w" id="zs' + i + '" data-re="1">' + opts(SRC(), z.room.type) + '</select>' + sourceFields(i, 'room', z.room) +
            '<details' + (z.floor_max > 0 ? ' open' : '') + '><summary>' + T('Limite do pavimento (opcional)', 'Floor limit (optional)') + '</summary>' +
            '<div class="row"><div><label>' + T('Máx. °C (0 = sem)', 'Max °C (0 = off)') + '</label><input type="number" step="0.5" id="zf' + i + '" value="' + (z.floor_max || 0) + '"></div>' +
            '<div style="flex:1"><label>' + T('Sonda do chão', 'Floor probe') + '</label><select class="w" id="zfs' + i + '" data-re="1">' + opts(SRC().filter(function (s) { return s[0] !== 0 || true; }), z.floor.type) + '</select></div></div>' +
            sourceFields(i, 'floor', z.floor) + '</details>';
        }
        h += '</div>';
      }
    } else if (step === 2) {
      for (var q = 0; q < W.zone_count; q++) {
        var zz = W.zones[q];
        if (zz.leader) continue;
        h += '<div class="zone"><h4>' + esc(zz.name) + '</h4><div class="row">' +
          '<div><label>' + T('Modo', 'Mode') + '</label><select id="zm' + q + '">' + opts([[1, T('Automático (horário)', 'Auto (schedule)')], [2, T('Fixo', 'Fixed')], [0, T('Desligado', 'Off')]], zz.mode) + '</select></div>' +
          '<div><label>' + T('Conforto °C', 'Comfort °C') + '</label><input type="number" step="0.5" id="zc' + q + '" value="' + zz.comfort + '"></div>' +
          '<div><label>Eco °C</label><input type="number" step="0.5" id="ze' + q + '" value="' + zz.eco + '"></div></div>' +
          '<details><summary>' + T('Horário', 'Schedule') + '</summary>' + schedEditor('s' + q + '_', zz.sched) +
          (q > 0 ? '' : '<div class="hint"><button type="button" data-copyall="1">' + T('Usar este horário em todas as zonas', 'Use this schedule for all zones') + '</button></div>') + '</details></div>';
      }
    } else if (step === 3) {
      var o = W.opt || {};
      function box(k, title, desc, extra) {
        return '<div class="opt"><label><input type="checkbox" id="o_' + k + '"' + (o[k] ? ' checked' : '') + '> ' + title + '</label><div class="hint">' + desc + '</div>' + (extra || '') + '</div>';
      }
      h += '<div class="row"><div><label>' + T('Bomba arranca após (s)', 'Pump start delay (s)') + '</label><input type="number" id="ps" value="' + W.pump_start_s + '"></div>' +
        '<div><label>' + T('Bomba para após (s)', 'Pump stop delay (s)') + '</label><input type="number" id="pe" value="' + W.pump_stop_s + '"></div>' +
        '<div><label>' + T('Sensor sem dados após (h)', 'Sensor stale after (h)') + '</label><input type="number" step="0.5" id="stale" value="' + (W.stale_min / 60) + '"></div>' +
        '<div><label>' + T('Férias: anti-gelo °C', 'Holiday frost °C') + '</label><input type="number" step="0.5" id="hol" value="' + W.holiday + '"></div></div>' +
        '<div class="hint">' + T('Shelly H&T a pilhas só envia quando a temperatura muda; se ficar estável muitas horas aumenta o limite (ou liga-o por USB: envia a cada 5 min).', 'Battery Shelly H&T only report on change; if the temperature is stable for hours raise the limit (or USB-power it: reports every 5 min).') + '</div>';
      h += box('pump_exercise', T('Anti-bloqueio da bomba', 'Pump anti-seize'), T('Segunda às 10:00, sem aquecimento há 24 h: abre uma zona e roda a bomba 1 min.', 'Mondays 10:00 with no heating for 24 h: opens a zone and runs the pump 1 min.'));
      h += box('valve_exercise', T('Anti-bloqueio dos atuadores', 'Actuator anti-seize'), T('À mesma hora abre e fecha todas as zonas.', 'At the same time opens and closes every zone.'));
      h += box('window', T('Janela aberta', 'Open window'), T('Queda de 1 °C em 10 min fecha a zona 30 min.', 'A 1 °C drop within 10 min closes the zone for 30 min.'));
      h += box('optimum_start', T('Arranque antecipado', 'Optimum start'), T('Aprende quanto cada zona demora a aquecer e começa mais cedo (até 4 h).', 'Learns how long each zone takes to heat and starts earlier (up to 4 h).'));
      h += box('pv', T('Excedente solar', 'Solar surplus'), T('Com excedente sobe a temperatura das zonas escolhidas (Home Assistant: ação set_pv_surplus, ou MQTT).', 'With surplus raises the chosen zones (Home Assistant: action set_pv_surplus, or MQTT).'),
        '<div class="row"><div><label>+°C</label><input type="number" step="0.5" id="pvd" value="' + W.pv.delta + '"></div><div style="flex:2"><label>' + T('Tópico MQTT (opcional)', 'MQTT topic (optional)') + '</label><input type="text" id="pvt" value="' + esc(W.pv.topic) + '"></div>' +
        '<div><label>' + T('Campo', 'Field') + '</label><input type="text" id="pvk" value="' + esc(W.pv.key) + '"></div><div><label>W &gt;</label><input type="number" id="pvw" value="' + W.pv.threshold_w + '"></div></div>' +
        '<div class="hint">' + T('Zonas: ', 'Zones: ') + W.zones.slice(0, W.zone_count).map(function (z, i) { return z.leader ? '' : '<label style="display:inline;font-weight:400"><input type="checkbox" id="pvz' + i + '"' + (z.pv_boost ? ' checked' : '') + '> ' + esc(z.name) + '</label>'; }).join(' ') + '</div>');
      h += box('tariff', T('Tarifa bi-horária', 'Two-rate tariff'), T('Horas de vazio: +°C; resto do dia: −°C.', 'Cheap hours: +°C; rest of the day: −°C.'),
        '<div class="row"><div><label>' + T('Vazio 1', 'Cheap 1') + '</label><input type="time" step="600" id="ts1" value="' + slotToTime(W.tariff.s1) + '"> – <input type="time" step="600" id="te1" value="' + slotToTime(W.tariff.e1) + '"></div>' +
        '<div><label>' + T('Vazio 2', 'Cheap 2') + '</label><input type="time" step="600" id="ts2" value="' + slotToTime(W.tariff.s2) + '"> – <input type="time" step="600" id="te2" value="' + slotToTime(W.tariff.e2) + '"></div>' +
        '<div><label>+°C</label><input type="number" step="0.5" id="tb" value="' + W.tariff.boost + '"></div><div><label>−°C</label><input type="number" step="0.5" id="tc" value="' + W.tariff.cut + '"></div></div>');
      h += box('altherma', T('Ligação ao ESPHome_Altherma', 'ESPHome_Altherma link'), T('Mostra a ida e o retorno da bomba de calor no diagrama (lido diretamente da outra placa).', 'Shows the heat pump flow and return in the diagram (read straight from the other board).'),
        '<div class="row"><div style="flex:1"><label>' + T('Endereço (IP ou nome)', 'Address (IP or name)') + '</label><input type="text" id="ah" value="' + esc(W.altherma.host) + '" placeholder="192.168.1.60"></div></div>' +
        '<div class="row"><div style="flex:1"><label>' + T('Ida', 'Flow') + '</label><input type="text" id="af" value="' + esc(W.altherma.flow) + '"></div><div style="flex:1"><label>' + T('Retorno', 'Return') + '</label><input type="text" id="ar" value="' + esc(W.altherma.ret) + '"></div></div>');
      h += '<div class="opt"><b>' + T('Broker MQTT (só se usares MQTT)', 'MQTT broker (only if you use MQTT)') + '</b><div class="row">' +
        '<div style="flex:2"><label>' + T('Endereço', 'Host') + '</label><input type="text" id="mh" value="' + esc(W.mqtt.host) + '" placeholder="192.168.1.10"></div>' +
        '<div><label>' + T('Porta', 'Port') + '</label><input type="number" id="mp" value="' + (W.mqtt.port || 1883) + '"></div>' +
        '<div style="flex:1"><label>' + T('Utilizador', 'User') + '</label><input type="text" id="mu" value="' + esc(W.mqtt.user) + '"></div>' +
        '<div style="flex:1"><label>' + T('Palavra-passe', 'Password') + '</label><input type="password" id="mw" value="' + esc(W.mqtt.pass) + '"></div></div></div>';
    } else {
      var used = W.roles.slice(W.zone_count).map(function (r, k) { return T('Relé ', 'Relay ') + (W.zone_count + k + 1) + ': ' + (ROLES()[r] || ROLES()[0])[1]; });
      h += '<p>' + T('Zonas: ', 'Zones: ') + '<b>' + W.zone_count + '</b> · ' + esc(W.zones.slice(0, W.zone_count).map(function (z) { return z.name; }).join(', ')) + '</p>' +
        '<p>' + esc(used.join(' · ')) + '</p>' +
        '<p class="hint">' + T('Ao gravar, a placa reinicia (cerca de 15 s) para aplicar os nomes das zonas. Se já adicionaste a placa ao Home Assistant, mudar nomes cria entidades novas.',
          'Saving restarts the device (about 15 s) to apply the zone names. If the device is already in Home Assistant, renaming creates new entities.') + '</p>';
    }
    h += '<div class="nav"><button type="button" data-n="close">' + T('Cancelar', 'Cancel') + '</button><span>' +
      (step > 0 ? '<button type="button" data-n="back">← ' + T('Anterior', 'Back') + '</button> ' : '') +
      (step < names.length - 1 ? '<button type="button" class="pri" data-n="next">' + T('Seguinte', 'Next') + ' →</button>' : '<button type="button" class="pri" data-n="save">💾 ' + T('Gravar', 'Save') + '</button>') + '</span></div>';
    var m = modal(h);
    if (step === 0) {
      var paintRoles = function () {
        var zc2 = +m.querySelector('#zc').value, r = '';
        for (var k2 = zc2; k2 < nR; k2++)
          r += '<label>' + T('Relé ', 'Relay ') + (k2 + 1) + '</label><select class="w" id="role' + k2 + '">' + opts(ROLES(), W.roles[k2] || 0) + '</select>';
        m.querySelector('#roles').innerHTML = r || '<div class="hint">' + T('Sem relés livres para bomba / pedido de calor.', 'No free relays for pump / heat demand.') + '</div>';
      };
      m.querySelector('#zc').addEventListener('change', paintRoles);
      paintRoles();
    }
    m.addEventListener('change', function (e) {
      if (e.target && e.target.getAttribute('data-re')) { collect(m); showStep(); }
    });
    m.addEventListener('click', function (e) {
      var t = e.target;
      if (!t || !t.getAttribute) return;
      var cp = t.getAttribute('data-copy');
      if (cp) {
        ['a', 'b', 'c', 'd'].forEach(function (s) { var v = m.querySelector('#' + cp + '0' + s).value; for (var k = 1; k < 5; k++) m.querySelector('#' + cp + k + s).value = v; });
        return;
      }
      if (t.getAttribute('data-copyall')) {
        collect(m);
        var src = null;
        for (var a = 0; a < W.zone_count; a++) if (!W.zones[a].leader) { src = W.zones[a].sched; break; }
        if (src) W.zones.forEach(function (z) { z.sched = JSON.parse(JSON.stringify(src)); });
        showStep();
        return;
      }
      var n = t.getAttribute('data-n');
      if (!n) return;
      if (n === 'close') { closeModal(); return; }
      collect(m);
      if (step === 3) W.zones.forEach(function (z, i) { var e2 = m.querySelector('#pvz' + i); if (e2) z.pv_boost = e2.checked; });
      if (n === 'back') { step--; showStep(); return; }
      if (n === 'next') { step++; showStep(); return; }
      if (n === 'save') save();
    });
  }

  function save() {
    W.configured = true;
    var body = JSON.parse(JSON.stringify(W));
    delete body.features; delete body.board; delete body.relays;
    postJSON('/ufh/config', body).then(function () {
      modal('<h2>' + T('Gravado', 'Saved') + '</h2><p>' + T('A aplicar… a página recarrega sozinha.', 'Applying… the page reloads by itself.') + '</p>');
      setTimeout(function () { location.reload(); }, 15000);
    }).catch(function (e) { alert(T('Erro ao gravar: ', 'Save failed: ') + e.message); });
  }

  // ---------- schedules & temperatures (quick editor) ----------
  function openSched() {
    getJSON('/ufh/config').then(function (c) {
      W = JSON.parse(JSON.stringify(c)); step = 2;
      var h = '<h2>' + T('Horários e temperaturas', 'Schedules & temperatures') + '</h2><p class="sub">' + T('Conforto nos períodos indicados, Eco no resto do tempo.', 'Comfort in the periods below, Eco the rest of the time.') + '</p>';
      var M = W.main || {mode: 1, comfort: 21, eco: 18, sched: []};
      h += '<div class="zone"><h4>🏠 ' + T('Geral', 'Main') + '</h4><div class="hint">' +
        T('As zonas com “Segue a geral” ligado usam estas temperaturas e este horário. Se escolheres Conforto ou Eco à mão no termóstato geral, vale até à próxima mudança do horário.',
          'Zones with “Follows main” on use these temperatures and this schedule. Choosing Comfort or Eco by hand on the main thermostat holds until the next schedule change.') + '</div><div class="row">' +
        '<div><label>' + T('Modo', 'Mode') + '</label><select id="mm">' + opts([[1, T('Automático (horário)', 'Auto (schedule)')], [2, T('Fixo', 'Fixed')], [0, T('Desligado', 'Off')]], M.mode) + '</select></div>' +
        '<div><label>' + T('Conforto °C', 'Comfort °C') + '</label><input type="number" step="0.5" id="mc" value="' + M.comfort + '"></div>' +
        '<div><label>Eco °C</label><input type="number" step="0.5" id="me" value="' + M.eco + '"></div></div>' +
        '<details><summary>' + T('Horário', 'Schedule') + '</summary>' + schedEditor('sm_', M.sched) + '</details></div>';
      for (var q = 0; q < W.zone_count; q++) {
        var zz = W.zones[q];
        if (zz.leader) continue;
        h += '<div class="zone"><h4>' + esc(zz.name) + '</h4>' +
          '<label class="lbl"><input type="checkbox" id="zfm' + q + '"' + (zz.follow ? ' checked' : '') + '> ' + T('Segue a geral (usa as temperaturas e o horário da geral)', 'Follows main (uses the main temperatures and schedule)') + '</label><div class="row">' +
          '<div><label>' + T('Modo', 'Mode') + '</label><select id="zm' + q + '">' + opts([[1, T('Automático (horário)', 'Auto (schedule)')], [2, T('Fixo', 'Fixed')], [0, T('Desligado', 'Off')]], zz.mode) + '</select></div>' +
          '<div><label>' + T('Conforto °C', 'Comfort °C') + '</label><input type="number" step="0.5" id="zc' + q + '" value="' + zz.comfort + '"></div>' +
          '<div><label>Eco °C</label><input type="number" step="0.5" id="ze' + q + '" value="' + zz.eco + '"></div></div>' +
          '<details><summary>' + T('Horário', 'Schedule') + '</summary>' + schedEditor('s' + q + '_', zz.sched) + '</details></div>';
      }
      h += '<div class="nav"><button type="button" data-n="close">' + T('Cancelar', 'Cancel') + '</button><button type="button" class="pri" data-n="save">💾 ' + T('Gravar', 'Save') + '</button></div>';
      var m = modal(h);
      m.addEventListener('click', function (e) {
        var t = e.target, cp = t && t.getAttribute && t.getAttribute('data-copy');
        if (cp) { ['a', 'b', 'c', 'd'].forEach(function (s) { var v = m.querySelector('#' + cp + '0' + s).value; for (var k = 1; k < 5; k++) m.querySelector('#' + cp + k + s).value = v; }); return; }
        var n = t && t.getAttribute && t.getAttribute('data-n');
        if (n === 'close') closeModal();
        if (n === 'save') {
          collect(m);
          W.zones.forEach(function (z, k) { var f = m.querySelector('#zfm' + k); if (f) z.follow = f.checked; });
          var body = {zones: W.zones.map(function (z) { return {mode: z.mode, comfort: z.comfort, eco: z.eco, sched: z.sched, follow: !!z.follow}; }),
            main: {mode: +m.querySelector('#mm').value, comfort: +m.querySelector('#mc').value, eco: +m.querySelector('#me').value, sched: readSched(m, 'sm_', M.sched)}};
          postJSON('/ufh/config', body).then(function () { closeModal(); poll(); }).catch(function (er) { alert(er.message); });
        }
      });
    });
  }

  // ---------- balance report ----------
  function openBalance() {
    if (!state) return;
    var rows = (state.zones || []).map(function (z, i) {
      return '<tr><td>' + (i + 1) + '</td><td>' + esc(z.n) + '</td><td>' + (z.duty == null ? '–' : z.duty + '%') + '</td><td>' + Math.round((z.today || 0)) + ' min</td><td>' +
        (z.lead ? '–' : (z.rate || '–') + ' °C/h') + '</td></tr>';
    }).join('');
    var m = modal('<h2>' + T('Equilíbrio das zonas', 'Zone balance') + '</h2><p class="sub">' +
      T('Percentagem do tempo a aquecer nas últimas 24 h e velocidade de aquecimento aprendida. Zonas com muito mais tempo que as outras para a mesma temperatura costumam precisar de mais caudal no coletor.',
        'Share of time heating over the last 24 h and learned heating rate. Zones needing much more time than the others for the same temperature usually need more flow at the manifold.') +
      '</p><table><tr><td>#</td><td>' + T('Zona', 'Zone') + '</td><td>24 h</td><td>' + T('Hoje', 'Today') + '</td><td>' + T('Aquece', 'Heats') + '</td></tr>' + rows + '</table>' +
      '<div class="nav"><span></span><button type="button" class="pri" data-n="close">OK</button></div>');
    m.addEventListener('click', function (e) { if (e.target.getAttribute && e.target.getAttribute('data-n')) closeModal(); });
  }

  // ---------- zone quick view (click on a zone) ----------
  function openZone(i) {
    var z = state && state.zones[i];
    if (!z) return;
    var h = '<h2>' + esc(z.n) + '</h2><p class="sub">' + T('Zona ', 'Zone ') + (i + 1) + ' · ' + (z.r ? T('a aquecer', 'heating') : T('fechada', 'closed')) + '</p>' +
      '<p>' + T('Temperatura: ', 'Temperature: ') + '<b>' + fmt(z.t) + ' °C</b>' + (z.h != null ? ' · ' + fmt(z.h, 0) + '%' : '') + (z.age >= 0 ? ' · ' + T('há ', '') + z.age + ' min' + T('', ' ago') : '') +
      (z.f != null ? ' · ' + T('chão ', 'floor ') + fmt(z.f) + ' °C' : '') + (z.b >= 0 ? ' · 🔋' + z.b + '%' : '') + '</p>' +
      '<p>' + T('Alvo: ', 'Target: ') + '<b>' + fmt(z.tg) + ' °C</b>' + (z.ovr ? ' (' + T('ajuste manual até à próxima mudança do horário', 'manual change until the next schedule change') + ')' : '') + (z.early ? ' · ' + T('arranque antecipado', 'optimum start') : '') + '</p>';
    if (!z.lead && z.fm) {
      h += '<div class="hint">🏠 ' + T('Esta zona segue a geral: a temperatura muda-se no termóstato geral.', 'This zone follows the main thermostat: change the temperature there.') + '</div>' +
        '<div class="row"><div><label>' + T('Modo', 'Mode') + '</label><select id="qm">' + opts([[1, T('Automático', 'Auto')], [2, T('Fixo', 'Fixed')], [0, T('Desligado', 'Off')]], z.m) + '</select></div></div>';
    } else if (!z.lead) {
      h += '<div class="row"><div><label>' + T('Mudar alvo agora °C', 'Change target now °C') + '</label><input type="number" step="0.5" id="qt" value="' + (z.tg == null ? 21 : z.tg) + '"></div>' +
        '<div><label>' + T('Modo', 'Mode') + '</label><select id="qm">' + opts([[1, T('Automático', 'Auto')], [2, T('Fixo', 'Fixed')], [0, T('Desligado', 'Off')]], z.m) + '</select></div></div>' +
        '<div class="hint">' + T('Em Automático, o alvo novo vale até à próxima mudança do horário.', 'In Auto the new target holds until the next schedule change.') + '</div>';
    }
    h += '<div class="nav"><button type="button" data-n="close">' + T('Fechar', 'Close') + '</button>' + (z.lead ? '' : '<button type="button" class="pri" data-n="apply">' + T('Aplicar', 'Apply') + '</button>') + '</div>';
    var m = modal(h);
    m.addEventListener('click', function (e) {
      var n = e.target.getAttribute && e.target.getAttribute('data-n');
      if (n === 'close') closeModal();
      if (n === 'apply') {
        var zones = [];
        for (var k = 0; k <= i; k++) zones.push({});
        var mode = +m.querySelector('#qm').value, z2 = {};
        var qt = m.querySelector('#qt');
        if (mode !== 0 && qt) z2.target = +qt.value;
        if (mode !== z.m) z2.mode = mode;
        zones[i] = z2;
        postJSON('/ufh/config', {zones: zones}).then(function () { closeModal(); setTimeout(poll, 1500); }).catch(function (er) { alert(er.message); });
      }
    });
  }

  // ---------- boot ----------
  function mount() {
    var sr = root();
    if (!sr) return false;
    if (!sr.getElementById('ufh-css')) {
      var st = document.createElement('style'); st.id = 'ufh-css'; st.textContent = CSS; sr.appendChild(st);
    }
    if (!document.getElementById('ufhm-css')) {
      var st2 = document.createElement('style'); st2.id = 'ufhm-css'; st2.textContent = MCSS; document.head.appendChild(st2);
    }
    if (!panel || !sr.contains(panel)) {
      panel = document.createElement('div');
      panel.className = 'ufh';
      panel.id = 'ufh-panel';
      panel.innerHTML = '<h3>🌡️ ' + T('Piso radiante', 'Underfloor heating') + '</h3><p>' + T('A ligar…', 'Connecting…') + '</p>';
      var main = sr.querySelector('esp-entity-table') || sr.firstChild;
      if (main && main.parentNode) main.parentNode.insertBefore(panel, main); else sr.appendChild(panel);
      panel.addEventListener('click', function (e) {
        var a = e.target.closest ? e.target.closest('[data-a]') : null;
        if (a) {
          var w = a.getAttribute('data-a');
          if (w === 'wizard') openWizard();
          if (w === 'sched') openSched();
          if (w === 'balance') openBalance();
          return;
        }
        var zg = e.target.closest ? e.target.closest('.z') : null;
        if (zg) openZone(+zg.getAttribute('data-z'));
      });
    }
    return true;
  }
  function boot() {
    if (!mount()) return false;
    poll();
    setInterval(function () { if (!document.hidden) { mount(); poll(); } }, 3000);
    window.addEventListener('ufh-lang', renderPanel);
    return true;
  }
  if (!boot()) { var n = 0, t = setInterval(function () { if (boot() || ++n > 80) clearInterval(t); }, 250); }
})();
