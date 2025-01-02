#include <AntaresESPMQTT.h>
#include <ArduinoJson.h>

// Relay Pins
const int relay1Pin = 32;
const int relay2Pin = 33;
const int relay3Pin = 15;
const int relay4Pin = 26;

// Antares MQTT
#define ACCESSKEY "ce0d0dc97f0249f7:9f118bba1a80a861"  // Ganti dengan access key Antares Anda
#define WIFISSID "Server"                              // Ganti dengan SSID Wi-Fi Anda
#define PASSWORD "kelompok"                            // Ganti dengan password Wi-Fi Anda

#define projectName "Innovillage_123"  // Nama proyek di Antares
#define deviceName "Lynx32"            // Nama device di Antares

AntaresESPMQTT antares(ACCESSKEY);

// Serial untuk komunikasi dengan Arduino
HardwareSerial SerialFromArduino(2);  // UART2, RX (GPIO16), TX (GPIO17)

String dataFromArduino;  // Variabel untuk menyimpan data dari Arduino
bool dataReady = false;  // Flag untuk menandai apakah data Arduino sudah siap

// Callback untuk menerima data MQTT
void callback(char topic[], byte payload[], unsigned int length) {
  antares.get(topic, payload, length);

  // Parsing JSON payload
  String jsonData = antares.getPayload();
  Serial.println("Data diterima dari MQTT:");
  Serial.println(jsonData);

  // Variabel status relay sementara
  static bool relay1Status = digitalRead(relay1Pin) == LOW;  // LOW = aktif
  static bool relay2Status = digitalRead(relay2Pin) == LOW;
  static bool relay3Status = digitalRead(relay3Pin) == LOW;
  static bool relay4Status = digitalRead(relay4Pin) == LOW;

  // Periksa apakah data JSON mengandung status relay
  if (jsonData.indexOf("\"Relay1\":") != -1) {
    relay1Status = extractBoolValue(jsonData, "\"Relay1\":");
  }
  if (jsonData.indexOf("\"Relay2\":") != -1) {
    relay2Status = extractBoolValue(jsonData, "\"Relay2\":");
  }
  if (jsonData.indexOf("\"Relay3\":") != -1) {
    relay3Status = extractBoolValue(jsonData, "\"Relay3\":");
  }
  if (jsonData.indexOf("\"Relay4\":") != -1) {
    relay4Status = extractBoolValue(jsonData, "\"Relay4\":");
  }

  // Update status relay berdasarkan nilai yang diterima atau status sebelumnya
  digitalWrite(relay1Pin, relay1Status ? LOW : HIGH);
  digitalWrite(relay2Pin, relay2Status ? LOW : HIGH);
  digitalWrite(relay3Pin, relay3Status ? LOW : HIGH);
  digitalWrite(relay4Pin, relay4Status ? LOW : HIGH);

  Serial.println("Relay status diperbarui berdasarkan MQTT.");
}


void setup() {
  // Atur pin relay sebagai output
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  pinMode(relay4Pin, OUTPUT);

  // Set relay mati saat awal
  digitalWrite(relay1Pin, HIGH);
  digitalWrite(relay2Pin, HIGH);
  digitalWrite(relay3Pin, HIGH);
  digitalWrite(relay4Pin, HIGH);

  // Serial Monitor dan Arduino
  Serial.begin(9600);
  SerialFromArduino.begin(9600, SERIAL_8N1, 16, 17);  // Baud rate untuk Arduino
  Serial.println("Inisialisasi...");

  // Koneksi Wi-Fi dan MQTT
  antares.setDebug(true);
  antares.wifiConnection(WIFISSID, PASSWORD);
  antares.setMqttServer();
  antares.setCallback(callback);

  Serial.println("Sistem siap.");
}

void loop() {
  // Periksa koneksi MQTT
  antares.checkMqttConnection();

  // Baca data dari Arduino jika tersedia
  if (SerialFromArduino.available()) {
    char c = SerialFromArduino.read();
    Serial.print(c);       // Tampilkan data yang diterima
    dataFromArduino += c;  // Tambahkan karakter ke dataFromArduino
    if (c == '\n') {       // Cek jika data sudah lengkap
      dataReady = true;    // Tandai bahwa data sudah siap
    }
  }

  // Parsing dan kirim data jika data dari Arduino sudah siap
  if (dataReady) {
    dataReady = false;  // Reset flag setelah data diolah

    // Parsing JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, dataFromArduino);

    if (error) {
      Serial.println("Error parsing JSON: ");
      Serial.println(error.c_str());
      dataFromArduino = "";  // Reset data
      return;
    }

    // Ambil data dari JSON
    float soilHumidity = doc["SoilHumidity"];
    float soilTemperature = doc["SoilTemperature"];
    int soilConductivity = doc["SoilConductivity"];
    float soilPH = doc["SoilPH"];
    int nitrogen = doc["Nitrogen"];
    int phosphorus = doc["Phosphorus"];
    int potassium = doc["Potassium"];

    // Tambahkan data ke buffer Antares
    antares.add("type", "uplink");
    antares.add("SoilHumidity", soilHumidity);
    antares.add("SoilTemperature", soilTemperature);
    antares.add("SoilConductivity", soilConductivity);
    antares.add("SoilPH", soilPH);
    antares.add("Nitrogen", nitrogen);
    antares.add("Phosphorus", phosphorus);
    antares.add("Potassium", potassium);

    // Kirim data ke Antares
    antares.publish(projectName, deviceName);
    Serial.println("data terkirim");

    // Reset buffer
    dataFromArduino = "";
  }
}

// Fungsi untuk mengekstrak nilai boolean dari JSON
bool extractBoolValue(String json, String key) {
  int startIndex = json.indexOf(key);
  if (startIndex == -1) {
    return false;  // Default ke false jika key tidak ditemukan
  }
  startIndex += key.length();
  int endIndex = json.indexOf(',', startIndex);
  if (endIndex == -1) {
    endIndex = json.indexOf('}', startIndex);
  }
  String value = json.substring(startIndex, endIndex);
  value.trim();
  return value == "true";  // Kembalikan true jika value "true"
}
