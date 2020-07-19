#include <Arduino.h>
#include <WiFi.h>

#include "http.hpp"
#include "main.hpp"
#include "modem.hpp"
#include "timer.h"
#include "timerManager.h"

// Debug output: Mirror all Modem output to Serial
#define MODEM_DEBUG

// Set serial for AT commands (to SIM800 module)
#define SerialAT  Serial1

void checkNet() {
  String csq = sendCommand("AT+CSQ", true);
  quality = csq.substring(6, csq.indexOf(",", 6)).toInt();
  Serial.println("INF: Quality (0-31): " + String(quality, DEC));

  String creg = sendCommand("AT+CREG?", true);
  regInfo = creg.substring(7);
  Serial.println("INF: Registration: " + regInfo);
}

bool expectString(String expect) { return readLine() == expect; }

void initModem() {
  // Set SerialAT reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  Serial.println("Initializing modem in 4s...");
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  readLine(4000);

  if (!sendBoolCommand("AT")) {
    Serial.println("Modem initializing not successful");
    return initModem();
  }

  Serial.println("Waiting for PIN ready...");

  while (!expectString("+CPIN: READY")) {
    delay(10);
  }

  Serial.println("Card ready, sending config...");

  if (!sendBoolCommand("AT+CMEE=2")) {
    Serial.println("ERR: Unable to set error logging to verbose (AT+CMEE=2)");
  }

  if (!sendBoolCommand("AT+CMGF=1")) {
    Serial.println("ERR: Unable to set text mode (AT+CMGF=1)");
  }

  if (!sendBoolCommand("AT+CSCS=\"8859-1\"")) {
    Serial.println("ERR: Unable to set charset (AT+CSCS=\"8859-1\")");
  }

  if (!sendBoolCommand("AT+CNMI=1,2,0,0,0")) {
    Serial.println("ERR: Unable to subscribe for new SMS (AT+CNMI=1,2,0,0,0)");
  }

  netCheck.setInterval(30000);
  netCheck.setCallback(checkNet);

  TimerManager::instance().start();
}

void initWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.setHostname(HOSTNAME);
  WiFi.begin(WIFI_NAME, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();
}

String iso88591ToUTF8(String in) {
  String out = "";

  for (int i = 0; i < in.length(); i++) {
    char ch = in.charAt(i);
    if (ch < 0x80) {
      out += ch;
    } else {
      out += (char)(0xc0 | (ch & 0xc0) >> 6);
      out += (char)(0x80 | (ch & 0x3f));
    }
  }

  return out;
}

void loop() {
  while (commandRunning) {
    delay(10);
  }

  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(MODEM_POWER_ON, LOW);
    esp_restart();
    return;
  }

  String line = readLine(1000);

  // Some error was received
  if (line.startsWith("+CME ERROR")) {
    Serial.println("ERR: " + line);
  }

  // DBG: Received +CMT: "+49157***","","20/07/19,12:57:45+08"
  // DBG: Received Sehr lange SMS um zu demonstrieren wie sich der Kram verh�lt wenn sowohl Sonderzeichen als auch viel Text in der Nachricht stecken
  if (line.startsWith("+CMT: ")) {
    String message = readLine();

    int pos = 6;
    int npos = line.indexOf(",", pos);
    String senderNo = line.substring(pos, npos);

    pos = npos + 1;
    npos = line.indexOf(",", pos);
    String senderName = line.substring(pos, npos);

    pos = npos + 1;
    String date = line.substring(pos);

    if (!sendMessage(trimQuotes(senderNo), trimQuotes(senderName), trimQuotes(date), iso88591ToUTF8(message))) {
      Serial.println("ERR: Unable to submit message");
    }
  }

  TimerManager::instance().update();
}

String readLine() {
  return readLine(0);
}

String readLine(int timeout) {
  String response = "";
  int start = millis();

  while(true)  {
    while (!SerialAT.available()) {
      delay(10);

      if (timeout > 0 && millis() > start + timeout) {
        return "";
      }
    }

    byte ch = SerialAT.read();

    // Drop invalid chars and CRs
    if (ch == 0 || ch == 13 || ch == 255) {
      continue;
    }

    // If we have a LF and a string, return it!
    if (ch == 10 && response != "") {
      break;
    } else if (ch == 10 && response == "") {
      // Silently drop CR and therefore line
    } else {
      response += (char)ch;
    }
  }

#ifdef MODEM_DEBUG
  Serial.println("DBG: <= " + response);
#endif

  return response;
}

bool sendBoolCommand(String command) {
  return sendCommand(command, false) == "OK";
}

String sendCommand(String command, bool readOK) {
  while (commandRunning) {
    delay(10);
  }

  Serial.println("DBG: => " + command);

  commandRunning = true;
  SerialAT.println(command);
  if (!expectString(command)) {
    commandRunning = false;
    return "ERR Command does not match";
  }
  String resp = readLine();
  if (readOK) {
    readLine();
  }
  commandRunning = false;
  return resp;
}

void setup() {
  Serial.begin(9600);
  delay(2000); // Give terminal chance to connect
  initWiFi();
  initModem();
}

String trimQuotes(String in) {
  while(in.startsWith("\"")) {
    in = in.substring(1);
  }
  while(in.endsWith("\"")) {
    in = in.substring(0, in.length() - 1);
  }

  return in;
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    SerialAT.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(SerialAT.available()) {
    Serial.write(SerialAT.read());//Forward what Software Serial received to Serial Port
  }
}
