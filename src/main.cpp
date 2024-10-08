#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "SPIFFS.h"
#include "FS.h"
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <string.h>

// Coisas do DS18B20 ================
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float tempDS18B20;

// Coisas do DHT22 ==================
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float tempDHT, humiDHT;

// Coisas do WiFi ===================
const char* ssid = "ESP32_Point";
const char* password = "password_1234";

// Coisas do Web Server
AsyncWebServer server(80);

// Coisas do Botão Push
#define BUTTON_PIN 18
bool buttonPressed = false;

// Variável para contar as amostras
int sampleCount = 0;

// Função para salvar dados em CSV
void saveDataToCSV(float data1, float data2, float data3) {
  File file = SPIFFS.open("/data.csv", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  // Incrementa o contador de amostras
  sampleCount++;

  // Salva os dados no formato: contagem, dado1, dado2, dado3
  file.printf("%d,%f,%f,%f\n", sampleCount, data1, data2, data3);
  file.close();
  Serial.printf("Data saved to CSV: %d, %f, %f, %f\n", sampleCount, data1, data2, data3);
}

// Função para lidar com o botão push
void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

// Função para configurar o WiFi e o servidor
void setupWiFiAndServer() {
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Inicia o mDNS
  if (MDNS.begin("iot-lea")) {
    Serial.println("mDNS started");
  }

  // Rota para download do arquivo CSV
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/data.csv", "text/csv");
  });

  server.begin();
  Serial.println("Webserver started");
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  dht.begin();

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  // Configuração do botão push
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, handleButtonPress, FALLING);
}

void loop() {
  // Leitura dos sensores
  sensors.requestTemperatures();
  tempDS18B20 = sensors.getTempCByIndex(0);

  tempDHT = dht.readTemperature();
  humiDHT = dht.readHumidity();

  if (isnan(humiDHT) || isnan(tempDHT)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print("DS18B20 Temp: ");
  Serial.print(tempDS18B20);
  Serial.print(" °C, DHT Temp: ");
  Serial.print(tempDHT);
  Serial.print(" °C, Humidity: ");
  Serial.print(humiDHT);
  Serial.println(" %");

  // Salva os dados em CSV
  saveDataToCSV(tempDS18B20, tempDHT, humiDHT);

  // Verifica se o botão foi pressionado
  if (buttonPressed) {
    buttonPressed = false;
    // Configura WiFi e servidor web
    setupWiFiAndServer();
    Serial.println("Button pressed! CSV file available for download.");
  }

  // Aguarda alguns segundos antes de realizar outra leitura
  delay(2000);
}
