// [ru] AlashOTA: что происходит во время обновления. Обработчики onStart / onProgress / onEnd / onError: можно погасить реле, показать процент на экране.
// [kk] AlashOTA: жаңарту кезінде не болады. onStart / onProgress / onEnd / onError өңдеушілері: релені өшіруге, экранда пайызды көрсетуге болады.
// [en] AlashOTA: what happens during an update. The onStart / onProgress / onEnd / onError handlers: you can switch off a relay or show the percentage on a screen.
#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <AlashOTA.h>

const char *WIFI_NAME = "ИМЯ_СЕТИ";
const char *WIFI_PASS = "ПАРОЛЬ_СЕТИ";

AlashOTA ota;

void onStart() { Serial.println("Загрузка началась: остановите моторы и реле"); }
void onProgress(uint32_t bytes) { Serial.print("Записано байт: "); Serial.println(bytes); }
void onEnd() { Serial.println("Готово, сейчас перезагрузка"); }
void onError() { Serial.println(ota.errorText()); }

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);
  ota.onStart(onStart);
  ota.onProgress(onProgress);
  ota.onEnd(onEnd);
  ota.onError(onError);
  ota.begin("1234");
  Serial.println(ota.url());
}

void loop() {
  ota.update();
}
