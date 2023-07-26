/*
  ----------------------------------------------------------
  ===================== [ CREATED BY ] =====================
  ===================== [   $ADMIN   ] =====================
  ----------------------------------------------------------
*/

#include <Arduino.h> // Base library
#include <ESP8266WiFi.h> // Connect to WiFi
#include <ESP8266HTTPClient.h> //To send HTTP packages
#include <OneWire.h> // Dependencies for DallasTemperature
#include <DallasTemperature.h> // For getting temperature from sensor

// DOWNLOAD FROM: https://github.com/milesburton/Arduino-Temperature-Control-Library
// DOWNLOAD DEPENDENCIES (OneWire): https://github.com/PaulStoffregen/OneWire

// ----------------------------[ CONFIGS ]----------------------------

// WiFi configs (esp8266 will connect to it)
#define WIFI_NETWORK_NAME "My_Home"  // name (ssid) of access point (wifi network)
#define WIFI_NETWORK_PASSWORD "0660353875" // password of network

// Connecting configs
#define SERVER_URL "http://getmytemperature.pythonanywhere.com" // url of python server (server will catch datas from esp8266)
#define URL_TO_UPLOAD_TEMP SERVER_URL + (String) "/addtemp?temp="
#define TIMEOUT 15 // delay from requests to server

// Sensor configs
#define SensorPin 0     // sensor`s pin (where it has been connected) 
#define DHTTYPE DHT11 // sensor type: DHT11 or DHT22

int FlautOfSensor = 0;

// Interface`s configs
#define USE_SERIAL Serial

// -------------------------------------------------------------------

OneWire oneWireConnector(SensorPin);
DallasTemperature sensorAPI(&oneWireConnector);


float temperature;
long int lastMillis = millis();

void setup() {
  USE_SERIAL.begin(115200); // Starting (com port) interface
  sensorAPI.begin(); // Starting sensor

  connectToNetwork();
}

void connectToNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK_NAME, WIFI_NETWORK_PASSWORD);

  USE_SERIAL.print("[INFO] Connecting to network");
  while (WiFi.status() != WL_CONNECTED) {
    USE_SERIAL.print(".");
    delay(500);
  }

  USE_SERIAL.print("[+] Connected to ");
  USE_SERIAL.println(WIFI_NETWORK_NAME);
  USE_SERIAL.print("[+] IP Address is : ");
  USE_SERIAL.println(WiFi.localIP());

  lastMillis = millis();
}

void loop() {
  sensorAPI.requestTemperatures();
  temperature = sensorAPI.getTempCByIndex(0); // Read temperature as Celsius (the default)

  // Check if reading was successful
  if (temperature == DEVICE_DISCONNECTED_C) { // If data is incorrect
    delay(TIMEOUT);
    USE_SERIAL.println("[-] Failed to read from DHT sensor. Try again...");
    delay(TIMEOUT);
    return;
  }

  HTTPClient http; // create http client

  USE_SERIAL.println("[HTTP] begin...");
  Serial.println("Now temperature: " + (String) temperature);
  String temperatureParsed = (String) (temperature + FlautOfSensor);
  temperatureParsed[2] = '_';
  http.begin(URL_TO_UPLOAD_TEMP + temperatureParsed);              // connecting to server and send getted datas    String) temperature
  http.addHeader("Content-Type", "text/plain");
  USE_SERIAL.println("[HTTP] GET...");
  int httpCode = http.GET();           // getting datas (code)

  Serial.println(httpCode);

  http.end();


  if (millis() - lastMillis > TIMEOUT * 4) {
    USE_SERIAL.println("[-] Disconnected.");
    connectToNetwork();
    lastMillis = millis();
  }

  delay(TIMEOUT);
}
