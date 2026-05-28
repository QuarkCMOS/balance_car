#include <Arduino.h>
#include "communication/wifi_manager.h"
#include "communication/websocket_manager.h"
#include "communication/http_server.h"
#include "config/constants.h"

WiFiManagerCustom  wifi;
WebSocketManager   ws(WS_PORT);
HttpServer         http;

static uint32_t lastMs = 0;

void setup() {
    Serial.begin(115200);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    http.begin();
    ws.begin();

    ws.onCommand([](const String& key, float val) {
        Serial.printf("[CMD] %s = %.2f\n", key.c_str(), val);
    });
}

void loop() {
    wifi.update();
    ws.handleClient();

    if (millis() - lastMs >= 50) {
        lastMs = millis();
        ws.sendTelemetry(
            "{\"angle\":5.2,\"pwm_l\":200,\"pwm_r\":200,\"rpm_l\":45.0,\"rpm_r\":44.5}"
        );
    }
}