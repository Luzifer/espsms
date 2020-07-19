#include <ArduinoJson.h>

#include "mqtt.hpp"

WiFiClient wifi;
PubSubClient mqtt;

bool initMQTT() {
  Serial.println("Connecting to MQTT...");

  mqtt.setClient(wifi);
  mqtt.setServer(MQTT_HOST, 1883);
  mqtt.setBufferSize(1024);

  return mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
}

bool sendMessage(String from, String fromName, String date, String message) {
  Serial.println("DBG: Sending message to MQTT...");

  if (mqtt.state() != 0) {
    Serial.println("ERR: Invalid MQTT state " + String(mqtt.state(), DEC));
    return false;
  }

  StaticJsonDocument<400> doc;
  doc["from"] = from;
  doc["fromName"] = fromName;
  doc["date"] = date;
  doc["message"] = message;

  char buffer[400];
  serializeJson(doc, buffer);

  return mqtt.publish(MQTT_TOPIC_MESSAGE, buffer);
}

bool sendTelemetry(int quality, String regInfo) {
  Serial.println("DBG: Sending telemetry to MQTT...");

  if (mqtt.state() != 0) {
    Serial.println("ERR: Invalid MQTT state " + String(mqtt.state(), DEC));
    return false;
  }

  StaticJsonDocument<100> doc;
  doc["quality"] = quality;
  doc["reg"] = regInfo;

  char buffer[100];
  serializeJson(doc, buffer);

  return mqtt.publish(MQTT_TOPIC_TELE, buffer);
}

