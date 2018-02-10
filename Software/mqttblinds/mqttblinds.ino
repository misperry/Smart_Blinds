#include <Servo.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <PubSubClient.h>


/************ WIFI and MQTT INFORMATION (CHANGE THESE FOR YOUR SETUP) ******************/
#define wifi_ssid "" //enter your WIFI SSID
#define wifi_password "" //enter your WIFI Password

#define mqtt_server "" // Enter your MQTT server adderss or IP. I use my DuckDNS adddress (yourname.duckdns.org) in this field
#define mqtt_user "" //enter your MQTT username
#define mqtt_password "" //enter your password








WiFiClient espClient;
PubSubClient client(espClient);

Servo myservo;

int val;
int itsatrap = 0;
int toggle = 1;
int pull_pin = 0;


void setup() {
  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, 1883); //CHANGE PORT HERE IF NEEDED
  client.setCallback(callback);

  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  pinMode(pull_pin, INPUT);

}


void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    String mytopic(topic);
    Serial.println(mytopic);
    if (itsatrap == 0 && mytopic == "blind/br/command" && message.equals("ON")){  
      Serial.println("made it to ON");
      myservo.attach(2);
      delay(500);
      myservo.write(0); 
      client.publish("blind/br/state", "ON");
      delay(1000);
      myservo.detach();
      }
    else if (mytopic == "blind/br/command" && message.equalsIgnoreCase("OFF")){
      Serial.println("Made it to OFF");
      myservo.attach(2);
      delay(500);
      myservo.write(90);  
      client.publish("blind/br/state", "OFF");
      delay(1000);
      myservo.detach();
    }
    else if (mytopic == "blind/br/level"){
      Serial.println("Made it to DIMM");
      myservo.attach(2);
      delay(500);
      val = message.toInt(); //converts command to integer to be used for positional arrangement
      val = map (val, 0, 99, 0, 90);
      val = (90 - val);
      myservo.write(val);
      Serial.println(val);
      client.publish("blind/br/state", "ON");
      delay(1000);
      myservo.detach();
      itsatrap = 1;
    }
    else{
        itsatrap = 0;
    }

}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  
  if (digitalRead(pull_pin) != toggle)
    { 
      toggle = digitalRead(pull_pin);
      if (!toggle)
      {
        Serial.println("Made it to Pull ON");
        myservo.attach(2);
        delay(500);
        myservo.write(25);
        Serial.println(25);
        client.publish("blind/br/state", "ON");
        delay(1000);
        myservo.detach();
        itsatrap = 1;
      }
      else
      {
        Serial.println("Made it to pull OFF");
        myservo.attach(2);
        delay(500);
        myservo.write(90);  
        client.publish("blind/br/state", "OFF");
        delay(1000);
        myservo.detach();
      }
      
    }
  client.loop();
}


void reconnect() {
  // Loop until we're reconnected
    while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
  if (client.connect("ESPBlindstl", mqtt_user, mqtt_password)) {
      Serial.println("connected");

      client.subscribe("blind/br/command");
      client.subscribe("blind/br/level");
      client.publish("blind/br/command", "OFF");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



