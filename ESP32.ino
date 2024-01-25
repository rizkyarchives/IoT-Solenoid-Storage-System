/*
  Proyek untuk: Smart Storage System for Enhanced Protection
  Kode untuk: ESP-32
  Ringkasan fungsi kode:
  - Send & receive data to/from Arduino UNO
  - Send & receive data to/from Web Server
  - Hosts for the Web Server
  - Send data to Thingspeak for Data Logging
  - Uses SPIFFS
*/

// Import library yang dibutuhkan
#include <HardwareSerial.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>

// Gunakan ssid dan password Wi-Fi Anda
const char* ssid = "Zenfone 9_3936";
const char* password = "painguin";

// Variabel untuk keperluan komunikasi dengan Arduino UNO
char incomingData;

String thingSpeakAddress = "api.thingspeak.com";
String writeAPIKey;
String tsfield1Name;
String request_string;
WiFiClient client;

// Kode untuk keperluan data logging dengan thingspeak
void kirim_thingspeak(float digValue) {
  if (client.connect("api.thingspeak.com", 80)) {
    request_string = "/update?";
    request_string += "key=";
    request_string += "VOCCTJQEWDUNNJ69"; //Gunakan API Keys Thingspeak Anda
    request_string += "&";
    request_string += "field1";
    request_string += "=";
    request_string += digValue;
    Serial.println(String("GET ") + request_string + " HTTP/1.1\r\n" +
                 "Host: " + thingSpeakAddress + "\r\n" +
                 "Connection: close\r\n\r\n");
                 
    client.print(String("GET ") + request_string + " HTTP/1.1\r\n" +
                 "Host: " + thingSpeakAddress + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");

  }
}

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// Fungsi untuk mengirim data ke Web Server
void notifyClients(String datas) {
  ws.textAll(datas);
}

// Fungsi untuk menerima dan memproses data dari Web Server
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "correct") == 0) {// "correct" = PIN berhasil dari Webserver
      Serial2.print("1");
      kirim_thingspeak(1.0); //1.0 = Berhasil masuk
    }
    else if (strcmp((char*)data, "reset") == 0) { // "reset" = Storage System ingin dikunci kembali
      Serial2.print("2");
    }
    else if (strcmp((char*)data, "incorrect") == 0) { //"incorrect" = PIN yang dimasukkan dari Webserver salah
      Serial2.print("3");
      kirim_thingspeak(0.0); //0.0 = Gagal masuk
    }
  }
}

// Kode yang akan memberitahu client (device) yang sedang mengakses webserver
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void setup(){
  // Serial.begin untuk monitoring, Serial2.begin untuk komunikasi dengan Arduino
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // pin 16 = RX2, pin 17 = TX2

  initSPIFFS();
  initWiFi();
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html",false);
  });

  server.serveStatic("/", SPIFFS, "/");

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);
  
  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  if(Serial2.available()){ // Jika menerima data dari Arduino
    incomingData = Serial2.read();
    if(incomingData == 'r'){ // 'r' = Storage system di reset
      notifyClients("reset"); //Mengirim data ke Webserver
    }else if (incomingData == 'i'){ //'i' = PIN atau RFID yang diinput salah
      kirim_thingspeak(0.0);
    }else if (incomingData == 'c'){ //'c' = PIN yang dimasukkan benar
      kirim_thingspeak(1.0);
      notifyClients("correct"); //Mengirim data ke Webserver
    }
  }
  incomingData = ' ';

}
