#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Definisi pin relay
const int relay1Pin = 32;
const int relay2Pin = 33;
const int relay3Pin = 15;
const int relay4Pin = 26;

// Konfigurasi Access Point
const char* ssid = "ESP32_MySahang";    // Nama Wi-Fi ESP32
const char* password = "MySahang2024";  // Password Wi-Fi

// Inisialisasi server web
WebServer server(80);

// Serial untuk komunikasi dengan sensor (misalnya dari Arduino lain)
HardwareSerial SerialFromArduino(2);  // UART2, RX (GPIO16), TX (GPIO17)

// Variabel untuk menyimpan data sensor
float soilHumidity = 0;
float soilTemperature = 0;
int soilConductivity = 0;
float soilPH = 0;
int nitrogen = 0;
int phosphorus = 0;
int potassium = 0;

// Fungsi untuk halaman utama
void handleRoot() {
  String html = "<html><head><title>ESP32 Web Server</title></head><body>";
  html += "<h1>Monitoring Data Sensor</h1>";
  html += "<p><strong>Soil Humidity:</strong> " + String(soilHumidity) + "%</p>";
  html += "<p><strong>Soil Temperature:</strong> " + String(soilTemperature) + "°C</p>";
  html += "<p><strong>Soil Conductivity:</strong> " + String(soilConductivity) + " µS/cm</p>";
  html += "<p><strong>Soil PH:</strong> " + String(soilPH) + "</p>";
  html += "<p><strong>Nitrogen:</strong> " + String(nitrogen) + " ppm</p>";
  html += "<p><strong>Phosphorus:</strong> " + String(phosphorus) + " ppm</p>";
  html += "<p><strong>Potassium:</strong> " + String(potassium) + " ppm</p>";
  html += "<h2>Relay Controls</h2>";
  html += "<p><a href=\"/relay1/on\">Relay 1 ON</a> | <a href=\"/relay1/off\">Relay 1 OFF</a></p>";
  html += "<p><a href=\"/relay2/on\">Relay 2 ON</a> | <a href=\"/relay2/off\">Relay 2 OFF</a></p>";
  html += "<p><a href=\"/relay3/on\">Relay 3 ON</a> | <a href=\"/relay3/off\">Relay 3 OFF</a></p>";
  html += "<p><a href=\"/relay4/on\">Relay 4 ON</a> | <a href=\"/relay4/off\">Relay 4 OFF</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Fungsi untuk mengontrol relay
void handleRelay1On() {
  digitalWrite(relay1Pin, LOW);  // Relay aktif
  server.send(200, "text/plain", "Relay 1 is ON");
}

void handleRelay1Off() {
  digitalWrite(relay1Pin, HIGH);  // Relay nonaktif
  server.send(200, "text/plain", "Relay 1 is OFF");
}

void handleRelay2On() {
  digitalWrite(relay2Pin, LOW);  // Relay aktif
  server.send(200, "text/plain", "Relay 2 is ON");
}

void handleRelay2Off() {
  digitalWrite(relay2Pin, HIGH);  // Relay nonaktif
  server.send(200, "text/plain", "Relay 2 is OFF");
}

void handleRelay3On() {
  digitalWrite(relay3Pin, LOW);  // Relay aktif
  server.send(200, "text/plain", "Relay 3 is ON");
}

void handleRelay3Off() {
  digitalWrite(relay3Pin, HIGH);  // Relay nonaktif
  server.send(200, "text/plain", "Relay 3 is OFF");
}

void handleRelay4On() {
  digitalWrite(relay4Pin, LOW);  // Relay aktif
  server.send(200, "text/plain", "Relay 4 is ON");
}

void handleRelay4Off() {
  digitalWrite(relay4Pin, HIGH);  // Relay nonaktif
  server.send(200, "text/plain", "Relay 4 is OFF");
}

void getData() {
  StaticJsonDocument<256> doc;
  doc["SoilHumidity"] = soilHumidity;
  doc["SoilTemperature"] = soilTemperature;
  doc["SoilConductivity"] = soilConductivity;
  doc["SoilPH"] = soilPH;
  doc["Nitrogen"] = nitrogen;
  doc["Phosphorus"] = phosphorus;
  doc["Potassium"] = potassium;

  String jsonResponse;
  serializeJson(doc, jsonResponse);

  server.send(200, "application/json", jsonResponse);
}

void setup() {
  // Inisialisasi serial monitor
  Serial.begin(115200);
  SerialFromArduino.begin(9600, SERIAL_8N1, 16, 17);  // Baud rate untuk sensor

  // Setup relay pins
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  pinMode(relay4Pin, OUTPUT);

  // Set semua relay ke nonaktif
  digitalWrite(relay1Pin, HIGH);
  digitalWrite(relay2Pin, HIGH);
  digitalWrite(relay3Pin, HIGH);
  digitalWrite(relay4Pin, HIGH);

  // Inisialisasi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Setup endpoint untuk halaman web
  server.on("/", handleRoot);
  server.on("/relay1/on", handleRelay1On);
  server.on("/relay1/off", handleRelay1Off);
  server.on("/relay2/on", handleRelay2On);
  server.on("/relay2/off", handleRelay2Off);
  server.on("/relay3/on", handleRelay3On);
  server.on("/relay3/off", handleRelay3Off);
  server.on("/relay4/on", handleRelay4On);
  server.on("/relay4/off", handleRelay4Off);
  server.on("/data", getData);

  // Start server
  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  // Tangani permintaan HTTP
  server.handleClient();

  // Baca data dari sensor melalui UART
  if (SerialFromArduino.available()) {
    String sensorData = SerialFromArduino.readStringUntil('\n');
    Serial.println("Data received from Arduino: " + sensorData);

    // Parsing data JSON dari sensor
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, sensorData);

    if (error) {
      Serial.println("Error parsing JSON: " + String(error.c_str()));
      return;
    }

    // Update variabel dengan data dari sensor
    soilHumidity = doc["SoilHumidity"];
    soilTemperature = doc["SoilTemperature"];
    soilConductivity = doc["SoilConductivity"];
    soilPH = doc["SoilPH"];
    nitrogen = doc["Nitrogen"];
    phosphorus = doc["Phosphorus"];
    potassium = doc["Potassium"];
  }
}
