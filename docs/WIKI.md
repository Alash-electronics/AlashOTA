# AlashOTA: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **обновления прошивки ESP32 и ESP8266 из браузера по Wi-Fi**: три строки в скетче, открыли страницу с телефона или компьютера, выбрали файл `.bin`, плата обновилась. Пароль обязателен, тип файла и чип проверяются **до записи**, поэтому неправильный файл не сломает плату. Только встроенные `WebServer` и `Update`. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashOTA** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin(password, path)` | Включить страницу обновления (логин admin) |
| `update()` | Обслуживать запросы (в loop) |
| `url()` | Адрес страницы |
| `setInfo(text)` | Название и версия на странице |
| `setAutoReboot(on)` | Перезагрузка после обновления |
| `onStart / onProgress / onEnd / onError` | Обратные вызовы |
| `updating()` | Идёт обновление |
| `diagnose(Serial)` | Самопроверка |

### Если что-то не работает
**Страница не открывается**

Телефон или компьютер должны быть в той же сети Wi-Fi. Адрес с http, не https. Запустите Diagnose_ESP.

**Какой файл выбирать**

Arduino IDE, Скетч, Экспорт скомпилированного бинарного файла. Нужен файл `имя.ino.bin` (не bootloader, не partitions, не merged).

**Ошибка «не помещается в память»**

Выберите схему разделов с OTA: Инструменты, Partition Scheme, Default (две копии приложения).

**Забыли пароль**

Он в вашем скетче (`ota.begin("...")`). Загрузите скетч по USB с новым паролем.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic_ESP`, `Progress_ESP`, `AccessPoint_ESP`, `Diagnose_ESP`

---

## Қазақша

**ESP32 және ESP8266 прошивкасын браузерден Wi-Fi арқылы жаңартуға** арналған қарапайым кітапхана: скетчте үш жол, телефоннан не компьютерден бетті ашып, `.bin` файлын таңдайсыз. Құпиясөз міндетті, файл түрі мен чип **жазардан бұрын** тексеріледі. Тек кірістірілген `WebServer` және `Update`.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashOTA** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin(password, path)` | Жаңарту бетін қосу |
| `update()` | Сұрауларға қызмет ету |
| `url()` | Бет мекенжайы |
| `setInfo(text)` | Беттегі атау |
| `setAutoReboot(on)` | Жаңартудан кейін қайта қосу |
| `onStart / onProgress / onEnd / onError` | Кері шақырулар |
| `updating()` | Жаңарту жүріп жатыр |
| `diagnose(Serial)` | Өзін тексеру |

### Бірдеңе жұмыс істемесе
**Бет ашылмайды**

Телефон не компьютер сол Wi-Fi желісінде болуы керек. http, https емес.

**Қандай файл таңдау**

Sketch, Export Compiled Binary. `аты.ino.bin` керек.

**Жадқа сыймайды**

Tools, Partition Scheme, Default (OTA бар).

**Құпиясөзді ұмыттым**

Ол скетчте. USB арқылы жаңасын жүктеңіз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic_ESP`, `Progress_ESP`, `AccessPoint_ESP`, `Diagnose_ESP`

---

## English

A simple library for **updating ESP32 and ESP8266 firmware from a browser over Wi-Fi**: three lines in the sketch, open the page on a phone or computer, choose the `.bin` file and the board updates. A password is required and the file type and chip are checked **before writing**, so a wrong file cannot break the board. Only the built-in `WebServer` and `Update`. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashOTA** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `begin(password, path)` | Enable the update page (login admin) |
| `update()` | Serve requests (in loop) |
| `url()` | Page address |
| `setInfo(text)` | Name and version on the page |
| `setAutoReboot(on)` | Restart after update |
| `onStart / onProgress / onEnd / onError` | Callbacks |
| `updating()` | Update in progress |
| `diagnose(Serial)` | Self-check |

### If something does not work
**The page does not open**

The phone or computer must be on the same Wi-Fi. Use http, not https. Run Diagnose_ESP.

**Which file to choose**

Arduino IDE, Sketch, Export Compiled Binary. You need `name.ino.bin` (not bootloader, partitions or merged).

**Error: does not fit**

Tools, Partition Scheme, choose one with OTA (Default has two app slots).

**Forgot the password**

It is in your sketch (`ota.begin("...")`). Upload the sketch over USB with a new one.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic_ESP`, `Progress_ESP`, `AccessPoint_ESP`, `Diagnose_ESP`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashOTA
