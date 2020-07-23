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

#include <ArduinoJson.h>
// ######################### PIN DEFINITIONS #########################
#define DHT_PIN D7     // Digital pin connected to the DHT sensor


#define SENSOR_POWER_PIN D6



#define RPI_HALTED_PIN D3 
#define RPI_SHUTDOWN_PIN D4 //FROM d2 move to D4
#define RPI_POWER_PIN D5 //from D1 move to D5

// ######################### GLOBAL VARIABLES #########################
const unsigned int FW_VERSION = 15;

#define DEFAULT_SLEEP_MS (3*60*1000) //3min
#define ERROR_SLEEP_MS (60*1000) //1min
#define ALLOWED_RPI_UPTIME_MS (DEFAULT_SLEEP_MS*2)

int error;
DHT dht(DHT_PIN, DHT22);
Thermistor thermistor(A0, 3.3, 3.3, 1023, 10000, 10000, 25, 3950, 10, 50, 55);

// provide this only once
// stored in internal flash memory after execution
const char* ssid = ""; 
const char* password = "";

String url = "http://192.168.2.117:5000/sensor-push";
String fw_url = "http://192.168.2.117:5000/sensor-firmware";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

struct {
    bool powered;
    bool halted;
    unsigned long boot_time_ms;
} rpi_state;

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
  digitalWrite(SENSOR_POWER_PIN, LOW);
  delay(100);
  digitalWrite(BAT_GATE_PIN, HIGH);
  delay(100);
  
  int bat_v = analogRead(A0);
  digitalWrite(BAT_GATE_PIN, LOW);

  return bat_v;
}

// deep sleep
void delayed_restart(String msg = "", unsigned long msec = ERROR_SLEEP_MS) {
  msg = msg + " Going to deep sleep for ms: " + msec;
  Serial.println(msg);
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

void shutdown_rpi(bool graceful = true, unsigned int timeout_s = 40) {

  if((!rpi_state.powered) || rpi_state.halted)
    return;

  Serial.println(F("requesting rpi shutdown"));
  digitalWrite(RPI_SHUTDOWN_PIN, LOW); //request shutdown

  if(graceful) {
      Serial.println(F("waiting for rpi to shut down gracefully"));
      unsigned long request_shutdown_time = millis();
      bool timeout_reached = false;
      while(digitalRead(RPI_HALTED_PIN) == HIGH && !timeout_reached) {
        Serial.print(".");
        timeout_reached = (millis()-request_shutdown_time)/1000 < timeout_s ? false : true;
        delay(500);
      }

      if(timeout_reached) {
        Serial.println(F("timeout reached, forcing shutdown"));
      } else {
        Serial.println(F("success"));
      }
  }

  Serial.println(F("turning off rpi power"));
  digitalWrite(RPI_POWER_PIN, LOW); //turn off rpi power
  digitalWrite(RPI_SHUTDOWN_PIN, HIGH); //reset request shutdown

  rpi_state.halted = true;
  rpi_state.powered = false;
  rpi_state.boot_time_ms = 0;
}

void boot_rpi(bool await = true, unsigned int timeout_s = 40) {

  Serial.println(F("turning on rpi power"));
  digitalWrite(RPI_SHUTDOWN_PIN, HIGH); //reset request shutdown
  digitalWrite(RPI_POWER_PIN, HIGH);

  rpi_state.powered = true;
  rpi_state.boot_time_ms = millis();

  if(await) {
      Serial.println(F("waiting for rpi to leave halted state"));
      unsigned long request_boot_time = millis();
      bool timeout_reached = false;
      while(digitalRead(RPI_HALTED_PIN) == LOW && !timeout_reached) {
        Serial.print(".");
        timeout_reached = (millis()-request_boot_time)/1000 < timeout_s ? false : true;
        delay(500);
      }

      if(timeout_reached) {
        Serial.println(F("timeout reached, could not start rpi"));
        rpi_state.halted = true;
      } else {
        Serial.println(F("success"));
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
//                Serial.println("task id 1");
//                if(! task["params"].isNull() && 
//                   ! task["params"]["delay_s"].isNull()) {
//
//                    unsigned int delay_ms = task["params"]["delay_s"].as<unsigned int>()*1000;
//                    Serial.print("waiting seconds: "); Serial.println(delay_ms/1000);
//                    delay(delay_ms);
//                    
//                }
//                shutdown_rpi();
//                break;
    
            //turn on rpi and wait until it left halted state
            //has to be turned off after ALLOWED_RPI_UPTIME_MS
            //params: delay_s
            case 2:
                Serial.println("task id 2");
                if(! task["params"].isNull() && 
                   ! task["params"]["delay_s"].isNull()) {

                    unsigned int delay_ms = task["params"]["delay_s"].as<unsigned int>()*1000;
                    Serial.print(F("waiting seconds: ")); Serial.println(delay_ms/1000);
                    delay(delay_ms);
                }
                boot_rpi();
                break;
        }
    }
}

String sensor_push() {

  //turn on sensors and wait 1 sec
  digitalWrite(BAT_GATE_PIN, LOW);
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  delay(1000);
  
  const float temp_ntc = thermistor.readTempC();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  //const float hum_dht = dht.readHumidity();
  // Read temperature as Celsius (the default)
  const float temp_dht = dht.readTemperature();

  //turn off sensors
  digitalWrite(SENSOR_POWER_PIN, LOW);

  // Check if any reads failed and exit early (to try again).
  //if (isnan(hum_dht) || isnan(temp_dht)) {
  if(isnan(temp_dht)) {
    Serial.println("Failed to read DHT sensor");
    return "";
  }

  error = !timeClient.update();
  unsigned long timestamp = timeClient.getEpochTime();
  if(error || timestamp < 1593453622) {
    Serial.println(F("Failed to ntp sync"));
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
  Serial.print(F("GET request to: ")); Serial.println(full_url);
  http.begin(full_url.c_str());
  int http_response_code = http.GET();
  
  String response = http.getString();
  http.end();

  Serial.print(F("HTTP response code: ")); Serial.println(http_response_code);

  if(http_response_code != 200) {
    Serial.println(F("Wrong HTTP response code"));
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
    Serial.println(msg);
    return false;
  }

  //DISPATCH TASKS FROM BACKEND
  dispatch_tasks(json_doc);
}

void setup() {
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, LOW);

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
  Serial.print(F("Reset reason: "));
  Serial.println(ESP.getResetReason());
  
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

  rpi_state.powered = false;
  rpi_state.halted = false;
  rpi_state.boot_time_ms = 0;

  if (!sensor_push_and_dispatch()) {
    delayed_restart("communication error", DEFAULT_SLEEP_MS);
  }

  unsigned long sensor_push_time_ms = millis();
  
  //if rpi is powered, turn it off after ALLOWED_RPI_UPTIME_MS to save battery
  if (rpi_state.powered) {
    unsigned long now_ms = millis();
    while(now_ms - rpi_state.boot_time_ms < ALLOWED_RPI_UPTIME_MS ) {

        if(now_ms - sensor_push_time_ms >= DEFAULT_SLEEP_MS) {
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
  if(time_since_last_sensor_push_ms > DEFAULT_SLEEP_MS ) {
    sleep_time_ms = 1;

  //substract time since last sensor_push from sleep time
  } else {
    sleep_time_ms = DEFAULT_SLEEP_MS - time_since_last_sensor_push_ms;
  }

  delayed_restart("All work done", sleep_time_ms);
}

void loop() {}
