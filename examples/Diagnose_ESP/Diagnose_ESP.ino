// AlashOTA: страница обновления не открывается? Скетч покажет состояние Wi-Fi, адрес и свободное место, и подскажет, что проверить.
#include <WiFi.h>        // для ESP8266 замените на <ESP8266WiFi.h>
#include <AlashOTA.h>

const char *WIFI_NAME = "ИМЯ_СЕТИ";
const char *WIFI_PASS = "ПАРОЛЬ_СЕТИ";

AlashOTA ota;

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) delay(250);
  ota.setLanguage(AlashOTA::LANG_RU);       // LANG_KK: қазақша, LANG_EN: English
  ota.begin("1234");
  ota.diagnose(Serial);
}

void loop() {
  ota.update();
}
