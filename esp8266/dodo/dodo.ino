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

#include <ESP8266httpUpdate.h>

#include <DHT.h>
#include <thermistor.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

// ######################### PIN DEFINITIONS #########################
#define DHT_PIN D7     // Digital pin connected to the DHT sensor

//never pull HIGH this and BAT_GATE_PIN at the same time
#define SENSPOR_POWER_PIN D6

#define BAT_GATE_PIN D5

#define RPI_HALTED_PIN D3
#define RPI_SHUTDOWN_PIN D2
#define RPI_POWER_PIN D1

// ######################### GLOBAL VARIABLES #########################
const unsigned int FW_VERSION = 8;

int error;
DHT dht(DHT_PIN, DHT22);
Thermistor thermistor(A0, 3.3, 3.3, 1023, 10000, 10000, 25, 3950, 10, 50, -20);

// provide this only once
// stored in internal flash memory after execution
const char* ssid = ""; 
const char* password = "";

String url = "http://192.168.2.117:5000/sensor-push";
String fw_url = "http://192.168.2.117:5000/sensor-firmware";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// busy waiting
//void delayed_restart(String msg = "", unsigned long msec = 60*1000) {
//  Serial.println(msg + " Restarting in ms: " + msec);
//  unsigned long now = millis();
//  while(1) {
//
//    if(millis() - now > msec) {
//      ESP.restart();
//    }
//
//    yield();
//  }
//}

int bat_charge() {
  //turn off sensors
  //now it is allowed to read battery voltage
  digitalWrite(SENSPOR_POWER_PIN, LOW);
  delay(100);
  digitalWrite(BAT_GATE_PIN, HIGH);
  delay(100);
  
  int bat_v = analogRead(A0);
  digitalWrite(BAT_GATE_PIN, LOW);

  return bat_v;
}

// deep sleep
void delayed_restart(String msg = "", unsigned long msec = 60*1000) {
  Serial.println(msg + " Going to deep sleep for ms: " + msec);
  ESP.deepSleep(msec*1000, WAKE_RF_DEFAULT);
}

void firmware_update() {
  Serial.println(F("Looking for new firmware"));
  t_httpUpdate_return ret = ESPhttpUpdate.update(fw_url, String(FW_VERSION));
  switch(ret) {
      case HTTP_UPDATE_FAILED:
          Serial.print(F("Firmware update failed: "));
          Serial.print(ESPhttpUpdate.getLastError());
          Serial.println(ESPhttpUpdate.getLastErrorString());
          break;
      case HTTP_UPDATE_NO_UPDATES:
          Serial.println(F("Firmware up to date"));
          break;
      case HTTP_UPDATE_OK:
          Serial.println(F("Firmware Update complete")); // may not be called since we reboot the ESP
          break;
  }
}

void setup() {

  //turn on sensors, since they have 1 second wake up time
  pinMode(SENSPOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSPOR_POWER_PIN, HIGH);
  unsigned long sensor_startup_time = millis();  

  pinMode(BAT_GATE_PIN, OUTPUT);
  digitalWrite(BAT_GATE_PIN, LOW);

  pinMode(RPI_HALTED_PIN, INPUT);

  pinMode(RPI_SHUTDOWN_PIN, OUTPUT);
  digitalWrite(RPI_SHUTDOWN_PIN, HIGH);

  pinMode(RPI_POWER_PIN, OUTPUT);
  digitalWrite(RPI_POWER_PIN, LOW);

  
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.print(F("Firmware version: "));
  Serial.println(FW_VERSION);

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

  firmware_update();

  dht.begin();

  timeClient.begin();
  error = !timeClient.update();
  unsigned long timestamp = timeClient.getEpochTime();
  if(error || timestamp < 1593453622) {
    delayed_restart("Failed to ntp sync!");
  }

  //wait here if 1 second has not passed yet
  while(millis() - sensor_startup_time < 1000) {
    delay(100);
  }

  const float temp_ntc = thermistor.readTempC();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  //const float hum_dht = dht.readHumidity();
  // Read temperature as Celsius (the default)
  const float temp_dht = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  //if (isnan(hum_dht) || isnan(temp_dht)) {
  if(isnan(temp_dht)) {
    delayed_restart("Failed to read from DHT sensor!");
  }

  //turn off sensors
  //now it is possible to read battery voltage
  digitalWrite(SENSPOR_POWER_PIN, LOW);

  //TODO 
  //read RPI state

  //TODO
  //read battery state
  int bat_v = bat_charge();
  Serial.print("battery charge: ");
  Serial.println(bat_v);

  //GET REQUEST
  HTTPClient http;

  String full_url = url 
    + "?temp\_in=" + temp_ntc 
    + "&temp_out=" + temp_dht
    + "&rpi_state=-1"
    + "&timestamp=" + timestamp
    + "&bat=" + bat_v
    + "&fw_version=" + FW_VERSION;
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
  //dispatch tasks from backend response

//  Serial.println("turning on rpi");
//  digitalWrite(RPI_POWER_PIN, HIGH);
//  delay(400000);
//
//  Serial.println("requesting shutdown");
//  digitalWrite(RPI_SHUTDOWN_PIN, LOW); //request shutdown
//
//  Serial.println("waiting for RPI to turn off");
//  while(digitalRead(RPI_HALTED_PIN) == HIGH) {
//    Serial.print(".");
//    delay(1000);
//  }
  
  delayed_restart("All work done", 5*60*1000); //5min
}

void loop() {}
