// ESPHome_UFH web_server v3 add-ons (the only js_include allowed):
//  1) project favicon (heating loop) via data: URI;
//  2) cap the debug log panel height;
//  3) PT/EN language toggle + entity/group/option name translation (MutationObserver).
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
  var ENT_PT = {"Zone 1": "Zona 1", "Zone 2": "Zona 2", "Zone 3": "Zona 3", "Zone 4": "Zona 4", "Zone 5": "Zona 5", "Zone 6": "Zona 6", "Zone 7": "Zona 7", "Zone 8": "Zona 8", "Zone 9": "Zona 9", "Zone 10": "Zona 10", "Zone 11": "Zona 11", "Zone 12": "Zona 12", "Zone 13": "Zona 13", "Zone 14": "Zona 14", "All Zones Off": "Desligar todas as zonas", "Active Zones": "Zonas ativas", "None": "Nenhuma", "Pump Mode": "Modo da bomba", "Auto": "Automático", "On": "Ligada", "Off": "Desligada", "Pump": "Bomba", "Heat Demand": "Pedido de calor", "Heat Pump Relay": "Relé da bomba de calor", "Heat Pump Control": "Controlo da bomba de calor", "Pump Start Delay": "Atraso a ligar a bomba", "Pump Stop Delay": "Atraso a desligar a bomba", "Status LED": "LED de estado", "Firmware Update": "Atualização do firmware", "Auto Update": "Atualização automática", "Update Status": "Estado da atualização", "Check for Update": "Verificar atualizações", "Install Update": "Instalar atualização", "Latest Version": "Última versão", "Update Available": "Atualização disponível", "Firmware Version": "Versão do firmware", "ESPHome Version": "Versão do ESPHome", "Board": "Placa", "Uptime": "Tempo ligado", "Reset Reason": "Motivo do reinício", "IP Address": "Endereço IP", "MAC Address": "Endereço MAC", "Restart": "Reiniciar", "Restart (Safe Mode)": "Reiniciar (modo seguro)"};
  var GROUP_PT = {"Zones": "Zonas", "Pump": "Bomba", "Settings": "Definições", "Maintenance": "Manutenção", "Configuration": "Configuração", "Diagnostic": "Diagnóstico", "Undefined": "Outros"};
  var KEY = 'esphome_ufh_lang', lang;
  try { lang = localStorage.getItem(KEY); } catch (e) {}
  if (lang !== 'pt' && lang !== 'en') lang = /^pt/i.test(navigator.language || '') ? 'pt' : 'en';
  window.__ufhLang = {
    get: function () { return lang; },
    set: function (l) {
      lang = l === 'pt' ? 'pt' : 'en';
      try { localStorage.setItem(KEY, lang); } catch (e) {}
      if (window.__ufhOnLang) window.__ufhOnLang(lang);
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
          var en = tn.__en, pt = ENT_PT[en];
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
