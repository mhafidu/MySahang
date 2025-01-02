#include <HardwareSerial.h>
#include <lorawan.h>

// relay
// Definisi pin relay
const int relay1Pin = 32;
const int relay2Pin = 33;
const int relay3Pin = 15;
const int relay4Pin = 26;

// Variabel untuk menyimpan status relay
bool relay1Status = false;
bool relay2Status = false;
bool relay3Status = false;
bool relay4Status = false;

// Parameter LoRaWAN (ganti dengan nilai dari platform Antares Anda)
const char *devAddr = "ec55c48a";                          // Replace with the Device Address
const char *nwkSKey = "f8ba620b0dae0b659b10e54c05f3fa31";  // Replace with the Network Session Key
const char *appSKey = "c574164debb6e95b21ca939a5936d782";  // Replace with the Application Session Key

// Interval pengiriman data (1 menit = 60.000 milidetik)
const unsigned long interval = 60000;
unsigned long previousMillis = 0;

// Variabel untuk pengiriman data
String dataSend = "";
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

void setup() {
  // Atur pin relay sebagai output
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  pinMode(relay4Pin, OUTPUT);

  // Pastikan relay dalam kondisi mati di awal
  digitalWrite(relay1Pin, HIGH);  // HIGH berarti relay mati (aktif low)
  digitalWrite(relay2Pin, HIGH);
  digitalWrite(relay3Pin, HIGH);
  digitalWrite(relay4Pin, HIGH);

  // Inisialisasi Serial Monitor
  Serial.begin(9600);
  SerialFromArduino.begin(9600, SERIAL_8N1, 16, 17);  // Baud rate harus sama dengan Arduino
  Serial.println("ESP32 siap menerima data dari Arduino");

  // Inisialisasi LoRaWAN
  delay(2000);
  if (!lora.init()) {
    Serial.println("RFM95 tidak terdeteksi!");
    while (1)
      ;  // Berhenti jika modul tidak terdeteksi
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
  // Simulasi data JSON dummy
  String jsonData = "{\"Relay1\":true, \"Relay2\":false, \"Relay3\":true, \"Relay4\":false}";
  // Parsing manual
  relay1Status = extractBoolValue(jsonData, "\"Relay1\":");
  relay2Status = extractBoolValue(jsonData, "\"Relay2\":");
  relay3Status = extractBoolValue(jsonData, "\"Relay3\":");
  relay4Status = extractBoolValue(jsonData, "\"Relay4\":");

  // Baca data dari Arduino jika tersedia
  if (SerialFromArduino.available()) {
    dataSend = SerialFromArduino.readStringUntil('\n');
    Serial.println("Data diterima dari Arduino:");
    Serial.println(dataSend);

    // Konversi String ke array char
    dataSend.toCharArray(payload, 256);
    digitalWrite(relay1Pin, LOW);
    digitalWrite(relay2Pin, LOW);
    digitalWrite(relay3Pin, LOW);
    digitalWrite(relay4Pin, LOW);
  }

  // Kirim data ke Antares setiap interval waktu
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    if (dataSend.length() > 0) {
      Serial.println("Mengirim data ke Antares:");
      Serial.println(dataSend);
      lora.sendUplink(payload, strlen(payload), 0, 5);
      Serial.println("Data telah dikirim.");
      // Kontrol relay berdasarkan status
      digitalWrite(relay1Pin, relay1Status ? LOW : HIGH);
      digitalWrite(relay2Pin, relay2Status ? LOW : HIGH);
      digitalWrite(relay3Pin, relay3Status ? LOW : HIGH);
      digitalWrite(relay4Pin, relay4Status ? LOW : HIGH);
    } else {
      Serial.println("Tidak ada data baru untuk dikirim.");
    }
  }

  // Periksa apakah ada data yang diterima dari Antares (Downlink)
  lora.update();
  char outStr[255];
  byte recvStatus = lora.readData(outStr);
  if (recvStatus) {
    Serial.println("Pesan Downlink diterima:");
    Serial.println(outStr);
  }
}

// Fungsi untuk mengekstrak nilai boolean dari JSON
bool extractBoolValue(String json, String key) {
  int startIndex = json.indexOf(key);
  if (startIndex == -1) {
    return false;  // Jika key tidak ditemukan, default ke false
  }
  startIndex += key.length();
  int endIndex = json.indexOf(',', startIndex);
  if (endIndex == -1) {
    endIndex = json.indexOf('}', startIndex);
  }
  String value = json.substring(startIndex, endIndex);
  value.trim();            // Hilangkan spasi atau karakter tidak penting
  return value == "true";  // Kembalikan true jika value adalah "true"
}