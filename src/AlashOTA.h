// AlashOTA: обновление прошивки ESP32 / ESP8266 из браузера по Wi-Fi: откройте страницу, выберите .bin, готово.
// Пароль обязателен, формат и плата проверяются до записи. Без зависимостей (встроенные WebServer и Update). MIT License. Alash Electronics.
#ifndef ALASH_OTA_H
#define ALASH_OTA_H

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
typedef WebServer AlashOtaServer;
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Updater.h>
typedef ESP8266WebServer AlashOtaServer;
#endif

class AlashOTA {
 public:
  typedef void (*Callback)();
  typedef void (*ProgressCallback)(uint32_t bytesWritten);
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_PASSWORD,     // begin() без пароля: обновление без пароля мог бы запустить кто угодно в сети
    ERR_NOT_CONNECTED,   // нет Wi-Fi: адрес страницы неизвестен
    ERR_AUTH,            // неверный пароль при загрузке
    ERR_BAD_FILE,        // файл не .bin
    ERR_BAD_FIRMWARE,    // файл не похож на прошивку ESP
    ERR_WRONG_CHIP,      // прошивка собрана для другого чипа (например ESP32-S3 вместо ESP32)
    ERR_NO_SPACE,        // прошивка не помещается в память
    ERR_WRITE,           // ошибка записи во флеш
    ERR_FINISH           // файл повреждён или оборван (проверка не прошла)
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  explicit AlashOTA(uint16_t port = 80);          // свой веб-сервер на порту (80 по умолчанию)
  explicit AlashOTA(AlashOtaServer &server);      // добавить страницу обновления в уже существующий WebServer
  ~AlashOTA();

  // ---- Запуск --------------------------------------------------------------
  bool begin(const char *password, const char *path = "/update");   // логин admin; вызывать после подключения к Wi-Fi
  void update();                                  // вызывайте в loop()
  String url() const;                             // адрес страницы, например http://192.168.1.50/update

  // ---- Настройка ---------------------------------------------------------------
  void setInfo(const char *text) { strncpy(_info, text, sizeof(_info) - 1); _info[sizeof(_info) - 1] = 0; }   // название и версия на странице
  void setAutoReboot(bool on) { _reboot = on; }   // перезагрузка после успешного обновления (по умолчанию да)
  void onStart(Callback cb) { _onStart = cb; }
  void onProgress(ProgressCallback cb) { _onProgress = cb; }   // сколько байт уже записано
  void onEnd(Callback cb) { _onEnd = cb; }        // успешно записано, сразу перед перезагрузкой
  void onError(Callback cb) { _onError = cb; }
  void setLanguage(Language l) { _lang = l; }

  // ---- Состояние и диагностика --------------------------------------------------------
  bool updating() const { return _busy; }
  Error error() const { return _err; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);

 private:
  AlashOtaServer *_srv;
  bool _own;
  char _pass[33] = "";
  uint16_t _port = 80;
  char _path[24] = "/update";
  char _info[40] = "";
  bool _reboot = true, _busy = false, _deny = false, _started = false, _failed = false, _first = true, _rebootPending = false;
  uint32_t _written = 0, _rebootAt = 0;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  Callback _onStart = nullptr, _onEnd = nullptr, _onError = nullptr;
  ProgressCallback _onProgress = nullptr;

  void handlePage();
  void handleDone();
  void handleUpload();
  void fail(Error e);
  bool authorized();
};

#endif  // ALASH_OTA_H
