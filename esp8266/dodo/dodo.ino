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

//never pull HIGH this and BAT_GATE_PIN at the same time
#define SENSOR_POWER_PIN D6

#define BAT_GATE_PIN D5

#define RPI_HALTED_PIN D3 //TODO: needs pull down resistor
#define RPI_SHUTDOWN_PIN D2
#define RPI_POWER_PIN D1

// ######################### GLOBAL VARIABLES #########################
const unsigned int FW_VERSION = 11;

#define DEFAULT_SLEEP_MS (5*60*1000) //5min
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

void shutdown_rpi(bool graceful = true, unsigned int timeout_s = 40) {

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

void boot_rpi(bool await = false, unsigned int timeout_s = 40) {

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
                    Serial.print("waiting seconds: "); Serial.println(delay_ms/1000);
                    delay(delay_ms);
                }
                boot_rpi();
                break;
        }
    }
}

void setup() {
  unsigned long boot_time_ms = millis();

  //turn on sensors first, since they have 1 second wake up time
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, HIGH);

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
  error = !timeClient.update();
  unsigned long timestamp = timeClient.getEpochTime();
  if(error || timestamp < 1593453622) {
    delayed_restart("Failed to ntp sync!");
  }

  //wait here if 1 second has not passed yet
  while(millis() - boot_time_ms < 1000) {
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

  //GET REQUEST
  HTTPClient http;

  String full_url = url 
    + "?temp\_in=" + temp_ntc 
    + "&temp_out=" + temp_dht
    + "&rpi_state=-1"
    + "&timestamp=" + timestamp
    + "&bat=" + bat_charge()
    + "&fw_version=" + FW_VERSION;
  Serial.print("GET request to: "); Serial.println(full_url);
  http.begin(full_url.c_str());
  int http_response_code = http.GET();

  Serial.print("HTTP response code: "); Serial.println(http_response_code);
  
  DynamicJsonDocument json_doc(1024);
  DeserializationError json_error;
  
  if(http_response_code == 200) {
      json_error = deserializeJson(json_doc, http.getString());
  } else {
    delayed_restart("Backend error", DEFAULT_SLEEP_MS);
  }
  
  http.end();

  if (json_error) {
    delayed_restart(
        String("deserializeJson() failed: ") + json_error.c_str(),
        DEFAULT_SLEEP_MS
    );
  }

  //DISPATCH TASKS FROM BACKEND
  dispatch_tasks(json_doc);

  //if rpi is powered, turn it off after ALLOWED_RPI_UPTIME_MS to save battery
  if (rpi_state.powered) {
    while(millis() - rpi_state.boot_time_ms < ALLOWED_RPI_UPTIME_MS ) {
        delay(1000);
    }

    shutdown_rpi();
  }

  unsigned long sleep_time_ms;
  unsigned long uptime_ms = millis() - boot_time_ms;

  //if esp was on longer than DEFAULT_SLEEP_MS, no sleep
  if(uptime_ms > DEFAULT_SLEEP_MS) {
     sleep_time_ms = 10;
     
  //if esp was on shorter than 5 min, substract on time
  } else {
    sleep_time_ms = DEFAULT_SLEEP_MS - uptime_ms; 
  }

  delayed_restart("All work done", sleep_time_ms);
}

void loop() {}
