/*
 * Scan_SSID_Synchronous_Example.cpp
 *
 *  Created on: Jun 23, 2021
 *      Author: vaj4088  (Ian Shef)
 */

#include "Scan_SSID_Synchronous_Example.h"

#include "ESP8266WiFi.h"

void setup_Scan_SSID_Synchronous_Example()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void loop_Scan_SSID_Synchronous_Example() {
	String ssid ;
  Serial.print("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" network(s) found");
  for (uint8_t i = 0; i < n; i++)
  {
	  ssid = WiFi.SSID() ;
	  ssid = WiFi.SSID(i) ;
    Serial.println(ssid);
  }
  Serial.println();

  delay(5000);
}


