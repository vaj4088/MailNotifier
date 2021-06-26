/*
 * Scan_SSID_Asynchronous_Example.cpp
 *
 *  Created on: Jun 23, 2021
 *      Author: vaj4088 (Ian Shef)
 */

#include "Scan_SSID_Asynchronous_Example.h"

#include "ESP8266WiFi.h"

#define BLINK_PERIOD 250
long lastBlinkMillis;
boolean ledState_Scan_SSID_Asynchronous_Example;

#define SCAN_PERIOD 5000
long lastScanMillis_Scan_SSID_Asynchronous_Example;


void setup_Scan_SSID_Asynchronous_Example()
 {
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void loop_Scan_SSID_Asynchronous_Example()
{
  long currentMillis = millis();

  // blink LED
  if (currentMillis - lastBlinkMillis > BLINK_PERIOD)
  {
    digitalWrite(LED_BUILTIN, ledState_Scan_SSID_Asynchronous_Example);
    ledState_Scan_SSID_Asynchronous_Example = !ledState_Scan_SSID_Asynchronous_Example;
    lastBlinkMillis = currentMillis;
  }

  // trigger Wi-Fi network scan
  if (currentMillis - lastScanMillis_Scan_SSID_Asynchronous_Example > SCAN_PERIOD)
  {
    WiFi.scanNetworks(true);
    Serial.print("\nScan start ... ");
    lastScanMillis_Scan_SSID_Asynchronous_Example = currentMillis;
  }

  // print out Wi-Fi network scan result upon completion
  int n = WiFi.scanComplete();
  if(n >= 0)
  {
    Serial.printf("%d network(s) found\n", n);
    for (int i = 0; i < n; i++)
    {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i+1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
    }
    WiFi.scanDelete();
  }
}
