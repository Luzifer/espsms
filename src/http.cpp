#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "http.hpp"

HTTPClient http;

bool sendMessage(String from, String fromName, String date, String message) {
  Serial.println("DBG: Sending message to HTTP endpoint...");

  StaticJsonDocument<400> doc;
  doc["from"] = from;
  doc["fromName"] = fromName;
  doc["date"] = date;
  doc["message"] = message;

  char buffer[400];
  serializeJson(doc, buffer);

  http.begin(HTTP_POST_URL);
  http.addHeader("Content-Type", "application/json");

  int respCode = http.POST(buffer);

  if (respCode > 299) {
    Serial.println("ERR: HTTP Post failed: " + http.getString());
  }

  http.end();

  return respCode >= 200 && respCode < 300;
}
