//
//  Angle Control Hub
//

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

#include "config.h"

#define MAX_DEVICES 20

class Device {
  String description;
  const char* type;
  String key;
  bool on;
  int onCode;
  int offCode;

  public:
  Device() { }
  
  void init(JsonVariant device) {
    Serial.println("init device: ");
    device.printTo(Serial);
    Serial.println();
    description = device.asObject().get<String>("description");
    type = device["type"];
    key = device.asObject().get<String>("key");
    on = device["on"];
    onCode = device["onCode"];
    offCode = device["offCode"];
  }
  
  void update(JsonVariant event) {
    if(event.asObject().get<String>("deviceKey") == key) {
      if(event.asObject().get<String>("valueKey") == String("on")) {
        if(event.asObject().get<bool>("data")) {
          Serial.print(description);
          Serial.println(": Send ON_CODE");
        } else {
          Serial.print(description);
          Serial.println(": Send OFF_CODE");
        }
      }
    }
    if(event["type"] == "rf-receive") {
      if(event["code"] == onCode) {
        Serial.print(description);
        Serial.println(": Receive ON_CODE");
      } else if(event["code"] == offCode) {
        Serial.print(description);
        Serial.println(": Receive OFF_CODE");
      }
    }
  }
};

Device devices[10];
int deviceCount = 0;

void initDevices(FirebaseObject event) {
  JsonVariant dataArr = convertToArray(event.getJsonVariant("data"));
  deviceCount = dataArr.asArray().size();
  for(byte i = 0; i < deviceCount; i++) {
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
    Serial.println("EVENT!");
    FirebaseObject event = Firebase.readEvent();
    event.getJsonVariant().printTo(Serial);
    Serial.println();
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

JsonVariant convertToArray(JsonVariant object) {
  StaticJsonBuffer<2000> jsonBuffer;
  JsonArray& array = jsonBuffer.createArray();
  JsonVariant node;
  for(JsonObject::iterator it=object.asObject().begin(); it!=object.asObject().end(); ++it) {
    node = it->value;
    node.asObject().set("key",it->key);
    array.add(node);
  }
  return array;
}



