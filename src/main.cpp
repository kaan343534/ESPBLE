#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>
//
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


const char* ssid = "TP-Link_9D6C";
const char* password = "85823440";

//Your Domain name with URL path or IP address with path
String serverName = "http://smartiotware.com";


namespace {
  BLEServer *pServer = nullptr;
  BLEService *pService = nullptr;
  BLECharacteristic *pCharacteristic = nullptr;
  
  // the following variables are unsigned longs because the time, measured in
  // milliseconds, will quickly become a bigger number than can be stored in an int.
  // Timer set to 10 minutes (600000)
  //unsigned long timerDelay = 600000;
  // Set timer to 5 seconds (5000)
  bool deviceConnected = false;
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void httpPost(String param) {
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = serverName;
      String payload = param;
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
    
      // Send HTTP GET request
      int httpResponseCode = http.POST(payload.c_str());
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}

void initBLE() {  

    BLEDevice::init("ESP 32 Server");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE |
                                          BLECharacteristic::PROPERTY_NOTIFY |
                                          BLECharacteristic::PROPERTY_INDICATE
                                        );

    pCharacteristic->setValue("");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  initBLE();
  //xTaskCreatePinnedToCore(&initBLE,"Init BLE",1024*4,NULL,2,NULL,1);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to wifi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void loop() {
  
  if (deviceConnected) {
    String value = pCharacteristic->getValue().c_str();
    Serial.println("Device connected getting value from service characteristics");
    if (value != "") {
      httpPost(value);
      pCharacteristic->setValue("");
    }
  }

  delay(2000);
}