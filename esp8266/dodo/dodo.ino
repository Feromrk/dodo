#define DEBUG
#ifdef DEBUG
  #define Sprintl(a) (Serial.print(a))
  #define Sprintln(a) (Serial.println(a))
#else
  #define Sprintl(a)
  #define Sprintln(a)
#endif

// ######################### INCLUDES #########################
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <ESP8266httpUpdate.h>

#include <DHT.h>
#include <Adafruit_ADS1015.h>
#include <thermistor.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <ArduinoJson.h>

// ######################### PIN DEFINITIONS #########################
#define DHT_PIN D7
#define SENSOR_POWER_PIN D6
#define RPI_HALTED_PIN D3 
#define RPI_SHUTDOWN_PIN D4 
#define RPI_POWER_PIN D5 

// ######################### GLOBAL VARIABLES #########################
#define FW_VERSION 16
#define CYCLE_TIME_MS (3*60*1000) //3min
#define ALLOWED_RPI_UPTIME_MS (CYCLE_TIME_MS*2)

int error;

// provide this only once
// stored in internal flash memory after execution
const char* ssid = ""; 
const char* password = "";

String url = "http://192.168.2.117:5000/sensor-push";
String fw_url = "http://192.168.2.117:5000/sensor-firmware";

struct {
    bool powered;
    bool halted;
    unsigned long boot_time_ms;
} rpi_state;

// ######################### PROGRAM LOGIC #########################

int bat_charge() {
  int bat_v = analogRead(A0);
  return bat_v;
}

void delayed_restart(String msg, unsigned long msec) {
  msg = msg + " Going to deep sleep for ms: " + msec;
  Sprintln(msg);
  ESP.deepSleep(msec*1000, WAKE_RF_DEFAULT);
}

void firmware_update() {
  Sprintln(F("Looking for new firmware"));
  t_httpUpdate_return ret = ESPhttpUpdate.update(fw_url, String(FW_VERSION));
  switch(ret) {
      case HTTP_UPDATE_FAILED:
          Sprintl(F("Firmware update failed: "));
          Sprintl(ESPhttpUpdate.getLastError());
          Sprintln(ESPhttpUpdate.getLastErrorString());
          break;
      case HTTP_UPDATE_NO_UPDATES:
          Sprintln(F("Firmware up to date"));
          break;
      case HTTP_UPDATE_OK:
          Sprintln(F("Firmware Update complete")); // may not be called since we reboot the ESP
          break;
  }
}

void shutdown_rpi(bool graceful = true, unsigned int timeout_s = 40) {

  if((!rpi_state.powered) || rpi_state.halted)
    return;

  Sprintln(F("requesting rpi shutdown"));
  digitalWrite(RPI_SHUTDOWN_PIN, LOW); //request shutdown

  if(graceful) {
      Sprintln(F("waiting for rpi to shut down gracefully"));
      unsigned long request_shutdown_time = millis();
      bool timeout_reached = false;
      while(digitalRead(RPI_HALTED_PIN) == HIGH && !timeout_reached) {
        Sprintl(".");
        timeout_reached = (millis()-request_shutdown_time)/1000 < timeout_s ? false : true;
        delay(500);
      }

      if(timeout_reached) {
        Sprintln(F("timeout reached, forcing shutdown"));
      } else {
        Sprintln(F("success"));
      }
  }

  Sprintln(F("turning off rpi power"));
  digitalWrite(RPI_POWER_PIN, LOW); //turn off rpi power
  digitalWrite(RPI_SHUTDOWN_PIN, HIGH); //reset request shutdown

  rpi_state.halted = true;
  rpi_state.powered = false;
  rpi_state.boot_time_ms = 0;
}

void boot_rpi(bool await = true, unsigned int timeout_s = 40) {

  Sprintln(F("turning on rpi power"));
  digitalWrite(RPI_SHUTDOWN_PIN, HIGH); //reset request shutdown
  digitalWrite(RPI_POWER_PIN, HIGH);

  rpi_state.powered = true;
  rpi_state.boot_time_ms = millis();

  if(await) {
      Sprintln(F("waiting for rpi to leave halted state"));
      unsigned long request_boot_time = millis();
      bool timeout_reached = false;
      while(digitalRead(RPI_HALTED_PIN) == LOW && !timeout_reached) {
        Sprintl(".");
        timeout_reached = (millis()-request_boot_time)/1000 < timeout_s ? false : true;
        delay(500);
      }

      if(timeout_reached) {
        Sprintln(F("timeout reached, could not start rpi"));
        rpi_state.halted = true;
      } else {
        Sprintln(F("success"));
        rpi_state.halted = false;
      }
  }
}

//json format: [{'id':2, 'params':{'delay_s': 5}}, {'id':5, 'params':{'timeout': 45}}]
void dispatch_tasks(const JsonDocument &json_doc) {
    auto tasks = json_doc.as<JsonArray>();
    for(const auto& task : tasks) {
        switch(task["id"].as<unsigned int>()) {
            //turn off rpi gracefully
            //params: delay_s
            //rpi will always be turned off after ALLOWED_RPI_UPTIME_MS, no need to turn off manually
//            case 1:
//                Sprintln("task id 1");
//                if(! task["params"].isNull() && 
//                   ! task["params"]["delay_s"].isNull()) {
//
//                    unsigned int delay_ms = task["params"]["delay_s"].as<unsigned int>()*1000;
//                    Sprintl("waiting seconds: "); Sprintln(delay_ms/1000);
//                    delay(delay_ms);
//                    
//                }
//                shutdown_rpi();
//                break;
    
            //turn on rpi and wait until it left halted state
            //has to be turned off after ALLOWED_RPI_UPTIME_MS
            //params: delay_s
            case 2:
                Sprintln("task id 2");
                if(! task["params"].isNull() && 
                   ! task["params"]["delay_s"].isNull()) {

                    unsigned int delay_ms = task["params"]["delay_s"].as<unsigned int>()*1000;
                    Sprintl(F("waiting seconds: ")); Sprintln(delay_ms/1000);
                    delay(delay_ms);
                }
                boot_rpi();
                break;
        }
    }
}

String sensor_push() {

  DHT dht(DHT_PIN, DHT22);
  Adafruit_ADS1115 ads;
  Thermistor thermistor(3, 3.3, 10000, 10000, 25, 3950, 10, 50, &ads);

  //turn on sensors and wait 1 sec
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  delay(1000);

  dht.begin();
  ads.begin();
  delay(10);

  const float temp_ntc = thermistor.readTempC();
  const float temp_dht = dht.readTemperature();

  //turn off sensors
  digitalWrite(SENSOR_POWER_PIN, LOW);

  if(isnan(temp_dht)) {
    Sprintln("Failed to read DHT sensor");
    return "";
  }

  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);
  timeClient.begin();  
  error = !timeClient.update();
  unsigned long timestamp = timeClient.getEpochTime();
  if(error || timestamp < 1593453622) {
    Sprintln(F("Failed to ntp sync"));
    return "";
  }

  //GET REQUEST
  HTTPClient http;

  String full_url = url 
    + "?temp\_in=" + temp_ntc 
    + "&temp_out=" + temp_dht
    + "&rpi_state=" + (rpi_state.powered ? 1 : 0)
    + "&timestamp=" + timestamp
    + "&bat=" + bat_charge()
    + "&fw_version=" + FW_VERSION;
  Sprintl(F("GET request to: ")); Sprintln(full_url);
  http.begin(full_url.c_str());
  int http_response_code = http.GET();
  
  String response = http.getString();
  http.end();

  Sprintl(F("HTTP response code: ")); Sprintln(http_response_code);

  if(http_response_code != 200) {
    Sprintln(F("Wrong HTTP response code"));
    return "";
  }

  return response;
}

//false on error, true otherwise
bool sensor_push_and_dispatch() {
  
  DynamicJsonDocument json_doc(1024);
  DeserializationError json_error;

  String response = sensor_push();
  
  if(response != "") {
      json_error = deserializeJson(json_doc, response);
  } else {
    return false;
  }

  if (json_error) {
    String msg = String("deserializeJson() failed: ") + json_error.c_str();
    Sprintln(msg);
    return false;
  }

  //DISPATCH TASKS FROM BACKEND
  dispatch_tasks(json_doc);
}

void setup() {
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, LOW);

  pinMode(RPI_HALTED_PIN, INPUT);

  pinMode(RPI_SHUTDOWN_PIN, OUTPUT);
  digitalWrite(RPI_SHUTDOWN_PIN, HIGH);

  pinMode(RPI_POWER_PIN, OUTPUT);
  digitalWrite(RPI_POWER_PIN, LOW);
  
  Serial.begin(115200);
  delay(10);

  Sprintln();
  Sprintl(F("Reset reason: "));
  Sprintln(ESP.getResetReason());
  
  Sprintl(F("Firmware version: "));
  Sprintln(FW_VERSION);

  WiFi.mode(WIFI_STA);
  if(strlen(ssid) > 0 && strlen(password) > 0 ) {
    Sprintl(F("Connecting to "));
    Sprintl(ssid);
    WiFi.begin(ssid, password);
  } else {
    Sprintl(F("Connecting to stored wifi config"));
    WiFi.begin();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Sprintl(F("."));
  }
  Sprintln();
  Sprintln(F("WiFi connected"));
  Sprintl(F("IP: ")); Sprintln(WiFi.localIP());

  firmware_update();

  rpi_state.powered = false;
  rpi_state.halted = false;
  rpi_state.boot_time_ms = 0;

  if (!sensor_push_and_dispatch()) {
    delayed_restart("communication error", CYCLE_TIME_MS);
  }

  unsigned long sensor_push_time_ms = millis();
  
  //if rpi is powered, turn it off after ALLOWED_RPI_UPTIME_MS to save battery
  if (rpi_state.powered) {
    unsigned long now_ms = millis();
    while(now_ms - rpi_state.boot_time_ms < ALLOWED_RPI_UPTIME_MS ) {

        if(now_ms - sensor_push_time_ms >= CYCLE_TIME_MS) {
          sensor_push_and_dispatch();
          sensor_push_time_ms = millis();
        }
      
        delay(1000);
        now_ms = millis();
    }

    shutdown_rpi();
  }

  unsigned long sleep_time_ms;
  unsigned long time_since_last_sensor_push_ms = millis() - sensor_push_time_ms;

  //no sleep, if sensor_push needs to be done immediately
  if(time_since_last_sensor_push_ms > CYCLE_TIME_MS ) {
    sleep_time_ms = 1;

  //substract time since last sensor_push from sleep time
  } else {
    sleep_time_ms = CYCLE_TIME_MS - time_since_last_sensor_push_ms;
  }

  delayed_restart("All work done", sleep_time_ms);
}

void loop() {}
