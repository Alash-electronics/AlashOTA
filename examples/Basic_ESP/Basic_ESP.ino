// [ru] AlashOTA: обновление прошивки из браузера. Один раз загрузите этот скетч по USB, дальше новые версии можно заливать по Wi-Fi.
//      Как обновить: Arduino IDE, Скетч, Экспорт скомпилированного бинарного файла; откройте адрес из монитора порта, логин admin, пароль ниже; выберите файл .ino.bin.
// [kk] AlashOTA: браузерден микробағдарламаны жаңарту. Бұл скетчті USB арқылы бір рет жүктеңіз, одан әрі жаңа нұсқаларды Wi-Fi арқылы жүктеуге болады.
//      Қалай жаңартады: Arduino IDE, Sketch, Export Compiled Binary; порт мониторындағы мекенжайды ашыңыз, логин admin, құпиясөз төменде; .ino.bin файлын таңдаңыз.
// [en] AlashOTA: firmware update from a browser. Upload this sketch over USB once, then new versions can be uploaded over Wi-Fi.
//      How to update: Arduino IDE, Sketch, Export Compiled Binary; open the address from the serial monitor, login admin, the password below; choose the .ino.bin file.
#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
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
