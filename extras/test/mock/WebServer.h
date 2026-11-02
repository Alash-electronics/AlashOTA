// Имитация WebServer (ESP32) с загрузкой файла: тест кладёт запросы в очередь, обработчики вызываются как в настоящем сервере.
#pragma once
#include <algorithm>
#include <map>
#include <vector>
#include "Arduino.h"
enum HTTPMethod { HTTP_GET, HTTP_POST };
enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };
struct HTTPUpload { HTTPUploadStatus status; String filename; uint8_t buf[1436]; size_t currentSize = 0; size_t totalSize = 0; };
struct HttpReq {
  HTTPMethod method = HTTP_GET; std::string path, auth;      // auth: "admin:пароль" или пусто
  bool hasUpload = false; std::string filename; std::vector<uint8_t> data; size_t chunk = 1000; bool abortAt = false; size_t abortAfter = 0;
};
struct HttpResp { int code; std::string type, body; };
inline std::vector<HttpReq> g_httpQueue;
inline std::vector<HttpResp> g_httpResp;
class WebServer {
 public:
  explicit WebServer(int p) : port(p) {}
  int port; bool started = false;
  struct Route { std::function<void()> fn, up; };
  std::map<std::pair<std::string, int>, Route> routes;
  HttpReq cur; HTTPUpload up_;
  void on(const char *path, HTTPMethod m, std::function<void()> f) { routes[{path, m}].fn = f; }
  void on(const char *path, HTTPMethod m, std::function<void()> f, std::function<void()> u) { routes[{path, m}] = {f, u}; }
  void begin() { started = true; }
  HTTPUpload &upload() { return up_; }
  void send(int code, const char *type, const String &body) { g_httpResp.push_back({code, type, body.s}); }
  bool authenticate(const char *u, const char *p) { return cur.auth == std::string(u) + ":" + p; }
  void requestAuthentication() { g_httpResp.push_back({401, "auth", ""}); }
  void handleClient() {
    while (!g_httpQueue.empty()) {
      cur = g_httpQueue.front(); g_httpQueue.erase(g_httpQueue.begin());
      auto it = routes.find({cur.path, cur.method});
      if (it == routes.end()) { g_httpResp.push_back({404, "", ""}); continue; }
      if (cur.hasUpload && it->second.up) {
        up_.status = UPLOAD_FILE_START; up_.filename = String(cur.filename.c_str()); up_.currentSize = 0; it->second.up();
        size_t total = 0;
        for (size_t pos = 0; pos < cur.data.size(); pos += cur.chunk) {
          up_.status = UPLOAD_FILE_WRITE; up_.currentSize = std::min(cur.chunk, cur.data.size() - pos);
          memcpy(up_.buf, &cur.data[pos], up_.currentSize); total += up_.currentSize; up_.totalSize = total;
          it->second.up();
          if (cur.abortAt && total >= cur.abortAfter) break;
        }
        up_.status = cur.abortAt ? UPLOAD_FILE_ABORTED : UPLOAD_FILE_END; up_.currentSize = 0; it->second.up();
      }
      it->second.fn();
    }
  }
};
