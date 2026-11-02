// AlashOTA без роутера: плата создаёт свою сеть Wi-Fi. Подключитесь к ней телефоном и откройте http://192.168.4.1/update
#include <WiFi.h>        // для ESP8266 замените на <ESP8266WiFi.h>
#include <AlashOTA.h>

AlashOTA ota;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Alash-OTA", "12345678");     // имя сети и пароль (не короче 8 символов)
  ota.setInfo("Робот v1.0");
  ota.begin("1234");
  Serial.println(ota.url());
}

void loop() {
  ota.update();
}
