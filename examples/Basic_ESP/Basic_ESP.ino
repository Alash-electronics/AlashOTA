// AlashOTA: обновление прошивки из браузера. Один раз загрузите этот скетч по USB, дальше новые версии можно заливать по Wi-Fi.
// Как обновить: Arduino IDE, Скетч, Экспорт скомпилированного бинарного файла; откройте адрес из монитора порта, логин admin, пароль ниже; выберите файл .ino.bin.
#include <WiFi.h>        // для ESP8266 замените на <ESP8266WiFi.h>
#include <AlashOTA.h>

const char *WIFI_NAME = "ИМЯ_СЕТИ";
const char *WIFI_PASS = "ПАРОЛЬ_СЕТИ";

AlashOTA ota;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);

  ota.setInfo("Мой датчик v1.0");        // покажется на странице: так видно, какая версия работает сейчас
  if (!ota.begin("1234")) {              // пароль обязателен (логин admin). Поменяйте его!
    Serial.println(ota.errorText());
    while (true) delay(1000);
  }
  Serial.print("Обновление прошивки: ");
  Serial.println(ota.url());
}

void loop() {
  ota.update();
  // ... ваша программа ...
}
