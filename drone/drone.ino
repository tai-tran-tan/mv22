#include <ESP8266WiFi.h>
#include <PubSubClient.h> //mqtt https://pubsubclient.knolleary.net/
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include "env.h"
#include <Wire.h>
#include <math.h>
// #include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

const char* TOPIC_SPEED_LEFT = "speed_left";
const char* TOPIC_SPEED_LEFT_REPORT = "speed_left_report";
const char* TOPIC_SPEED_RIGHT = "speed_right";
const char* TOPIC_SPEED_RIGHT_REPORT = "speed_right_report";
const char* TOPIC_THRUST = "thrust";
const char* TOPIC_THRUST_REPORT = "thrust_report";

uint total_thrust = 0;

Adafruit_MPU6050 mpu;
Servo motor_left;
Servo motor_right;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // pending for serial init
  Serial.println("Starting up...");
  
  initWifi();
  connectMqtt();
  initMotors();
  initMpu6050();
  // setupInterrupt();
}

// void IRAM_ATTR 
void detectsMovement() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  sensors_vec_t acc = a.acceleration;

  float delta = (acc.x * 10 * total_thrust) / 45;
  int left = total_thrust + (delta * 2);
  int right = total_thrust - delta;

  setSpeedLeft(left);
  setSpeedRight(right);

  char msg[] = "Thrust %u Adj %f | X %f Y %f Z %f | left %d right %d\n";
 
  Serial.printf(msg, total_thrust, delta, acc.x, acc.y, acc.z, left, right);
  // publish(TOPIC_THRUST_REPORT, msg);
}

const int itr_pin = D7;

void setupInterrupt() {
  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(itr_pin, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(itr_pin), detectsMovement, RISING);
}

void initMotors() {
  // declare pin 9 to be an output:
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);

  motor_left.attach(D5);
  motor_right.attach(D6);

  setSpeedLeft(0); // Initial speed must be 0
  setSpeedRight(0);
}

void initMpu6050() {
  Serial.println("Setting up MPU6050");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  
  mpu.setMotionInterrupt(true);
  mpu.setInterruptPinLatch(true);
  mpu.setInterruptPinPolarity(true);

  Serial.println("MPU initialized!");
  delay(100);
}

void setThrust(uint t) {
  total_thrust = t;
}

String trimMsg(String msg, uint8_t len) {
  if (msg.length() > len) return msg.substring(0, len);
  return msg;
}

void initWifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = trimMsg(String((char*)payload), length);

  // Serial.printf("Message arrived [%s] %s\n", topic, message);

  if (strcmp(topic, TOPIC_SPEED_LEFT) == 0) {
    uint speed = message.toInt();
    setSpeed(motor_left, speed);
  } else if (strcmp(topic, TOPIC_SPEED_RIGHT) == 0) {
    uint speed = message.toInt();
    setSpeed(motor_right, speed);
  } else if (strcmp(topic, TOPIC_THRUST) == 0) {
    uint thrust = message.toInt();
    setThrust(thrust);
  } else {
    Serial.println("Unknown topic, ignore message.");
  }
}

WiFiClient wifiClient;
PubSubClient client(MQTT_ENDPOINT, MQTT_PORT, mqttCallback, wifiClient);

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
    // Serial.print("Published to ");
    // Serial.println(topic);
  } else {
    Serial.print("Failed to publish to ");
    Serial.println(topic);
  }
}

void connectMqtt() {
  Serial.print("Attempting MQTT connection...");
  // Serial.printf("Connecting to %s:%s\n", MQTT_ENDPOINT, MQTT_PORT);

  // Attempt to connect
  if (client.connect("esp8266-client")) {
    Serial.println("connected");

    // subscribe
    subscribe(TOPIC_SPEED_LEFT);
    subscribe(TOPIC_SPEED_RIGHT);
    subscribe(TOPIC_THRUST);

    // Once connected, get current status...
    // publish("$aws/things/aquarium/shadow/get", "");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");

    // Wait 5 seconds before retrying
  }
  while (!client.connected()) {
    Serial.println("Waiting for MQTT...");
    delay(1000);
  }
}

void setSpeedLeft(int s) {
  int speed = setSpeed(motor_left, s);
  char str[3];
  itoa(speed, str, 10);
  publish(TOPIC_SPEED_LEFT_REPORT, str);
}

void setSpeedRight(int s) {
  int speed = setSpeed(motor_right, s);
  char str[3];
  itoa(speed, str, 10);
  publish(TOPIC_SPEED_RIGHT_REPORT, str);
}

int setSpeed(Servo& motor, int angle) {
  // Serial.print("Angle write ");
  // Serial.println(angle);

  motor.write(abs(angle)); // angle

  // Serial.print("Angle read ");
  const int& speed = motor.read();
  return speed;
}

// the loop routine runs over and over again forever:
void loop() {
  client.loop();
  detectsMovement();

  // delay(10);
  // uint input = digitalRead(A0);
  // Serial.println(input);
}
