/*************************************************************************************
 * The sketch is in part based on Edoardo Viola's (@edovio) TemperatureMonitor_v2_1: *
 * pin definitions, setup and connecting to WiFi                                     *
 * This code is under Creative Commons - Attribution 4.0 International (CC BY 4.0)   *
 * ***********************************************************************************/

//Library for the DHT11 sensor
#include "DHTesp.h"
DHTesp dht;

//Libraries for Mozilla IOT Gateway
#include <Arduino.h>
#include "Thing.h"
#include "WebThingAdapter.h"

//Pin Conversion for ESP8266
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

//Define about the type and PIN Connection for the Temperature Sensor
#define DHTPIN D3

//Temperature Sensor Inizialization
int samplingPeriod = 1000 * 60 * 5;

double lastHumidityValue = 0;
double lastTemperatureValue = 0;

const double TemperatureThreshold = 0.1;
const double HumidityThreshold = 1;

//Led Status PIN
#if defined(LED_BUILTIN)
const int ledPin = LED_BUILTIN;
#else
const int ledPin = 13; // manually configure LED pin
#endif

//Your wifi credentials here
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebThingAdapter *adapter;

const char* deviceTypes[] = {"MultiLevelSensor", "Sensor", nullptr};
// Sets up the device info, first two parameters are free-form strings
ThingDevice temp("DHT11-1", "DHT11 Sensor", deviceTypes);

ThingProperty temperature("temperature", "Input pin", NUMBER, "TemperatureProperty");
ThingProperty humidity("humidity", "Input pin", NUMBER, "HumidityProperty");

bool lastOn = false;

void setup(void){
  dht.setup(DHTPIN, DHTesp::DHT11); // Change second parameter to DHTesp::DHT22 for DHT22 support
  pinMode(ledPin,OUTPUT);
  Serial.begin(115200);
  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(ssid);
  Serial.println("\"");
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  bool blink = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(ledPin, blink ? LOW : HIGH); // active low led
    blink = !blink;
  }
  digitalWrite(ledPin, HIGH); // active low led

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  adapter = new WebThingAdapter("w25", WiFi.localIP());
  
  temp.addProperty(&temperature);
  temp.addProperty(&humidity);
  adapter->addDevice(&temp);
  adapter->begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(temp.id);
}

void loop(void){
  delay(samplingPeriod);
  
  float humidityVal = dht.getHumidity();
  float temperatureVal = dht.getTemperature();
  if (isnan(humidityVal) || isnan(temperatureVal)) {
    delay(1000);
  } else if (humidityVal > 100) {
    Serial.println("Invalid reading.");
    delay(1000);
  } else {
    double humidityValDouble = (double) humidityVal;
    if (fabs(humidityValDouble - lastHumidityValue) >= HumidityThreshold) {
      Serial.print("log: Humidity: ");
      Serial.print(humidityValDouble);
      Serial.println("%");
      ThingPropertyValue humidityLevelValue;
      humidityLevelValue.number = humidityValDouble;
      humidity.setValue(humidityLevelValue);
      lastHumidityValue = humidityValDouble;
    }
    
    double temperatureValDouble = (double) temperatureVal;
    if (fabs(temperatureValDouble - lastTemperatureValue) >= TemperatureThreshold) {
      Serial.print("log: Temperature: ");
      Serial.print(temperatureValDouble);
      Serial.println("°C");
      ThingPropertyValue temperatureLevelValue;
      temperatureLevelValue.number = temperatureValDouble;
      temperature.setValue(temperatureLevelValue);
      lastTemperatureValue = temperatureValDouble;
    } else {
      Serial.print("Diff:");
      Serial.print(temperatureValDouble);
      Serial.print(" ");
      Serial.print(lastTemperatureValue);
      Serial.print(" ");
      Serial.println(fabs(humidityValDouble - lastHumidityValue));
    }
    adapter->update();
  }
}
