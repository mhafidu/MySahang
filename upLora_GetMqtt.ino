#include <AntaresESPMQTT.h>
#include <HardwareSerial.h>
#include <lorawan.h>

// Relay
const int relay1Pin = 32;
const int relay2Pin = 33;
const int relay3Pin = 15;
const int relay4Pin = 26;

// Antares MQTT
#define ACCESSKEY "ce0d0dc97f0249f7:9f118bba1a80a861"  // Ganti dengan access key Antares Anda
#define WIFISSID "Roemah Poetih"                              // Ganti dengan SSID Wi-Fi Anda
#define PASSWORD "Abzx1234"                            // Ganti dengan password Wi-Fi Anda

#define projectName "Innovillage_123"  // Nama proyek di Antares
#define deviceName "Lynx32"            // Nama device di Antares

AntaresESPMQTT antares(ACCESSKEY);

// LoRaWAN
const char *devAddr = "ec55c48a";                          // Device Address
const char *nwkSKey = "f8ba620b0dae0b659b10e54c05f3fa31";  // Network Session Key
const char *appSKey = "c574164debb6e95b21ca939a5936d782";  // Application Session Key

const unsigned long uplinkInterval = 60000;  // Interval pengiriman data (60 detik)
unsigned long previousUplinkMillis = 0;

char payload[256];

// Definisi pin LoRaWAN
const sRFM_pins RFM_pins = {
  .CS = 5,     // NSS
  .RST = 0,    // RST
  .DIO0 = 27,  // DIO0
  .DIO1 = 2    // DIO1
};

// Serial untuk komunikasi dengan Arduino
HardwareSerial SerialFromArduino(2);  // UART2, RX (GPIO16), TX (GPIO17)

String dataFromArduino;  // Variabel untuk menyimpan data dari Arduino
bool dataReady = false;  // Flag untuk menandai apakah data Arduino sudah siap

// Fungsi callback untuk menerima data MQTT
void callback(char topic[], byte payload[], unsigned int length) {
  antares.get(topic, payload, length);

  // Parsing JSON payload
  String jsonData = antares.getPayload();
  Serial.println("Data diterima dari MQTT:");
  Serial.println(jsonData);

  // Parsing status relay dari JSON
  bool relay1Status = extractBoolValue(jsonData, "\"Relay1\":");
  bool relay2Status = extractBoolValue(jsonData, "\"Relay2\":");
  bool relay3Status = extractBoolValue(jsonData, "\"Relay3\":");
  bool relay4Status = extractBoolValue(jsonData, "\"Relay4\":");

  // Update status relay
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
  SerialFromArduino.begin(9600, SERIAL_8N1, 16, 17);  // Baud rate harus sama dengan Arduino
  Serial.println("Inisialisasi...");

  // Koneksi Wi-Fi dan MQTT
  antares.setDebug(true);
  antares.wifiConnection(WIFISSID, PASSWORD);
  antares.setMqttServer();
  antares.setCallback(callback);

  // Inisialisasi LoRaWAN
  delay(2000);
  if (!lora.init()) {
    Serial.println("RFM95 tidak terdeteksi!");
    while (1)
      ;
  }

  // Pengaturan LoRaWAN
  lora.setDeviceClass(CLASS_C);  // Gunakan Class C
  lora.setDataRate(SF10BW125);   // Pengaturan data rate
  lora.setChannel(MULTI);        // Saluran acak
  lora.setTxPower1(15);          // Kekuatan Tx maksimum
  lora.setNwkSKey(nwkSKey);      // Network Session Key
  lora.setAppSKey(appSKey);      // Application Session Key
  lora.setDevAddr(devAddr);      // Device Address

  Serial.println("LoRaWAN siap.");
}

void loop() {
  // Periksa koneksi MQTT
  antares.checkMqttConnection();

  // Baca data dari Arduino jika tersedia
  if (SerialFromArduino.available()) {
    char c = SerialFromArduino.read();
    Serial.print(c);  // Menampilkan byte per byte yang diterima
    dataFromArduino += c;  // Menambahkan karakter ke dataFromArduino
    if (c == '\n') {
      dataReady = true;  // Tandai jika karakter newline diterima
    }
  }

  // Kirim data dari Arduino ke Antares melalui LoRa setiap interval
  if (millis() - previousUplinkMillis >= uplinkInterval && dataReady) {
    previousUplinkMillis = millis();
    dataReady = false;  // Reset flag setelah data dikirim

    // Konversi data dari Arduino ke array char
    dataFromArduino.toCharArray(payload, sizeof(payload));

    // Kirim data melalui LoRa
    Serial.println("Mengirim data Arduino ke Antares melalui LoRa:");
    Serial.println(dataFromArduino);
    lora.sendUplink(payload, strlen(payload), 0, 5);
    Serial.println("Data telah dikirim.");
  }

  // Periksa apakah ada data downlink dari LoRa
  lora.update();
  char downlinkData[255];
  byte recvStatus = lora.readData(downlinkData);
  if (recvStatus) {
    Serial.println("Pesan Downlink diterima:");
    Serial.println(downlinkData);
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
