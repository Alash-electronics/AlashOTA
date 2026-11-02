# AlashOTA

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **обновления прошивки ESP32 и ESP8266 из браузера по Wi-Fi**: три строки в скетче, открыли страницу с телефона или компьютера, выбрали файл `.bin`, плата обновилась. Пароль обязателен, тип файла и чип проверяются **до записи**, поэтому неправильный файл не сломает плату. Только встроенные `WebServer` и `Update`. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <WiFi.h>
#include <AlashOTA.h>
AlashOTA ota;

void setup() {
  WiFi.begin("NETWORK", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(250);
  ota.begin("1234");              // логин admin, пароль 1234
  Serial.println(ota.url());      // http://192.168.1.50/update
}
void loop() { ota.update(); }
```

### Что умеет
- **Три строки:** `begin(пароль)`, `update()` в `loop()`, адрес страницы в `url()`.
- **Безопасно:** пароль обязателен (без него `begin()` откажет), доступ по логину admin.
- **Не сломает плату:** проверяются расширение `.bin`, признак прошивки ESP и чип (ESP32 / S2 / S3 / C3...) до записи. Если загрузка оборвалась или файл повреждён, остаётся старая прошивка.
- **Без роутера:** работает и в режиме точки доступа (пример AccessPoint_ESP).
- **Страница** на русском, казахском и английском по языку браузера, с полосой загрузки и названием текущей прошивки (`setInfo`).
- **Обратные вызовы:** `onStart`, `onProgress`, `onEnd`, `onError`, чтобы остановить моторы или показать процент.
- **Свой сервер или чужой:** `AlashOTA ota;` или `AlashOTA ota(server);` для готового `WebServer`.
- **Понятные подсказки** на трёх языках: `errorText()`, `diagnose(Serial)`.
- Для **ESP32 и ESP8266**. В Arduino IDE выберите схему разделов с OTA (по умолчанию есть).

### Чем отличается от других библиотек
Мы посмотрели ArduinoOTA, ElegantOTA (Ayush Sharma) и AsyncElegantOTA:
- ArduinoOTA работает только из Arduino IDE по mDNS и часто не находит плату в школьной сети; у нас обычная страница в браузере.
- ElegantOTA требует внешние библиотеки (AsyncWebServer и др.), а часть версий платная; у нас только встроенные.
- Во всех трёх нет проверки чипа: файл от ESP32-S3 на ESP32 затирает работающую прошивку. У нас проверка до записи.
- Нет подсказок на казахском.

### Примеры
`Basic_ESP`, `Progress_ESP`, `AccessPoint_ESP`, `Diagnose_ESP`

### Тесты
Имитация WebServer с загрузкой файла, Update и ESP проверяет пароль, страницу, запись прошивки байт в байт (куски по 1-1436 байт), все проверки до записи (расширение, признак ESP, чип, место), сбой записи, повреждённый файл, обрыв загрузки, отложенную перезагрузку и работу в готовом сервере: `extras/test`. Реальную запись во флеш и перезагрузку платы тест не проверяет.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**ESP32 және ESP8266 прошивкасын браузерден Wi-Fi арқылы жаңартуға** арналған қарапайым кітапхана: скетчте үш жол, телефоннан не компьютерден бетті ашып, `.bin` файлын таңдайсыз. Құпиясөз міндетті, файл түрі мен чип **жазардан бұрын** тексеріледі. Тек кірістірілген `WebServer` және `Update`.

```cpp
#include <WiFi.h>
#include <AlashOTA.h>
AlashOTA ota;

void setup() {
  WiFi.begin("NETWORK", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(250);
  ota.begin("1234");              // логин admin, пароль 1234
  Serial.println(ota.url());      // http://192.168.1.50/update
}
void loop() { ota.update(); }
```

### Мүмкіндіктері
- **Үш жол:** `begin(құпиясөз)`, `loop()` ішінде `update()`, бет мекенжайы `url()`.
- **Қауіпсіз:** құпиясөз міндетті, логин admin.
- **Тақтаны бұзбайды:** `.bin` кеңейтімі, ESP прошивка белгісі және чип жазардан бұрын тексеріледі. Жүктеу үзілсе, ескі прошивка қалады.
- **Роутерсіз:** қол жеткізу нүктесі режимінде де жұмыс істейді.
- **Бет** браузер тіліне қарай орысша, қазақша, ағылшынша; жүктеу жолағымен.
- **Кері шақырулар:** `onStart`, `onProgress`, `onEnd`, `onError`.
- **Өз серверің не дайын:** `AlashOTA ota;` не `AlashOTA ota(server);`.
- **Түсінікті кеңестер** үш тілде: `errorText()`, `diagnose(Serial)`.
- **ESP32 және ESP8266**. OTA бөлім схемасын таңдаңыз.

### Басқа кітапханалардан айырмашылығы
ArduinoOTA, ElegantOTA және AsyncElegantOTA қарадық:
- ArduinoOTA тек Arduino IDE-ден жұмыс істейді; бізде браузердегі бет.
- ElegantOTA сыртқы кітапханалар мен ақылы нұсқаларды қажет етеді; бізде тек кірістірілгендер.
- Үшеуінде де чипті тексеру жоқ; бізде жазардан бұрын тексеру бар.
- Қазақша кеңестер жоқ.

### Мысалдар
`Basic_ESP`, `Progress_ESP`, `AccessPoint_ESP`, `Diagnose_ESP`

### Тесттер
WebServer, Update және ESP имитациясы құпиясөзді, бетті, прошивканы жазуды, жазардан бұрынғы тексерулерді, жазу қатесін, үзілген жүктеуді тексереді: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for **updating ESP32 and ESP8266 firmware from a browser over Wi-Fi**: three lines in the sketch, open the page on a phone or computer, choose the `.bin` file and the board updates. A password is required and the file type and chip are checked **before writing**, so a wrong file cannot break the board. Only the built-in `WebServer` and `Update`. Made for classes and Alash Electronics kits.

```cpp
#include <WiFi.h>
#include <AlashOTA.h>
AlashOTA ota;

void setup() {
  WiFi.begin("NETWORK", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(250);
  ota.begin("1234");              // логин admin, пароль 1234
  Serial.println(ota.url());      // http://192.168.1.50/update
}
void loop() { ota.update(); }
```

### Features
- **Three lines:** `begin(password)`, `update()` in `loop()`, the page address from `url()`.
- **Safe:** a password is required (`begin()` refuses without one), login admin.
- **Will not break the board:** the `.bin` extension, the ESP image mark and the chip (ESP32 / S2 / S3 / C3...) are checked before writing. If the upload is cut or the file is damaged the old firmware stays.
- **No router needed:** works in access point mode too (AccessPoint_ESP example).
- **Page** in Russian, Kazakh and English by the browser language, with a progress bar and the name of the running firmware (`setInfo`).
- **Callbacks:** `onStart`, `onProgress`, `onEnd`, `onError` to stop motors or show the percent.
- **Own server or an existing one:** `AlashOTA ota;` or `AlashOTA ota(server);` for an existing `WebServer`.
- **Clear tips** in three languages: `errorText()`, `diagnose(Serial)`.
- For **ESP32 and ESP8266**. In Arduino IDE choose a partition scheme with OTA (the default has it).

### How it differs from other libraries
We looked at ArduinoOTA, ElegantOTA (Ayush Sharma) and AsyncElegantOTA:
- ArduinoOTA works only from Arduino IDE over mDNS and often does not find the board on a school network; ours is a normal web page.
- ElegantOTA needs external libraries (AsyncWebServer and others) and some versions are paid; ours uses only built-in ones.
- None of the three checks the chip: a file built for ESP32-S3 flashed to an ESP32 overwrites the working firmware. We check before writing.
- No Kazakh tips.

### Examples
`Basic_ESP`, `Progress_ESP`, `AccessPoint_ESP`, `Diagnose_ESP`

### Tests
A simulated WebServer with file upload, Update and ESP check the password, the page, byte-exact firmware writing (chunks of 1-1436 bytes), all pre-write checks (extension, ESP mark, chip, space), write failure, damaged file, aborted upload, the delayed restart and use inside an existing server: `extras/test`. The test does not cover real flash writing or a real board restart.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
