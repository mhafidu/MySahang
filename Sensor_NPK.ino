#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 2);  // RX, TX

int DE = 3;
int RE = 4;

unsigned long previousMillis = 0;      // Waktu terakhir pengiriman data
const unsigned long interval = 60000;  // Interval 1 menit (60000 ms)

void setup() {
  Serial.begin(9600);    // Serial untuk komunikasi ke ESP32
  mySerial.begin(4800);  // Serial untuk komunikasi dengan sensor
  pinMode(DE, OUTPUT);
  pinMode(RE, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // Kirim data setiap 1 menit
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    byte queryData[]{ 0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08 };
    byte receivedData[19];
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);

    mySerial.write(queryData, sizeof(queryData));  // Kirim query data ke sensor

    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    delay(1000);

    if (mySerial.available() >= sizeof(receivedData)) {        // Periksa apakah data tersedia
      mySerial.readBytes(receivedData, sizeof(receivedData));  // Membaca data

      // Parsing data yang diterima
      unsigned int soilHumidity = (receivedData[3] << 8) | receivedData[4];
      unsigned int soilTemperature = (receivedData[5] << 8) | receivedData[6];
      unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
      unsigned int soilPH = (receivedData[9] << 8) | receivedData[10];
      unsigned int nitrogen = (receivedData[11] << 8) | receivedData[12];
      unsigned int phosphorus = (receivedData[13] << 8) | receivedData[14];
      unsigned int potassium = (receivedData[15] << 8) | receivedData[16];

      // Konversi nilai menjadi JSON
      String jsonData = "{";
      jsonData += "\"SoilHumidity\":" + String((float)soilHumidity / 10.0) + ", ";
      jsonData += "\"SoilTemperature\":" + String((float)soilTemperature / 10.0) + ", ";
      jsonData += "\"SoilConductivity\":" + String(soilConductivity) + ", ";
      jsonData += "\"SoilPH\":" + String((float)soilPH / 10.0) + ", ";
      jsonData += "\"Nitrogen\":" + String(nitrogen) + ", ";
      jsonData += "\"Phosphorus\":" + String(phosphorus) + ", ";
      jsonData += "\"Potassium\":" + String(potassium);
      jsonData += "}";

      // Kirim data JSON ke ESP32 melalui Serial
      Serial.println(jsonData);
    }
  }
}