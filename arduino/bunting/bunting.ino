#include <ArduinoJson.h> //https://arduinojson.org/
#include <BuntingMachine.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Motor steps per revolution.
#define MOTOR_STEPS 200
#define RPM 25
#define DEPLOY_DEGREES 2484
#define DIR_MULTI -1  // -1 for new version, +1 for old version

// Microstepping 1=full step, 2=half step etc.
#define MICROSTEPS 32

// Control pins
#define DIR 32
#define STEP 33
#define ENABLE 26

//Stopper pin
#define STOPPER 12
#define debounceTime 500

//BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP, ENABLE);
//BuntingMachine bm(stepper, STOPPER, DIR_MULTI, DEPLOY_DEGREES, RPM, MICROSTEPS);
BuntingMachine bm(DIR_MULTI, MOTOR_STEPS, DIR, STEP, ENABLE, STOPPER, DEPLOY_DEGREES, RPM, MICROSTEPS);


// Network Setup
#define SSID "SSID"
#define PSK "PSK"
#define HOLIDAY_URL "http://api.doineedbunting.today/england-and-wales"
#define MAX_RETRIES 3

HTTPClient http;

// JSON
const size_t bufferSize = JSON_OBJECT_SIZE(4) + 70;
DynamicJsonBuffer jsonBuffer(bufferSize);

// ######################################
// ############### NETWORK ##############
// ######################################

void connectWiFi() {
  Serial.print("Connecting to WiFi..");
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(SSID);
}

void disconnectWiFi() {
  Serial.println("Disconnecting from WiFi");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

// ######################################
// ############### DATETIME #############
// ######################################

// Return true/false for deploying bunting
bool shouldDeployBunting() {
  const char* json = getHolidayJson();
  JsonObject& today = jsonBuffer.parseObject(json);
  
  if (today.get<bool>("bunting") == true) {
    Serial.print("Get the bunting! :) It's ");
    Serial.println(today.get<char*>("title")); 
    return today.get<bool>("bunting");
  }
  Serial.println("Sadly, today is not a day for bunting :(");
  return false;
}

const char* getHolidayJson() {
  static String json;
  bool success = false;
  int attempts = 0;
  connectWiFi();

  do {
    http.begin(HOLIDAY_URL); //Specify the URL
    int httpCode = http.GET(); //Make the request
    if (httpCode > 0) { //Check for the returning code
        Serial.print("Got today's data. ");
        json = http.getString();
        success = true;
      }
    else {
      Serial.println("Error, retrying in 5 seconds...");
      delay(5000);
      attempts++;
    }
    http.end(); //Free the resources
  } while ((success == false) and 
           (attempts <= MAX_RETRIES));
  if (success == false) {
    Serial.println("Connection failed");
  }
  disconnectWiFi();
  return json.c_str();
}

// ######################################
// ############## INTERRUPT #############
// ######################################

void stopISR() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > debounceTime) 
  {
    bm.stop();
  }
  last_interrupt_time = interrupt_time;
}

// ######################################
// ################# MAIN ###############
// ######################################

void setup() {
  // initialize the serial port:
  Serial.begin(115200);
  delay(100);
  
  // Setup the Bunting Machine interrupt and wind in the bunting.
  bm.setup(stopISR);
  delay(1000);

  // Connect to the WiFi
//  connectWiFi();
//  delay(1000);
}

void loop() {
  if (shouldDeployBunting()) {
    bm.deploy();
  } else {
    bm.store();
  }
  delay(3600000);
//  delay(60000);
}

