#ifndef H_MQTT
#define H_MQTT

#define MQTT_TOPIC_TELE    "espsms/tele"
#define MQTT_TOPIC_MESSAGE "espsms/message"

#include <PubSubClient.h>
#include <WiFi.h>

extern WiFiClient wifi;
extern PubSubClient mqtt;

bool initMQTT();
bool sendMessage(String from, String fromName, String date, String message);
bool sendTelemetry(int quality, String regInfo);

#endif
