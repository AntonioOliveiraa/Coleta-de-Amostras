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

// Coisas do Web Server =============
AsyncWebServer server(80);

// Coisas do Botão Push =============
#define BUTTON_PIN 19
bool buttonPressed = false;

// Variável para contar as amostras
int sampleCount = 0;
const int maxSamples = 1000;  // Limita o número de amostras no arquivo CSV

// Função para salvar dados em CSV
void saveDataToCSV(float data1, float data2, float data3) {
  // Reseta o arquivo CSV se ultrapassar o limite de amostras
  if (sampleCount >= maxSamples) {
    SPIFFS.remove("/data.csv");  // Apaga o arquivo CSV antigo
    sampleCount = 0;
    Serial.println("CSV file reset after 1000 samples");
  }

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

// Função para cuidar com o botão push (com debounce)
void IRAM_ATTR handleButtonPress() {
  static unsigned long lastPress = 0;
  if (millis() - lastPress > 200) {  // Debounce de 200ms
    buttonPressed = true;
    lastPress = millis();
  }
}

// Função para configurar o WiFi e o servidor
void setupWiFiAndServer() {
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  if (IP) {
    Serial.print("AP IP address: ");
    Serial.println(IP);
  } else {
    Serial.println("Failed to start WiFi AP");
    return;
  }

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

  // Initializa SPIFFS (com formatação automática em caso de falha)
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  // Configuração do botão push
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, handleButtonPress, FALLING);
}

void loop() {
  // Leitura dos sensores com tentativas de repetição para o DHT
  sensors.requestTemperatures();
  tempDS18B20 = sensors.getTempCByIndex(0);

  for (int i = 0; i < 3; i++) {
    tempDHT = dht.readTemperature();
    humiDHT = dht.readHumidity();
    if (!isnan(humiDHT) && !isnan(tempDHT)) break;
    delay(1000);  // Tenta novamente após 1 segundo
  }

  if (isnan(humiDHT) || isnan(tempDHT)) {
    Serial.println(F("Failed to read from DHT sensor after 3 attempts!"));
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
