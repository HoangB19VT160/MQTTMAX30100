//MAX30100 ESP8266 WebServer
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);

#include <PubSubClient.h>
#include <WiFiClient.h>

// Update these with values suitable for your network.
const char* mqtt_server = "mqtt-dashboard.com";  // MQTT server address
const char* mqtt_user = "hoang2k1"; // MQTT username
const char* mqtt_password = "hoang2k1"; // MQTT password
const char* mqtt_client_name = "esp8266-client"; // MQTT client name
const char* mqtt_topic_spo2 = "SPO2"; // MQTT topic for SpO2
const char* mqtt_topic_bpm = "BPM"; // MQTT topic for BPM
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);


#define REPORTING_PERIOD_MS     1000
float BPM, SpO2;
/*Put your SSID & Password*/
const char* ssid = "TH TRUEMILK";  // Enter SSID here
const char* password = "hoang001";  //Enter hoangPassword here
PulseOximeter pox;
uint32_t tsLastReport = 0;
ESP8266WebServer server(80);
void setup() {
  Serial.begin(115200);
  pinMode(16, OUTPUT);
  delay(100);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  //connect to your local wi-fi network
  WiFi.begin(ssid, password);
  
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  Serial.print("Initializing pulse oximeter..");

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
      Serial.println("SSD1306 allocation failed");
      for(;;);
    }
    client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
    display.setFont(&FreeSerif9pt7b);
    display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);        
    display.setCursor(20,15);             
    display.println("Welcom to");
    display.setCursor(0,40);             
    display.println("TEAM 12");
    display.display();
    display.setTextSize(1);
    delay(2000); 

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    
  }

  
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    Serial.print("BPM: ");
    Serial.println(BPM);
    Serial.print("SpO2: ");
    Serial.print(SpO2);
    Serial.println("%");
    Serial.println("*********************************");

    display.clearDisplay(); 
    display.setCursor(10,12); 
    display.print("Pulse Oximeter");     
    display.setCursor(0,35); 
    display.print("HeartR:");
    display.setCursor(62,35);
    display.print(BPM,0); 
    display.println(" bpm");
    display.setCursor(0,59);
    display.print("SpO2  : ");
    display.setCursor(62,59);
    display.print(SpO2);
    display.println(" %");
    display.display();
    client.publish(mqtt_topic_spo2, String(SpO2).c_str());
    client.publish(mqtt_topic_bpm, String(BPM).c_str());
    tsLastReport = millis();
    
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client_name, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("esp8266/status", "connected");
      // ... and resubscribe
      client.subscribe("esp8266/receive");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


