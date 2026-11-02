// [ru] AlashOTA без роутера: плата создаёт свою сеть Wi-Fi. Подключитесь к ней телефоном и откройте http://192.168.4.1/update
// [kk] AlashOTA роутерсіз: тақта өз Wi-Fi желісін жасайды. Оған телефонмен қосылып, http://192.168.4.1/update ашыңыз
// [en] AlashOTA without a router: the board creates its own Wi-Fi network. Connect to it with a phone and open http://192.168.4.1/update
#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
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
