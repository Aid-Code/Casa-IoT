/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "ORT-IoT"
#define WIFI_PASSWORD "OrtIOT24"

// Insert Firebase project API Key
#define API_KEY "AIzaSyA2oT3O2Pua7ZgGroxqvzpvH7g0MxER0Ds"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://casa-iot-c2a84-default-rtdb.firebaseio.com/" 

#define SWITCH 4

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

int lectura_switch = 0;
int estado_actual = 0;
int estado_previo = 0;

bool cambio = false;
bool light_on = false;
bool light_on_bd = false;

void setup()
{
  Serial.begin(115200);
  pinMode(SWITCH, INPUT_PULLUP);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  lectura_switch = digitalRead(SWITCH);

  estado_actual = lectura_switch;

  if (estado_actual != estado_previo)
  {
    cambio = true;
  }
  else if (estado_actual == estado_previo)
  {
    cambio = false;
  }

  estado_previo = lectura_switch;

  if (Firebase.ready() && signupOK)
  {
    if (Firebase.RTDB.getBool(&fbdo, "pedernera/hab_aiden/luz_techo")) 
    {
      if (fbdo.dataType() == "bool") light_on_bd = fbdo.boolData();
      
      if (cambio && light_on_bd == false) light_on = true;
      
      if (cambio && light_on_bd == true) light_on = false;
    }

    if (Firebase.RTDB.setBool(&fbdo, "pedernera/hab_aiden/luz_techo", light_on))
    {
      Serial.print("LED: ");
      if (light_on) Serial.println("ON");
      else Serial.println("OFF");
    }
    else 
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
