#ifndef H_MAIN
#define H_MAIN

#include "timer.h"

bool commandRunning;
Timer netCheck;
int quality;
String regInfo;

void checkNet();
bool expectString(String expect);
void initModem();
void initWiFi();
String readLine();
String readLine(int timeout);
bool sendBoolCommand(String command);
String sendCommand(String command, bool readOK);
String trimQuotes(String in);
void updateSerial();

#endif
