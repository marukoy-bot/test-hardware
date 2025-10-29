#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* ap_ssid = "ESP32 RX";
const char* ap_password = "00000000";
#define WS_PORT 81

String g_pos_web = "___";  // for web (UTF-8)
String g_dist = "___";

WebServer server(80);
WebSocketsServer webSocket(WS_PORT);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <title>ESP32 LoRa RX</title>
  <style>
    body { font-family: Arial, sans-serif; text-align:center; margin-top:40px; }
    .card { display:inline-block; padding:20px 30px; border-radius:8px; box-shadow: 0 2px 8px rgba(0,0,0,0.2); }
    h1 { margin:0 0 10px 0; }
    .value { font-size:2.4rem; margin:10px 0; }
  </style>
</head>
<body>
  <div class="card">
    <h1>ESP32 LoRa Receiver</h1>
    <div>Position (angle): <div id="pos" class="value">--</div></div>
    <div>Distance: <div id="dist" class="value">--</div></div>
    <p><small>Connect to Wi-Fi SSID: <strong>ESP32 RX</strong></small></p>
  </div>

<script>
  var gateway = "192.168.4.1";
  var ws;
  function initWS() {
    ws = new WebSocket("ws://" + gateway + ":81/");
    ws.onopen = function() { console.log("WS open"); };
    ws.onclose = function() { console.log("WS closed, retry in 2s"); setTimeout(initWS, 2000); };
    ws.onmessage = function(evt) {
      try {
        var obj = JSON.parse(evt.data);
        if (obj.pos !== undefined) document.getElementById('pos').innerHTML = obj.pos;
        if (obj.dist !== undefined) document.getElementById('dist').innerText = obj.dist;
      } catch(e) { console.log("parse err", e); }
    };
  }
  window.onload = initWS;
</script>
</body>
</html>
)rawliteral";

void broadcastUpdate() {
    g_dist = String(random(0, 100));
  String payload = "{\"pos\":\"" + g_pos_web + "\",\"dist\":\"" + g_dist + "\"}";
  webSocket.broadcastTXT(payload);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    broadcastUpdate();
  }
}

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void setup() {
    Serial.begin(115200);
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress ip = WiFi.softAPIP();
    Serial.println("AP IP: ");
    Serial.println(ip);

    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP Server Started.");

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("Websocket server started on port: " + String(WS_PORT));
}

unsigned long lastBroadcast = 0;
bool cw = true;
int currPos = 0;
void loop() {
    server.handleClient();
    webSocket.loop();

    if (millis() - lastBroadcast > 1000) {
        lastBroadcast = millis();
        if (cw) {
            currPos += 30;
            g_pos_web = String(currPos);
            Serial.println(g_pos_web);
            broadcastUpdate();
            if (currPos >= 360) cw = false;
        }
        else {
            currPos -= 30;
            g_pos_web = String(currPos);
            Serial.println(g_pos_web);
            broadcastUpdate();
            if (currPos < 0) cw = true;
        }
    }
    
}  