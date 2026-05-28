#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Balance Car Dashboard</title>
  <style>
    body { font-family: 'Segoe UI', Arial, sans-serif; background-color: #121212; color: #ffffff; text-align: center; margin: 0; padding: 20px; }
    h2 { color: #00ffcc; }
    .container { display: flex; flex-wrap: wrap; justify-content: center; max-width: 900px; margin: 0 auto; }
    .card { background-color: #1e1e1e; padding: 20px; margin: 10px; width: 280px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.5); text-align: left; }
    .card h3 { margin-top: 0; color: #ff007f; border-bottom: 1px solid #333; padding-bottom: 5px; }
    .value { font-size: 20px; font-weight: bold; color: #ffeb3b; float: right; }
    .input-group { margin-bottom: 10px; }
    .input-group label { display: inline-block; width: 40px; }
    .input-group input { width: 100px; background: #333; color: #fff; border: 1px solid #555; padding: 5px; border-radius: 4px; }
    .btn { background: #007bff; color: #fff; border: none; padding: 8px 15px; border-radius: 5px; cursor: pointer; width: 100%; font-size: 16px; }
    .btn:active { background: #0056b3; }
    .control-btn { background: #28a745; width: 70px; height: 50px; margin: 5px; font-size: 14px; border: none; color: white; border-radius: 8px; }
  </style>
</head>
<body>
  <h2>XE CÂN BẰNG - CONTROL PANEL</h2>
  <div class="container">

    <div class="card">
      <h3>Cấu Hình PID</h3>
      <div class="input-group">
        <label>P:</label><input type="number" id="p_val" step="0.1" value="0.0">
      </div>
      <div class="input-group">
        <label>I:</label><input type="number" id="i_val" step="0.1" value="0.0">
      </div>
      <div class="input-group">
        <label>D:</label><input type="number" id="d_val" step="0.01" value="0.0">
      </div>
      <button class="btn" onclick="sendPID()">Gửi Hệ Số</button>
    </div>

    <div class="card">
      <h3>Trạng Thái Hệ Thống</h3>
      <p>Trạng thái: <span class="value" id="status">Disconnected</span></p>
      <p>Góc nghiêng: <span class="value" id="pitch">0.0</span>°</p>
      <p>PWM Motor: <span class="value" id="pwm">0 / 0</span></p>
      <p>Tốc độ (Enc): <span class="value" id="speed">0 / 0</span></p>
    </div>

    <div class="card" style="text-align: center;">
      <h3>Điều Khiển Hướng</h3>
      <button class="control-btn" onclick="sendMove('FORWARD')">Tiến</button><br>
      <button class="control-btn" onclick="sendMove('LEFT')">Trái</button>
      <button class="control-btn" style="background:#dc3545;" onclick="sendMove('STOP')">Dừng</button>
      <button class="control-btn" onclick="sendMove('RIGHT')">Phải</button><br>
      <button class="control-btn" onclick="sendMove('BACKWARD')">Lùi</button>
    </div>

  </div>

  <script>
    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket;

    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen    = function(e) { document.getElementById('status').innerHTML = "Connected"; };
      websocket.onclose   = function(e) { document.getElementById('status').innerHTML = "Disconnected"; setTimeout(initWebSocket, 2000); };
      websocket.onmessage = onMessage;
    }

    // Nhận dữ liệu từ ESP32
    // ESP32 gửi: { angle, pwm_l, pwm_r, rpm_l, rpm_r }
    function onMessage(event) {
      var data = JSON.parse(event.data);
      if (data.angle !== undefined)
        document.getElementById('pitch').innerHTML = parseFloat(data.angle).toFixed(2);
      if (data.pwm_l !== undefined && data.pwm_r !== undefined)
        document.getElementById('pwm').innerHTML   = data.pwm_l + " / " + data.pwm_r;
      if (data.rpm_l !== undefined && data.rpm_r !== undefined)
        document.getElementById('speed').innerHTML = parseFloat(data.rpm_l).toFixed(1)
                                                   + " / "
                                                   + parseFloat(data.rpm_r).toFixed(1);
    }

    // Gửi hệ số PID lên ESP32
    function sendPID() {
      var pidData = {
        type: "PID",
        p: parseFloat(document.getElementById('p_val').value),
        i: parseFloat(document.getElementById('i_val').value),
        d: parseFloat(document.getElementById('d_val').value)
      };
      websocket.send(JSON.stringify(pidData));
      console.log("Sent PID:", pidData);
    }

    // Gửi lệnh điều hướng (tạo sẵn để đấy)
    function sendMove(direction) {
      var moveData = { type: "MOVE", dir: direction };
      websocket.send(JSON.stringify(moveData));
      console.log("Sent Move:", direction);
    }

    window.addEventListener('load', initWebSocket);
  </script>
</body>
</html>
)rawliteral";

#endif