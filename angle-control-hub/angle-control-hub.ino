//
//  Angle Control Hub
//

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

#include "config.h"

class Device {
  String description;
  const char* type;
  const char* key;
  bool on;
  int onCode;
  int offCode;

  public:
  Device() { }
  
  void init(JsonVariant device) {
    description = device.asObject().get<String>("description");
    type = device["type"];
    key = device["key"];
    on = device["on"];
    onCode = device["onCode"];
    offCode = device["offCode"];
  }
  
  void update(JsonVariant event) {
    if(event.asObject().get<String>("key") == key) {
      if(event.asObject().get<String>("valueKey") == "on") {
        if(event.asObject().get<bool>("data")) {
          Serial.println("Send ON_CODE");
        } else {
          Serial.println("Send OFF_CODE");
        }
      }
    }
    if(event["type"] == "rf-receive") {
      if(event["code"] == onCode) {
        Serial.println("Receive ON_CODE");
      } else if(event["code"] == offCode) {
        Serial.println("Receive OFF_CODE");
      }
    }
  }
};

Device devices[10];
int deviceCount = 0;

void initDevices(FirebaseObject event) {
  JsonVariant dataArr = event.convertToArray("data");
  deviceCount = dataArr.asArray().size();

  for(byte i = 0; i < deviceCount; i++) {
    const char* deviceKey = dataArr[i]["key"];
    devices[i].init(dataArr[i]);
  }
}

void setup() {
  mySwitch.enableReceive(13);
  Serial.begin(115200);
  delay(500);
  Serial.println("Setup Angle Control Hub...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting...");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(WiFi.status());
    delay(500);
  }
  Serial.println();
  Serial.println("Connected.");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.stream(String("hubs/") + String(HUB_KEY) + String("/devices"));
  while (!Firebase.available()) {
    if (Firebase.failed()) {
      Serial.print("Sreaming Error: ");
      Serial.println(Firebase.error());
    }
  }
  initDevices(Firebase.readEvent());
}

void loop() {
  if (Firebase.failed()) {
    Serial.print("Sreaming Error: ");
    Serial.println(Firebase.error());
  }
    
  if (Firebase.available()) {
    FirebaseObject event = Firebase.readEvent();
    if(event.getString("type") == "put") {
      String path = event.getString("path");
      path = path.substring(1);
      JsonVariant eventInfo = event.getJsonVariant();
      eventInfo.asObject()["deviceKey"] = path.substring(0,path.indexOf('/'));
      eventInfo.asObject()["valueKey"] = path.substring(path.indexOf('/') + 1);
      updateDevices(eventInfo);
    }
  }
  if (mySwitch.available()) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& event = jsonBuffer.createObject();
    event["type"] = "rf-receive";
    event["code"] = mySwitch.getReceivedValue();
    updateDevices(event);
    mySwitch.resetAvailable();
  }
}

void updateDevices(JsonVariant eventInfo) {
  for(byte i = 0; i < deviceCount; i++) {
    devices[i].update(eventInfo);
  }
}



