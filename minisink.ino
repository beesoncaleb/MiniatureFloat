#include <WiFi.h>
#include <WebServer.h>
#include <Stepper.h>
#include <Wire.h>
#include "MS5837.h"

//global initial time
String initial_time;

//global csv string
String dive_csv;

bool handling_request = false;

//global boolean for last sink or float
bool already_sank = false;

//constants for stepper distance
#define CM_PER_REVOLUTION 8
#define MAX_DISTANCE 8
#define STEPS_PER_REVOLUTION 200
#define MAX_STEPS ((MAX_DISTANCE / CM_PER_REVOLUTION) * STEPS_PER_REVOLUTION)

//sensor and motor pins
#define IN1 38
#define IN2 37
#define IN3 36
#define IN4 35
#define SDA_PIN 12
#define SCL_PIN 13

const char* ssid = "MiniSink";
const char* password = "minisink";

//sensor
MS5837 sensor;

//stepper motor
Stepper motor(STEPS_PER_REVOLUTION, IN1, IN2, IN3, IN4);

//configure default IP Address
IPAddress localIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer miniserver(80);

String readData(unsigned long time) {
  String dataline = "";
  dataline += time;
  dataline += ",";

  sensor.read();
  dataline += sensor.depth();
  dataline += ",";
  dataline += sensor.pressure(0.1);
  dataline += ",";
  dataline += sensor.temperature();
  dataline += "\n";

  Serial.println(dataline);

  return dataline;
}
void toggle_sinkfloat(bool sink) {
  if (sink) {
    if (!already_sank) {
      motor.step(-MAX_STEPS);
      already_sank = true;
    }
  }
  else {
    if (already_sank) {
      motor.step(MAX_STEPS);
      already_sank = false;
    }
  }
}

//request handlers
void handleRoot() {
  sensor.read();
  String initial_comm = "Team Name: Minisink\nInitial Time: ";
  initial_comm += initial_time;
  initial_comm += "\nInitial Pressure: ";
  initial_comm += sensor.pressure(100);
  initial_comm += " Pa\nInitial Depth: ";
  initial_comm += sensor.depth();
  initial_comm += " m";
  miniserver.send(200, "text/plain", initial_comm);
}
void handleNotFound() {
  miniserver.send(404, "text/plain", "Resource not found");
}
void gettime() {
  initial_time = miniserver.arg("plain");
  miniserver.send(200, "text/plain", "Time set");
}
void profile1() {
  miniserver.send(200, "text/plain", "Profile 1 initiated");

  if (handling_request) {
    return;
  }
  handling_request = true;

  //sink to 9 feet then resurface
  dive_csv = "time,depth,pressure,temperature\n";
  toggle_sinkfloat(true);
  dive_csv += readData(0);
  unsigned long start_time = millis();
  unsigned long current_time;
  while (sensor.depth() < 2.4) {
    delay(1000);
    current_time = (millis() - start_time) / 1000;
    if (current_time % 5 == 0) {
      dive_csv += readData(current_time);
    }
    miniserver.handleClient();  //check for reset request
  }
  toggle_sinkfloat(false);

  while (sensor.depth() > 0.2) {
    delay(1000);
    current_time = (millis() - start_time) / 1000;
    if (current_time % 5 == 0) {
      dive_csv += readData(current_time);
    }
  }

  miniserver.handleClient();  //check for reset request
  handling_request = false;
}
void profile2() {
  miniserver.send(200, "text/plain", "Profile 2 initiated");

  if (handling_request) {
    return;
  }
  handling_request = true;

  //sink to 4m then resurface
  dive_csv = "time,depth,pressure,temperature\n";
  toggle_sinkfloat(true);
  dive_csv += readData(0);
  unsigned long start_time = millis();
  unsigned long current_time;
  while (sensor.depth() < 3.5) {
    delay(1000);
    current_time = (millis() - start_time) / 1000;
    if (current_time % 5 == 0) {
      dive_csv += readData(current_time);
    }

    miniserver.handleClient();  //check for reset request
  }
  toggle_sinkfloat(false);
  while (sensor.depth() > 0.2) {
    delay(1000);
    current_time = (millis() - start_time) / 1000;
    if (current_time % 5 == 0) {
      dive_csv += readData(current_time);
    }

    miniserver.handleClient();  //check for reset request
  }

  //sink to 2m then resurface
  toggle_sinkfloat(true);
  while (sensor.depth() < 1.5) {
    delay(1000);
    current_time = (millis() - start_time) / 1000;
    if (current_time % 5 == 0) {
      dive_csv += readData(current_time);
    }

    miniserver.handleClient();  //check for reset request
  }
  toggle_sinkfloat(false);
  while (sensor.depth() > 0.2) {
    delay(1000);
    current_time = (millis() - start_time) / 1000;
    if (current_time % 5 == 0) {
      dive_csv += readData(current_time);
    }

    miniserver.handleClient();  //check for reset request
  }

  handling_request = false;
}
void plot() {
  miniserver.send(200, "text/csv", dive_csv);
}
void resetesp() {
  if (already_sank) {
    toggle_sinkfloat(false);
    delay(100);
  }
  miniserver.send(200, "text/plain", "Resetting ESP32");
  delay(100);
  Serial.println("\nResetting ESP32");
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  //setup sensor
  Wire.begin(SDA_PIN, SCL_PIN);
  while(!sensor.init()) {
    Serial.println("Sensor didn't initiate\n");
    delay(5000);
  }
  sensor.setFluidDensity(997);
  Serial.println("Sensor initialized\n");

  //setup motor
  motor.setSpeed(60);

  //setup wifi network
  Serial.println("Setting up Access Point...");
  if (!WiFi.softAPConfig(localIP, gateway, subnet)) {
    log_e("Static IP config failed.");
    while (1);
  }
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //assign request handlers
  miniserver.on("/", handleRoot);
  miniserver.onNotFound(handleNotFound);
  miniserver.on("/reset", resetesp);
  miniserver.on("/time", HTTP_POST, gettime);
  miniserver.on("/profile1", profile1);
  miniserver.on("/profile2", profile2);
  miniserver.on("/plot", plot);
  miniserver.begin();
  Serial.println("Server started\n\n"); 
}

void loop() {
  miniserver.handleClient();
  delay(2);
}