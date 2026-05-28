#pragma once
#include <ESPAsyncWebServer.h>
#include "web_page.h"

class HttpServer {
public:
    void begin() {
        _server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
            req->send_P(200, "text/html", index_html);
        });
        _server.begin();
        Serial.println("[HTTP] Web server started on port 80");
    }
private:
    AsyncWebServer _server{80};
};