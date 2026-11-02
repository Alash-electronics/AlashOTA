// Тесты AlashOTA без железа (имитация WebServer с загрузкой файла, Update и ESP; плата ESP32):
//   g++ -std=c++17 -Wall -DESP32 -DCONFIG_IDF_TARGET_ESP32 -I mock -I ../../src test_ota.cpp ../../src/AlashOTA.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "WiFi.h"
#include "WebServer.h"
#include "Update.h"
#include "AlashOTA.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static bool has(const std::string &h, const char *n) { return h.find(n) != std::string::npos; }

static int starts, ends, errors; static uint32_t lastProgress; static int progressCalls; static bool busyInStart;
static AlashOTA *gOta;
static void onStart() { starts++; busyInStart = gOta->updating(); }
static void onEnd() { ends++; }
static void onErr() { errors++; }
static void onProg(uint32_t b) { lastProgress = b; progressCalls++; }

static void resetAll() {
  if (gOta) { g_ms += 5000; gOta->update(); }          // дать завершиться отложенной перезагрузке прошлого шага
  g_httpQueue.clear(); g_httpResp.clear(); Update = UpdateClass(); ESP = EspClass(); WiFi = WiFiClass(); g_ms = 100000;
  starts = ends = errors = progressCalls = 0; lastProgress = 0; busyInStart = false;
}
// Прошивка ESP32: байт 0xE9, номер чипа по смещению 12, дальше случайные данные
static std::vector<uint8_t> firmware(size_t n, int chip = 0, uint8_t magic = 0xE9) {
  std::vector<uint8_t> d(n);
  uint32_t x = 12345;
  for (auto &b : d) { x = x * 1664525u + 1013904223u; b = x >> 24; }
  d[0] = magic; d[12] = chip & 0xFF; d[13] = chip >> 8;
  return d;
}
static HttpResp post(AlashOTA &o, const std::vector<uint8_t> &data, const char *name = "sketch.ino.bin", const char *auth = "admin:secret", size_t chunk = 1000) {
  HttpReq r; r.method = HTTP_POST; r.path = "/update"; r.auth = auth; r.hasUpload = true; r.filename = name; r.data = data; r.chunk = chunk;
  size_t before = g_httpResp.size();
  g_httpQueue.push_back(r);
  o.update();
  return g_httpResp.size() > before ? g_httpResp.back() : HttpResp{0, "", ""};
}
static HttpResp get(AlashOTA &o, const char *auth) {
  HttpReq r; r.method = HTTP_GET; r.path = "/update"; r.auth = auth;
  size_t before = g_httpResp.size();
  g_httpQueue.push_back(r);
  o.update();
  return g_httpResp.size() > before ? g_httpResp.back() : HttpResp{0, "", ""};
}

int main() {
  // 1) пароль обязателен
  resetAll();
  AlashOTA o;
  gOta = &o;
  CHECK(!o.begin("") && o.error() == AlashOTA::ERR_NO_PASSWORD);
  CHECK(!o.begin(nullptr) && o.error() == AlashOTA::ERR_NO_PASSWORD);
  CHECK(!o.begin("0123456789012345678901234567890123") && o.error() == AlashOTA::ERR_NO_PASSWORD);
  CHECK(o.begin("secret") && o.error() == AlashOTA::ERR_OK);
  o.onStart(onStart); o.onEnd(onEnd); o.onError(onErr); o.onProgress(onProg);

  // 2) страница: только с паролем, название прошивки без разметки
  o.setInfo("Датчик <b>v1.2</b> & Ко");
  CHECK(get(o, "").code == 401);
  CHECK(get(o, "admin:wrong").code == 401);
  CHECK(get(o, "root:secret").code == 401);
  HttpResp pg = get(o, "admin:secret");
  CHECK(pg.code == 200 && has(pg.type, "text/html") && has(pg.body, "type=file") && has(pg.body, "Датчик &lt;b&gt;v1.2&lt;/b&gt; &amp; Ко"));
  CHECK(!has(pg.body, "<b>v1.2"));
  CHECK(has(pg.body, "Alash OTA") && has(pg.body, "XMLHttpRequest"));

  // 3) успешное обновление: данные записаны точно, обратные вызовы, перезагрузка не сразу
  std::vector<uint8_t> fw = firmware(5000);
  HttpResp ok = post(o, fw);
  CHECK(ok.code == 200 && has(ok.body, "Готово"));
  CHECK(Update.data == fw && Update.ends == 1 && Update.aborts == 0);
  CHECK(starts == 1 && ends == 1 && errors == 0 && lastProgress == 5000 && progressCalls == 5);
  CHECK(busyInStart && !o.updating() && o.error() == AlashOTA::ERR_OK);
  CHECK(ESP.restarts == 0);
  g_ms += 700; o.update(); CHECK(ESP.restarts == 0);
  g_ms += 200; o.update(); CHECK(ESP.restarts == 1);
  g_ms += 5000; o.update(); CHECK(ESP.restarts == 1);                      // только один раз

  // 4) без автоперезагрузки
  resetAll(); o.setAutoReboot(false);
  CHECK(post(o, firmware(2500)).code == 200); g_ms += 5000; o.update(); CHECK(ESP.restarts == 0);
  o.setAutoReboot(true);

  // 5) неверный пароль: ничего не записано, Update даже не начат
  resetAll();
  HttpResp bad = post(o, firmware(3000), "sketch.ino.bin", "admin:nope");
  CHECK(bad.code == 401 && Update.data.empty() && !Update.active && Update.aborts == 0 && Update.ends == 0 && starts == 0);
  bad = post(o, firmware(3000), "sketch.ino.bin", "");
  CHECK(bad.code == 401 && Update.data.empty() && ESP.restarts == 0);
  g_ms += 5000; o.update(); CHECK(ESP.restarts == 0);

  // 6) не .bin; регистр расширения не важен
  resetAll();
  HttpResp r = post(o, firmware(3000), "photo.jpg");
  CHECK(r.code == 500 && has(r.body, ".bin") && Update.data.empty() && o.error() == AlashOTA::ERR_BAD_FILE && errors == 1);
  r = post(o, firmware(3000), "NOEXT");
  CHECK(r.code == 500 && Update.data.empty());
  r = post(o, firmware(3000), "SKETCH.INO.BIN"); CHECK(r.code == 200 && o.error() == AlashOTA::ERR_OK);   // после ошибки следующая загрузка работает

  // 7) не прошивка ESP (первый байт не 0xE9): прервано до записи, старая прошивка работает
  resetAll();
  r = post(o, firmware(3000, 0, 0x7F));
  CHECK(r.code == 500 && o.error() == AlashOTA::ERR_BAD_FIRMWARE && Update.data.empty() && Update.aborts == 1 && Update.ends == 0 && ESP.restarts == 0);
  g_ms += 5000; o.update(); CHECK(ESP.restarts == 0);
  CHECK(!o.updating());
  // пустой файл
  resetAll(); r = post(o, std::vector<uint8_t>()); CHECK(r.code == 500 && o.error() == AlashOTA::ERR_BAD_FIRMWARE && Update.ends == 0);

  // 8) прошивка для другого чипа (S3 = 9, C3 = 5, S2 = 2) отклоняется, ESP32 (0) принимается
  for (int chip : {2, 5, 9, 12, 13, 16}) {
    resetAll(); r = post(o, firmware(3000, chip));
    CHECK(r.code == 500 && o.error() == AlashOTA::ERR_WRONG_CHIP && Update.data.empty());
  }
  resetAll(); r = post(o, firmware(3000, 0)); CHECK(r.code == 200);
  // маленький файл меньше заголовка (меньше 14 байт): проверка чипа пропускается, но формат проверен
  resetAll(); r = post(o, firmware(10)); CHECK(r.code == 200 || r.code == 500);

  // 9) нет места
  resetAll(); Update.capacity = 0;
  r = post(o, firmware(3000)); CHECK(r.code == 500 && o.error() == AlashOTA::ERR_NO_SPACE && has(r.body, "OTA"));
  resetAll(); Update.capacity = 2000;
  r = post(o, firmware(5000)); CHECK(r.code == 500 && o.error() == AlashOTA::ERR_WRITE && Update.aborts == 1 && ESP.restarts == 0);

  // 10) сбой записи посередине, повреждённый файл в конце
  resetAll(); Update.failWriteAfter = 2500;
  r = post(o, firmware(6000)); CHECK(r.code == 500 && o.error() == AlashOTA::ERR_WRITE && Update.aborts == 1 && Update.data.empty() && errors == 1 && ends == 0);
  resetAll(); Update.corruptEnd = true;
  r = post(o, firmware(6000)); CHECK(r.code == 500 && o.error() == AlashOTA::ERR_FINISH && ESP.restarts == 0 && ends == 0 && errors == 1);
  g_ms += 5000; o.update(); CHECK(ESP.restarts == 0);

  // 11) обрыв загрузки: ничего не применяется, следующая загрузка работает с нуля
  resetAll();
  HttpReq ab; ab.method = HTTP_POST; ab.path = "/update"; ab.auth = "admin:secret"; ab.hasUpload = true; ab.filename = "x.bin"; ab.data = firmware(8000); ab.abortAt = true; ab.abortAfter = 3000;
  g_httpQueue.push_back(ab); o.update();
  CHECK(Update.aborts == 1 && Update.ends == 0 && !o.updating() && ESP.restarts == 0);
  std::vector<uint8_t> fw2 = firmware(4000);
  r = post(o, fw2); CHECK(r.code == 200 && Update.data == fw2 && ESP.restarts == 0);
  g_ms += 1000; o.update(); CHECK(ESP.restarts == 1);

  // 12) разные размеры кусков не влияют на результат
  for (size_t chunk : {1u, 7u, 1000u, 1436u}) {
    resetAll(); std::vector<uint8_t> f = firmware(3000);
    r = post(o, f, "a.bin", "admin:secret", chunk); CHECK(r.code == 200 && Update.data == f);
  }

  // 13) адрес страницы
  resetAll(); WiFi.st = WL_DISCONNECTED;
  CHECK(o.url().length() == 0);
  WiFi.st = WL_CONNECTED;
  CHECK(strcmp(o.url().c_str(), "http://192.168.1.50/update") == 0);
  WiFi.st = WL_DISCONNECTED; WiFi.md = WIFI_AP;
  CHECK(strcmp(o.url().c_str(), "http://192.168.4.1/update") == 0);
  { AlashOTA p(8080); WiFi.st = WL_CONNECTED; WiFi.md = WIFI_STA; p.begin("pw", "/fw"); CHECK(strcmp(p.url().c_str(), "http://192.168.1.50:8080/fw") == 0); }

  // 14) добавление в готовый сервер: маршруты регистрируются, сервер сам не запускается и update() его не обслуживает
  resetAll(); WebServer shared(80);
  { AlashOTA q(shared); CHECK(q.begin("pw")); CHECK(!shared.started);
    HttpReq g; g.method = HTTP_GET; g.path = "/update"; g.auth = "admin:pw"; g_httpQueue.push_back(g);
    q.update(); CHECK(g_httpResp.empty());                                  // update() сервер не трогает
    shared.handleClient(); CHECK(g_httpResp.size() == 1 && g_httpResp[0].code == 200);
    HttpReq u; u.method = HTTP_POST; u.path = "/update"; u.auth = "admin:pw"; u.hasUpload = true; u.filename = "a.bin"; u.data = firmware(2000); g_httpQueue.push_back(u);
    shared.handleClient(); CHECK(g_httpResp.back().code == 200 && Update.data.size() == 2000);
    g_ms += 1000; q.update(); CHECK(ESP.restarts == 1); }
  CHECK(o.updating() == false);

  // 15) тексты ошибок на трёх языках, все разные
  const AlashOTA::Language langs[3] = {AlashOTA::LANG_RU, AlashOTA::LANG_KK, AlashOTA::LANG_EN};
  for (int e = 1; e <= 9; e++) {
    const char *tx[3];
    for (int l = 0; l < 3; l++) {
      resetAll(); AlashOTA t; t.setLanguage(langs[l]);
      switch (e) {
        case 1: t.begin(""); break;
        case 2: break;
        case 3: t.begin("pw"); post(t, firmware(100), "a.bin", "admin:x"); break;
        case 4: t.begin("pw"); post(t, firmware(100), "a.jpg", "admin:pw"); break;
        case 5: t.begin("pw"); post(t, firmware(100, 0, 1), "a.bin", "admin:pw"); break;
        case 6: t.begin("pw"); post(t, firmware(100, 9), "a.bin", "admin:pw"); break;
        case 7: t.begin("pw"); Update.capacity = 0; post(t, firmware(100), "a.bin", "admin:pw"); break;
        case 8: t.begin("pw"); Update.failWriteAfter = 10; post(t, firmware(100), "a.bin", "admin:pw"); break;
        case 9: t.begin("pw"); Update.corruptEnd = true; post(t, firmware(100), "a.bin", "admin:pw"); break;
      }
      tx[l] = (const char *)t.errorText();
    }
    if (e == 2 || e == 3) continue;   // ERR_NOT_CONNECTED не выставляется автоматически, ERR_AUTH короткий
    CHECK(strlen(tx[0]) > 10 && strcmp(tx[0], tx[1]) != 0 && strcmp(tx[1], tx[2]) != 0);
  }

  // 16) diagnose
  resetAll(); WiFi.st = WL_CONNECTED;
  AlashOTA d; d.begin("pw"); d.setLanguage(AlashOTA::LANG_EN);
  Print out; out.echo = true;
  printf("--- diagnose ---\n"); d.diagnose(out); printf("--- end ---\n");
  CHECK(has(out.log, "http://192.168.1.50/update") && has(out.log, "1310720"));

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
