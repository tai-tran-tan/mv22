#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://pubsubclient.knolleary.net/
// #include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <Servo.h>
#include "env.h"

Servo ESC;
int speed = 0; //initial speed at 0

void callback(char* topic, byte* payload, unsigned int length) {
  String message = trimMsg(String((char*) payload), length);
  
  Serial.printf("Message arrived [%s] %s\n", topic, message);
  
  processMessage(message);
}

String trimMsg(String msg, uint8_t len) {
  if (msg.length() > len) {
    return msg.substring(0, len);
  }
  return msg;
}

void processMessage(String message) {
  uint8_t speed = message.toInt();
  setSpeed(speed);
}

WiFiClient wifiClient;
PubSubClient client(MQTT_ENDPOINT, MQTT_PORT, callback, wifiClient); //set MQTT port number to 8883 as per //standard

void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  // WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void subscribe(const char* topic) {
  if (client.subscribe(topic)) {
    Serial.print("Subcribed to ");
    Serial.println(topic);
  } else {
    Serial.print("Failed to subcribe to ");
    Serial.println(topic);
  }
}

void publish(const char* topic, const char* payload) {
  if (client.publish(topic, payload)) {
    Serial.print("Published to ");
    Serial.println(topic);
  } else {
    Serial.print("Failed to publish to ");
    Serial.println(topic);
  }
}

void connectMqtt() {
  // if(!wifiClient.connected()) {
  //   Serial.println("Wifi not connected!");
  //   return;
  // }

  Serial.print("Attempting MQTT connection...");
  
  // Attempt to connect
  if (client.connect("mcu-client")) {
    Serial.println("connected");

    // subscribe
    subscribe("speed");

    // Once connected, get current status...
    // publish("$aws/things/aquarium/shadow/get", "");

  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");

    // Wait 5 seconds before retrying
    delay(5000);
  }
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  
  // declare pin 9 to be an output:
  pinMode(D5, OUTPUT);
  ESC.attach(D5,1000,2000);
  
  setup_wifi();
}

void setSpeed(int speed){
  int angle = max(0, min(180, speed)); //Sets servo positions to different speeds
  
  Serial.print("Angle write ");
  Serial.println(angle);
  
  ESC.write(angle); //angle
  
  Serial.print("Angle read ");
  Serial.println(ESC.read());
}

// the loop routine runs over and over again forever:
void loop() {
  if (!client.connected()) {
    connectMqtt();
    return;
  }
  client.loop();

  if (Serial.available() > 0) {
    String str = Serial.readString();
    str.trim();
    speed = str.toInt();
    
    // set the speed of pin 9:
    Serial.print("Speed ");
    Serial.println(speed);
    
    setSpeed(speed);
  }

  // wait for 30 milliseconds to see the dimming effect
  delay(1000);
}
