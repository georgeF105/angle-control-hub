//
//  Angle Control Hub
//

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <SoftwareSerial.h>

#include "config.h"

//SoftwareSerial mySerial(3,4); // pin 2 = TX, pin 3 = RX (unused)

class Device {
  String description;
  const char* type;
  const char* key;
  bool on;
  int onCode;
  int offCode;

  public:
  Device() {
    
  }
  void init(JsonVariant device) {
    Serial.print("init device");
    device.printTo(Serial);
    description = device.asObject().get<String>("description");
    type = device["type"];
    key = device["key"];
    on = device["on"];
    onCode = device["onCode"];
    offCode = device["offCode"];
  }
  void update(JsonVariant event) {
    if(event.asObject().get<String>("key") == key) {
      Serial.print("Updating Device: ");
      Serial.print(key);
      Serial.print("  Action: ");
      if(event.asObject().get<String>("valueKey") == "on") {
        if(event.asObject().get<bool>("data")) {
          Serial.println("Send ON_CODE");
        } else {
          Serial.println("Send OFF_CODE");
        }
      }
    }
  }
};

Device devices[10];
int deviceCount = 0;

void initDevices(FirebaseObject event) {
  JsonVariant dataArr = event.convertToArray("data");
//  Serial.print("Initilizing Devices...");
//  dataArr.printTo(Serial);
//  Serial.println();
  deviceCount = dataArr.asArray().size();

  for(byte i = 0; i < deviceCount; i++) {
//    Serial.print("Device #");
//    Serial.print(i);
//    Serial.print(" = ");
//    dataArr[i].printTo(Serial);
//    Serial.println();
//    Serial.print("device key: ");
    const char* deviceKey = dataArr[i]["key"];
//    Serial.println(deviceKey);
    devices[i].init(dataArr[i]);
  }
}

void setup() {
  
//  mySerial.begin(9600); // set up serial port for 9600 baud
//  delay(500); // wait for display to boot up

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
//    Serial.println("EVENT!");
    FirebaseObject event = Firebase.readEvent();
    if(event.getString("type") == "put") {
      String path = event.getString("path");
      path = path.substring(1);
      JsonVariant eventInfo = event.getJsonVariant();
      eventInfo.asObject()["deviceKey"] = path.substring(0,path.indexOf('/'));
      eventInfo.asObject()["valueKey"] = path.substring(path.indexOf('/') + 1);
      updateDevices(eventInfo);
    }
    /*
    Serial.print("UPDATE. event type: ");
    Serial.println(event.getString("type"));
    event.getJsonVariant().printTo(Serial);
    Serial.println();
    if(event.getString("type") == "put") {
      String path = event.getString("path");
      path = path.substring(1);
      Serial.print("Trimed path");
      Serial.println(path);
      String deviceKey = path.substring(0,path.indexOf('/'));
      Serial.print("Device Key: ");
      Serial.println(deviceKey);
      String valueKey = path.substring(path.indexOf('/') + 1);
      Serial.print("Value Key: ");
      Serial.println(valueKey);
    }*/
    
  }
  
//  JsonVariant devices = fetchDevices();
//  int deviceCount = devices.asArray().size();
//
//  Device deviceArr[deviceCount];
//
//  for(byte i = 0; i < deviceCount; i++) {
//    deviceArr[i].init(devices[i]);
//  }
//  delay(2000);
  /*
  Firebase.stream(String("hubs/") + String(HUB_KEY) + String("/devices"));
  
  while(1) {
    if (Firebase.failed()) {
      Serial.print("Sreaming Error: ");
      Serial.println(Firebase.error());
    }
    if (Firebase.available()) {
      
      FirebaseObject event = Firebase.readEvent();
      Serial.print("UPDATE. event type: ");
      Serial.println(event.getString("type"));
      for(byte i = 0; i < deviceCount; i++) {
        deviceArr[i].update(event);
      }
    }
//    delay(1000);
  }*/
}

void updateDevices(JsonVariant eventInfo) {
  for(byte i = 0; i < deviceCount; i++) {
    devices[i].update(eventInfo);
  }
}

JsonVariant fetchDevices() {
  return Firebase.get(String("hubs/") + String(HUB_KEY) + String("/devices")).convertToArray();
}



