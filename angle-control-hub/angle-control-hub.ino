//
//  Angle Control Hub
//

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <SoftwareSerial.h>

#include "config.h"

SoftwareSerial mySerial(3,4); // pin 2 = TX, pin 3 = RX (unused)

void setup() {
  
//  mySerial.begin(9600); // set up serial port for 9600 baud
//  delay(500); // wait for display to boot up

  Serial.begin(115200);
  delay(500);
  Serial.println("Setup Angle Control Hub...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting...");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected.");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  Serial.print("Message: ");
  String message = Firebase.getString(String("hubs/") + String(HUB_KEY) + String("/message"));
  Serial.println(message);
  
  FirebaseObject devices = Firebase.get(String("hubs/") + String(HUB_KEY) + String("/devices"));
  DynamicJsonBuffer jsonBuffer;
//  JsonObject& root = devices.getJsonVariant();
  const char* type = devices.getJsonVariant()["0"]["type"];
  Serial.print("type");
  Serial.println(type);
  
  Serial.print("devices JsonVariant: ");
  devices.getJsonVariant().printTo(Serial);
  Serial.println();
  Serial.print("sub JsonVariant: ");
  devices.convertToArray().printTo(Serial);
//  Serial.println(devices.getNodeArray());
//  Serial.print("Test get array: ");
  
//  const char* testOutput = devices.getNodeArray(1)["type"];
  if(devices.failed()) {
    Serial.print("Error : ");
    Serial.println(devices.error());
  }
//  Serial.print(testOutput);
//  jsonBuffer.parseObject(FirebaseObject(devices), 3);

//  for(JsonObject::iterator key = devices.getJsonVariant().begin(); key != devices.getJsonVariant().end(); ++key) {
//    Serial.print("description: ");
//    Serial.println(devices.getJsonVariant()[*key]["description"];
//  }

  
//  boolean thing = Firebase.getBool("boolean");
  delay(3000);
}


void updateDevices() {
  
}

