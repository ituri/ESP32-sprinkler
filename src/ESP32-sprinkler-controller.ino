/*
    ESP32-sprinkler-controller.ino
    Copyright (C) 2021 Luca Kilkenny
    GNU GPLv3
    https://github.com/bugfx/ESP32-sprinkler-controller
*/


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>



// https://github.com/me-no-dev/ESPAsyncWebServer/issues/325#issuecomment-917680259
#define WS_MAX_QUEUED_MESSAGES 32
// Lib for Dashboard
  #include <Arduino.h>
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <ESPDash.h>


// the included example files can be used to generate the needed credential files
// if saved in the libraries folder the files can be included in different projects
#include "credentialsWifi.h"
//#include "credentialsMqttServer.h"


// ------------------------------------------------
//  ESP32-WROOM-32 - pin definitions
// ------------------------------------------------

//io-pin                name of the area to sprinkle        terminal in the control cabinet
#define valve01Pin 27   //Bewässerung Hofeinfahrt           Klemme 06
#define valve02Pin 12   //Bewässerung Hofeinfahrt           Klemme 06

// ------------------------------------------------
//  definitions
// ------------------------------------------------

char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password


const char* mqttServer = "192.168.1.10";
const int mqttPort = 1883;


WiFiClient espClient;
PubSubClient mqttClient(espClient);

String mqttIncommingTopic = "";
String mqttIncommingPayload = "";
String mqttPayload = "";

unsigned long millisLastUpdateHeartbeat = millis();

//define names for the mqtt topics used for the heartbeat and valves
const String heartbeat = "SYS000AE01SW01";
const String valve01command = "SAN320YS01SW01/command";
const String valve01state   = "SAN320YS01SW01/state";
const String valve02command = "SAN320YS02SW01/command";
const String valve02state   = "SAN320YS02SW01/state";


// start webserver
AsyncWebServer server(80);
ESPDash dashboard(&server); 
Card ValveSchuppen(&dashboard, BUTTON_CARD, "PIN 12 - IN 1");
Card ValveIN2(&dashboard, BUTTON_CARD, "PIN 12 - IN 1");
Card temperature(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
Card humidity(&dashboard, HUMIDITY_CARD, "Humidity", "%");
Card card2(&dashboard, PROGRESS_CARD, "Progress1", "", 0, 255);

    //int prog = 20;


// ------------------------------------------------
//  setup
// ------------------------------------------------
//  setup routine at startup to initialize
//  wifi, mqtt connection and io-pins
// ------------------------------------------------



int prog = 0;

void setup() {
    prog = 4;
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);


    //WiFi.begin(wifiSsid, wifiPass);
    WiFi.begin(ssid, pass);
    //WiFi.begin(Ssid, Password);
    
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS 2: ");
  Serial.println(WiFi.dnsIP(1));
 
 server.begin();

delay(5000);


    //try a long time to get wifi working, to avoid an to quick reboot
    for (int i = 0; ( (WiFi.status() != WL_CONNECTED) && (i < 600) ); i++) {
        delay(1000);
    }

    //if the wifi is not working restart the esp
    if (WiFi.status() != WL_CONNECTED) ESP.restart();

    //connect to the mqtt server  
    // Problems with new mosquitto X509 Certs waiting for fix in WiFiClientSecure
    // SSL still aktive, but no server verification possible at this time
    // acceptable risk in controlled homenetwork with wifi encryption  
    //sslWifiClient.setInsecure();
    // sslWifiClient.setCACert(mqttCaCert);

    mqttClient.setServer(mqttServer, mqttPort);

    mqttClient.setCallback(mqttCallbackReceived);

    //setup io-pins
    pinMode(valve01Pin, OUTPUT);
    pinMode(valve02Pin, OUTPUT);


    //make sure all the valves are closed
    digitalWrite(valve01Pin, HIGH);
    digitalWrite(valve02Pin, HIGH);
  
}


// ------------------------------------------------
//  main loop
// ------------------------------------------------
//  mqtt connection and reconnects if needed
//  listens for incoming mqtt data
// ------------------------------------------------

void loop() {
    // ArduinoOTA.setHostname("myesp8266");
   //  ArduinoOTA.Handle();
  /* Send Updates to our Dashboard (realtime) */
  // dashboard.sendUpdates();


  
    //reconnect to mqtt server if connection is offline
    if (!mqttClient.connected()) {
    
        mqttconnect();
    }

    mqttcheckHeartbeat();

    if (mqttClient.connected()) mqttClient.loop(); //listen for trafic - no delays

    temperature.update((int)random(0, 50));
    humidity.update((int)random(0, 100));
    card2.update(prog);
    dashboard.sendUpdates();                
    
    //
    // ACHTUNG
    //
    // Delay wieder rausnehmen, nur zu Testzwecken!
    delay(3000);
    // for (int prog = 0; prog < 200; prog++);
    prog = prog + 1;
    Serial.println(prog);

        ValveSchuppen.attachCallback([&](bool value){
        Serial.println("[Card1] Button Callback Triggered: "+String((value)?"true":"false"));
        if (value == true) {
            Serial.println("True");
            mqttClient.publish("SAN320YS01SW01/command", "ON");
        } else {
            Serial.println("Not true.");
            mqttClient.publish("SAN320YS01SW01/command", "OFF");
        }
        ValveSchuppen.update(value);
        dashboard.sendUpdates();
        });

        ValveIN2.attachCallback([&](bool value){
        Serial.println("[Card2] Button Callback Triggered: "+String((value)?"true":"false"));
        if (value == true) {
            Serial.println("True");
            mqttClient.publish("SAN320YS02SW01/command", "ON");
        } else {
            Serial.println("Not true.");
            mqttClient.publish("SAN320YS02SW01/command", "OFF");
        }
        ValveIN2.update(value);
        dashboard.sendUpdates();
        });
}


// ------------------------------------------------
//  mqttconnect
// ------------------------------------------------
//  connects to the mqtt server and subscribes
//  to the mqtt topics used control the valves
//  and receive the heartbeat.
//  closes all the valves to get an controlled state
//  to work with.
//  reports the valves status back to the server.
// ------------------------------------------------

void mqttconnect() {
    // if (mqttClient.connect("esp32_Bewaesserung", mqttUser, mqttPass)) {
    while (!mqttClient.connected()) {
    //if (mqttClient.connect("esp32_Bewaesserung")) {
    if (mqttClient.connect("esp", NULL, NULL)) {
          Serial.println("Connected to MQTT server");
          //subscrib tp mqtt topics
          mqttClient.subscribe(heartbeat.c_str());
          mqttClient.subscribe(valve01command.c_str());
          mqttClient.subscribe(valve02command.c_str());




          delay(100);
      
          //close the valves
          digitalWrite(valve01Pin, HIGH);
          digitalWrite(valve02Pin, HIGH);

          //tell the server that all the valves are closed
          mqttPayload = "OFF";
          mqttClient.publish(valve01state.c_str(), mqttPayload.c_str());
          mqttClient.publish(valve02state.c_str(), mqttPayload.c_str());

        
    } 
    else { 
      Serial.println("MQTT connection failed \n");
      Serial.print (mqttClient.state());
      delay(2000);
    }
    }
}


// ------------------------------------------------
//  mqttCallbackReceived
// ------------------------------------------------
//  reacts to the incoming mqtt data.
//  checks if the data is an heartbeat
//  opens and closes the valves.
//  sends feedback about the state of changed valve.
// ------------------------------------------------

void mqttCallbackReceived(char* topicByte, byte* payloadByte, unsigned int length) {
  
    mqttIncommingTopic = String((char *)topicByte);
    mqttIncommingPayload = "";
    for (unsigned int i=0; i < length; i++) mqttIncommingPayload += (char)payloadByte[i];

    mqttcheckHeartbeat();
  


  
    if (mqttIncommingTopic == valve01command) {
        if (mqttIncommingPayload == "OFF") {
            Serial.println("\n Command: off");
            digitalWrite(valve01Pin, HIGH);
            mqttPayload = "OFF";
            mqttClient.publish(valve01state.c_str(), mqttPayload.c_str());
        }
        if (mqttIncommingPayload == "ON") {
          Serial.println("\n Command: on");
            digitalWrite(valve01Pin, LOW);
            mqttPayload = "ON";
            mqttClient.publish(valve01state.c_str(), mqttPayload.c_str());
        } 
        } else if (mqttIncommingTopic == valve02command) {
        if (mqttIncommingPayload == "OFF") {
            digitalWrite(valve02Pin, HIGH);
            mqttPayload = "OFF";
            mqttClient.publish(valve02state.c_str(), mqttPayload.c_str());
        }
        if (mqttIncommingPayload == "ON") {
            digitalWrite(valve02Pin, LOW);
            mqttPayload = "ON";
            mqttClient.publish(valve02state.c_str(), mqttPayload.c_str());
        }
    
    }
   
}


// ------------------------------------------------
//  mqttcheckHeartbeat
// ------------------------------------------------
//  checks if incoming mqtt data is an heartbeat.
//  the server hast to send an heartbeat at least
//  every 15 minutes to prove the wifi and mqtt
//  connection is working
// ------------------------------------------------

void mqttcheckHeartbeat() {

    //was the last incoming mqtt topic an heartbeat from the server
    if (mqttIncommingTopic == "SYS000AE01SW01") {

        //reset the 15 minute countdown
        millisLastUpdateHeartbeat = millis();
        //remove the heartbeat from the buffer
        mqttIncommingTopic = "";
      
    }

    //restart esp if the the was no heartbeat detected in the last 15 minutes
    if((unsigned long)(millis() - millisLastUpdateHeartbeat) > 900000) {
        ESP.restart();
    }
      
}


