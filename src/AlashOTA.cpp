// MIT License. Alash Electronics.
#include "AlashOTA.h"
#if defined(ESP32) || defined(ESP8266)

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

static const char PAGE_HEAD[] PROGMEM = R"HTML(<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1"><title>Alash OTA</title>
<style>body{font-family:system-ui,sans-serif;max-width:30rem;margin:2rem auto;padding:0 1rem;color:#222}h1{font-size:1.4rem}button{padding:.6rem 1.2rem;font-size:1rem}progress{width:100%;height:1.2rem;margin:1rem 0}#m{white-space:pre-wrap}.e{color:#b00020}.o{color:#0a7d2c}</style></head><body>
<h1 id=t>Alash OTA</h1><p id=i>)HTML";

static const char PAGE_TAIL[] PROGMEM = R"HTML(</p><p><input type=file id=f accept=".bin"></p><p><button id=b></button></p><progress id=p value=0 max=100 hidden></progress><p id=m></p>
<script>
var L={ru:["Обновление прошивки","Загрузить","Выберите файл .bin","Загрузка: ","Нет связи с платой"],
kk:["Прошивканы жаңарту","Жүктеу",".bin файлын таңдаңыз","Жүктелуде: ","Тақтамен байланыс жоқ"],
en:["Firmware update","Upload","Choose a .bin file","Uploading: ","No connection to the board"]};
var l=L[(navigator.language||"ru").slice(0,2)]||L.ru,$=function(x){return document.getElementById(x)};
$("t").textContent=l[0];$("b").textContent=l[1];
$("b").onclick=function(){var f=$("f").files[0],m=$("m");m.className="";
if(!f){m.textContent=l[2];m.className="e";return}
var d=new FormData();d.append("firmware",f,f.name);var x=new XMLHttpRequest();
$("p").hidden=false;$("b").disabled=true;
x.upload.onprogress=function(e){if(e.lengthComputable){$("p").value=e.loaded*100/e.total;m.textContent=l[3]+Math.round(e.loaded*100/e.total)+"%"}};
x.onload=function(){$("b").disabled=false;m.textContent=x.responseText;m.className=x.status==200?"o":"e"};
x.onerror=function(){$("b").disabled=false;m.textContent=l[4];m.className="e"};
x.open("POST",location.pathname);x.send(d)};
</script></body></html>)HTML";

// ESP32 принимает прошивку неизвестного размера, ESP8266 нужен размер: даём всё свободное место
#if defined(ESP8266)
#define ALASH_UPDATE_SIZE ((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)
static void updateAbort() { Update.end(false); }
#else
#define ALASH_UPDATE_SIZE UPDATE_SIZE_UNKNOWN
static void updateAbort() { Update.abort(); }
#endif

AlashOTA::AlashOTA(uint16_t port) : _srv(new AlashOtaServer(port)), _own(true), _port(port) {}
AlashOTA::AlashOTA(AlashOtaServer &server) : _srv(&server), _own(false) {}
AlashOTA::~AlashOTA() { if (_own) delete _srv; }

// Номер чипа, для которого собрана эта прошивка; в заголовке файла он записан по смещению 12 (только ESP32)
static int ownChipId() {
#if defined(ESP32)
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  return 9;
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
  return 2;
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  return 5;
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
  return 13;
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
  return 12;
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
  return 16;
#elif defined(CONFIG_IDF_TARGET_ESP32)
  return 0;
#endif
#endif
  return -1;
}

bool AlashOTA::begin(const char *password, const char *path) {
  if (!password || !*password || strlen(password) > 32) { _err = ERR_NO_PASSWORD; return false; }
  strncpy(_pass, password, sizeof(_pass) - 1);
  strncpy(_path, path, sizeof(_path) - 1);
  _err = ERR_OK;
  _srv->on(_path, HTTP_GET, [this]() { handlePage(); });
  _srv->on(_path, HTTP_POST, [this]() { handleDone(); }, [this]() { handleUpload(); });
  if (_own) _srv->begin();
  _started = true;
  return true;
}

void AlashOTA::update() {
  if (_own) _srv->handleClient();
  if (_rebootPending && (int32_t)(millis() - _rebootAt) >= 0) {
    _rebootPending = false;
    ESP.restart();
  }
}

String AlashOTA::url() const {
  if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) return String();
  String u = "http://";
  u += (WiFi.status() == WL_CONNECTED ? WiFi.localIP() : WiFi.softAPIP()).toString();
  if (_port != 80) { u += ':'; u += String((int)_port); }
  u += _path;
  return u;
}

bool AlashOTA::authorized() { return _srv->authenticate("admin", _pass); }

void AlashOTA::handlePage() {
  if (!authorized()) { _srv->requestAuthentication(); return; }
  String page;
  page.reserve(sizeof(PAGE_HEAD) + sizeof(PAGE_TAIL) + 80);
  page += FPSTR(PAGE_HEAD);
  for (const char *p = _info; *p; p++) {         // название прошивки без разметки
    if (*p == '<') page += F("&lt;");
    else if (*p == '>') page += F("&gt;");
    else if (*p == '&') page += F("&amp;");
    else page += *p;
  }
  page += FPSTR(PAGE_TAIL);
  _srv->send(200, "text/html; charset=utf-8", page);
}

void AlashOTA::fail(Error e) {
  if (_busy) updateAbort();
  _busy = false;
  _failed = true;
  _err = e;
  if (_onError) _onError();
}

void AlashOTA::handleUpload() {
  HTTPUpload &up = _srv->upload();
  if (up.status == UPLOAD_FILE_START) {
    _err = ERR_OK; _failed = false; _deny = false; _first = true; _written = 0;
    if (!authorized()) { _deny = true; _failed = true; _err = ERR_AUTH; return; }
    String fn = up.filename;
    fn.toLowerCase();
    if (!fn.endsWith(".bin")) { fail(ERR_BAD_FILE); return; }
    if (!Update.begin(ALASH_UPDATE_SIZE)) { fail(ERR_NO_SPACE); return; }
    _busy = true;
    if (_onStart) _onStart();
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (_failed || !_busy) return;
    if (_first) {
      _first = false;
      if (up.currentSize == 0 || up.buf[0] != 0xE9) { fail(ERR_BAD_FIRMWARE); return; }
      int chip = ownChipId();
      if (chip >= 0 && up.currentSize >= 14 && (up.buf[12] | (up.buf[13] << 8)) != chip) { fail(ERR_WRONG_CHIP); return; }
    }
    if (Update.write(up.buf, up.currentSize) != up.currentSize) { fail(ERR_WRITE); return; }
    _written += up.currentSize;
    if (_onProgress) _onProgress(_written);
  } else if (up.status == UPLOAD_FILE_END) {
    if (_failed || !_busy) return;
    if (_written == 0) { fail(ERR_BAD_FIRMWARE); return; }       // пустой файл
    if (!Update.end(true)) { fail(ERR_FINISH); return; }
    _busy = false;
    if (_onEnd) _onEnd();
  } else if (up.status == UPLOAD_FILE_ABORTED) {
    if (_busy) { updateAbort(); _busy = false; }
    _failed = true;
  }
}

void AlashOTA::handleDone() {
  if (_deny) { _deny = false; _srv->requestAuthentication(); return; }
  if (_failed || _err != ERR_OK) {
    String msg = errorText();
    _srv->send(500, "text/plain; charset=utf-8", msg);
    return;
  }
  _srv->send(200, "text/plain; charset=utf-8", String(T("Готово: прошивка записана.", "Дайын: прошивка жазылды.", "Done: the firmware is written.")) +
                 (_reboot ? String(T(" Плата перезагружается.", " Тақта қайта қосылады.", " The board is restarting.")) : String()));
  if (_reboot) { _rebootPending = true; _rebootAt = millis() + 800; }   // дать ответу дойти до браузера
}

const __FlashStringHelper *AlashOTA::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_PASSWORD:
      return T("Нужен пароль (1-32 символа): без него обновить плату мог бы любой в сети. Логин admin", "Құпиясөз керек (1-32 таңба): онсыз желідегі кез келген адам жаңарта алады. Логин admin",
               "A password is required (1-32 characters): otherwise anyone on the network could update the board. Login is admin");
    case ERR_NOT_CONNECTED:
      return T("Нет Wi-Fi: сначала подключитесь к сети (или создайте точку доступа), потом вызовите begin()", "Wi-Fi жоқ: алдымен желіге қосылыңыз, содан кейін begin() шақырыңыз",
               "No Wi-Fi: connect to a network (or start an access point) first, then call begin()");
    case ERR_AUTH:
      return T("Неверный пароль", "Құпиясөз қате", "Wrong password");
    case ERR_BAD_FILE:
      return T("Нужен файл .bin. В Arduino IDE: Скетч, Экспорт скомпилированного бинарного файла", ".bin файлы керек. Arduino IDE: Sketch, Export Compiled Binary",
               "A .bin file is needed. In Arduino IDE: Sketch, Export Compiled Binary");
    case ERR_BAD_FIRMWARE:
      return T("Файл не похож на прошивку ESP. Выберите файл без bootloader и partitions (обычно с именем скетча .ino.bin)", "Файл ESP прошивкасына ұқсамайды. bootloader және partitions жоқ файлды таңдаңыз (.ino.bin)",
               "The file does not look like ESP firmware. Choose the file without bootloader and partitions (usually sketch.ino.bin)");
    case ERR_WRONG_CHIP:
      return T("Прошивка собрана для другого чипа (например ESP32-S3 вместо ESP32). Выберите правильную плату в Arduino IDE и соберите заново", "Прошивка басқа чипке жиналған (мысалы ESP32 орнына ESP32-S3). Arduino IDE-де дұрыс тақтаны таңдап қайта жинаңыз",
               "The firmware was built for another chip (for example ESP32-S3 instead of ESP32). Pick the right board in Arduino IDE and build again");
    case ERR_NO_SPACE:
      return T("Прошивка не помещается в память: выберите схему разделов с двумя приложениями (OTA)", "Прошивка жадқа сыймайды: екі қолданбасы бар бөлім схемасын (OTA) таңдаңыз",
               "The firmware does not fit: choose a partition scheme with two apps (OTA)");
    case ERR_WRITE:
      return T("Ошибка записи во флеш. Повторите загрузку; плата продолжит работать со старой прошивкой", "Флешке жазу қатесі. Қайталаңыз; тақта ескі прошивкамен жұмыс істей береді",
               "Flash write error. Try again; the board keeps running the old firmware");
    case ERR_FINISH:
      return T("Файл повреждён или оборван при загрузке. Повторите; плата продолжит работать со старой прошивкой", "Файл бүлінген не жүктеу үзілген. Қайталаңыз; тақта ескі прошивкамен жұмыс істей береді",
               "The file is damaged or the upload was cut. Try again; the board keeps running the old firmware");
  }
  return F("");
}

void AlashOTA::diagnose(Print &out) {
  out.println(T("AlashOTA: диагностика", "AlashOTA: диагностика", "AlashOTA: diagnostics"));
  out.print(T("Wi-Fi: ", "Wi-Fi: ", "Wi-Fi: "));
  out.println(WiFi.status() == WL_CONNECTED ? T("подключено", "қосылған", "connected") : T("нет подключения", "қосылмаған", "not connected"));
  String u = url();
  out.print(T("Страница: ", "Бет: ", "Page: "));
  out.println(u.length() ? u.c_str() : "-");
  out.print(T("Свободно под прошивку, байт: ", "Прошивкаға бос орын, байт: ", "Free for firmware, bytes: "));
  out.println((int)ESP.getFreeSketchSpace());
  out.println(_started ? T("Сервер обновления запущен. Логин admin.", "Жаңарту сервері іске қосылған. Логин admin.", "The update server is running. Login is admin.")
                       : T("begin() не вызван или завершился ошибкой.", "begin() шақырылмаған не қатемен аяқталды.", "begin() was not called or failed."));
  if (!_started || _err != ERR_OK) out.println(errorText());
  out.println(T("Если страница не открывается: телефон / компьютер должны быть в той же сети Wi-Fi, адрес без https.", "Бет ашылмаса: телефон / компьютер сол Wi-Fi желісінде болуы керек, мекенжай https-сіз.",
                "If the page does not open: the phone / computer must be on the same Wi-Fi, use http (not https)."));
}

#endif
