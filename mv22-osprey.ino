#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://pubsubclient.knolleary.net/
// #include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <Servo.h>
#include "env.h"

// ESP32 Guide: https://RandomNerdTutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
// ESP8266 Guide: https://RandomNerdTutorials.com/esp8266-nodemcu-mpu-6050-accelerometer-gyroscope-arduino/
// Arduino Guide: https://RandomNerdTutorials.com/arduino-mpu-6050-accelerometer-gyroscope/

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
Servo motorA;
Servo motorB;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  initWifi();
  initMpu6050();
  initMotors();
}

void initMotors() {
  // declare pin 9 to be an output:
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  motorA.attach(D5,1000,2000);
  motorB.attach(D6,1000,2000);
  setSpeed(motorA, 0);// Initial speed must be 0
  setSpeed(motorB, 0);
}

void initMpu6050() {
  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = trimMsg(String((char*) payload), length);
  
  Serial.printf("Message arrived [%s] %s\n", topic, message);
  
  processMessage(message);
}

WiFiClient wifiClient;
PubSubClient client(MQTT_ENDPOINT, MQTT_PORT, callback, wifiClient); 

String trimMsg(String msg, uint8_t len) {
  if (msg.length() > len) {
    return msg.substring(0, len);
  }
  return msg;
}

void processMessage(String message) {
  uint8_t speed = message.toInt();
  setSpeed(motorA, speed);
  setSpeed(motorB, speed);
}

void initWifi() {
  delay(100);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

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
  Serial.print("Attempting MQTT connection...");
  //Serial.printf("Connecting to %s:%s\n", MQTT_ENDPOINT, MQTT_PORT);

  // Attempt to connect
  if (client.connect("esp8266-client")) {
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

void setSpeed(Servo& motor, int s){
  int angle = s; //max(0, min(180, speed)); //Sets servo positions to different speeds
  
  Serial.print("Angle write ");
  Serial.println(angle);
  
  motor.write(angle); //angle
  
  Serial.print("Angle read ");
  Serial.println(motor.read());
}

// the loop routine runs over and over again forever:
void loop() {
  if (!client.connected()) {
    connectMqtt();
    return;
  }
  client.loop();

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");

  // wait for 30 milliseconds to see the dimming effect
  delay(1000);
}
