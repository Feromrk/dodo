// ######################### MISC #########################
#define ENABLE_PRINT //comment this to disable all serial debug outputs
#ifndef ENABLE_PRINT
// disable Serial output
#define Serial SomeOtherwiseUnusedName
static class {
  public:
    void begin(...) {}
    void print(...) {}
    void println(...) {}
} Serial;
#endif

// ######################### INCLUDES #########################
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <DHT.h>
#include <thermistor.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

// ######################### PIN DEFINITIONS #########################
#define DHTPIN D1     // Digital pin connected to the DHT sensor

// ######################### GLOBAL VARIABLES #########################
int error;
DHT dht(DHTPIN, DHT22);
Thermistor *thermistor;


#define ESP_PLATFORM

// provide this only once
// stored in internal flash memory after execution
const char* ssid = ""; 
const char* password = "";

String url = "http://192.168.2.117:5000/update-sensor";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// this function never returns
void delayed_restart(String msg = "", unsigned long msec = 30*1000) {
  Serial.println(msg + " Restarting in ms: " + msec);
  unsigned long now = millis();
  while(1) {
    ArduinoOTA.handle();

    if(millis() - now > msec) {
      ESP.restart();
    }

    yield();
  }
}

void setup() {
  Serial.begin(9600);
  delay(10);

  Serial.println();
  Serial.println(F("Starting up"));

  ArduinoOTA.begin();

  dht.begin();
  thermistor = new Thermistor(A0, 3.3, 3.3, 1023, 10000, 10000, 25, 3950, 10, 50);

  Serial.println();

  WiFi.mode(WIFI_STA);
  if(strlen(ssid) > 0 && strlen(password) > 0 ) {
    Serial.print(F("Connecting to "));
    Serial.print(ssid);
    WiFi.begin(ssid, password);
  } else {
    Serial.print(F("Connecting to stored wifi config"));
    WiFi.begin();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.print(F("IP: ")); Serial.println(WiFi.localIP());

  timeClient.begin();
  error = !timeClient.update();
  unsigned long timestamp = timeClient.getEpochTime();
  if(error || timestamp < 1593453622) {
    delayed_restart("Failed to ntp sync!");
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  const float hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  const float temp = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp)) {
    delayed_restart("Failed to read from DHT sensor!");
  }

  const float temp_ntc = thermistor->readTempC();

  //TODO 
  //read RPI state

  //GET REQUEST
  HTTPClient http;

  String full_url = url 
    + "?temp\_in=" + temp 
    + "&temp_out=" + temp_ntc
    + "&hum_in=" + hum
    + "&rpi_state=-1"
    + "&timestamp=" + timestamp
    + "&bat=-1";
  Serial.print("GET request to: "); Serial.println(full_url);
  http.begin(full_url.c_str());
  int http_response_code = http.GET();

  Serial.print("HTTP response code: "); Serial.println(http_response_code);
  if(http_response_code == 200) {
      String payload = http.getString();
      Serial.println(payload);
  }
  http.end();

  //TODO 
  //Aufgaben aus payload abarbeiten
  
  delayed_restart("All work done", 5*60*1000); //5min
}

void loop() {}
