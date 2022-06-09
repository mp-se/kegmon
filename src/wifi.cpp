/*
MIT License

Copyright (c) 2021-22 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <incbin.h>

#include <config.hpp>
#include <main.hpp>
#include <wifi.hpp>

// Settings for DRD
#define ESP_DRD_USE_LITTLEFS true
#define ESP_DRD_USE_SPIFFS false
#define ESP_DRD_USE_EEPROM false
#include <ESP_DoubleResetDetector.h>
#define DRD_TIMEOUT 3
#define DRD_ADDRESS 0

// Settings for WIFI Manager
#define USE_ESP_WIFIMANAGER_NTP false
#define USE_CLOUDFLARE_NTP false
#define USING_CORS_FEATURE false
#define NUM_WIFI_CREDENTIALS 1
#define USE_STATIC_IP_CONFIG_IN_CP false
#define _WIFIMGR_LOGLEVEL_ 3
#include <ESP_WiFiManager.h>
ESP_WiFiManager *myWifiManager;
DoubleResetDetector *myDRD;
WifiConnection myWifi;

const char *userSSID = USER_SSID;
const char *userPWD = USER_SSID_PWD;

//
// Initialize
//
void WifiConnection::init() {
  myDRD = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
}

//
// Check if we have a valid wifi configuration
//
bool WifiConnection::hasConfig() {
  if (strlen(myConfig.getWifiSSID(0))) return true;
  if (strlen(userSSID)) return true;

  // Check if there are stored WIFI Settings we can use.
  String ssid = WiFi.SSID();
  if (ssid.length()) {
    Log.notice(F("WIFI: Found credentials in EEPORM." CR));
    myConfig.setWifiSSID(ssid, 0);

    if (WiFi.psk().length()) myConfig.setWifiPass(WiFi.psk(), 0);

    myConfig.saveFile();
    return true;
  }
  return false;
}

//
// Check if the wifi is connected
//
bool WifiConnection::isConnected() { return WiFi.status() == WL_CONNECTED; }

//
// Get the IP adress
//
String WifiConnection::getIPAddress() { return WiFi.localIP().toString(); }

//
// Additional method to detect double reset.
//
bool WifiConnection::isDoubleResetDetected() {
  if (strlen(userSSID))
    return false;  // Ignore this if we have hardcoded settings.
  return myDRD->detectDoubleReset();
}

//
// Stop double reset detection
//
void WifiConnection::stopDoubleReset() { myDRD->stop(); }

//
// Start the wifi manager
//
void WifiConnection::startPortal() {
  Log.notice(F("WIFI: Starting Wifi config portal." CR));

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  myWifiManager = new ESP_WiFiManager(WIFI_MDNS);
  myWifiManager->setMinimumSignalQuality(-1);
  myWifiManager->setConfigPortalChannel(0);
  myWifiManager->setConfigPortalTimeout(myConfig.getWifiPortalTimeout());

  String mdns("<p>Default mDNS name is: http://");
  mdns += myConfig.getMDNS();
  mdns += ".local<p>";
  ESP_WMParameter deviceName(mdns.c_str());
  myWifiManager->addParameter(&deviceName);

  myWifiManager->startConfigPortal(WIFI_DEFAULT_SSID, WIFI_DEFAULT_PWD);

  if (myWifiManager->getSSID(0).length()) {
    myConfig.setWifiSSID(myWifiManager->getSSID(0), 0);
    myConfig.setWifiPass(myWifiManager->getPW(0), 0);
    myConfig.setWifiSSID(myWifiManager->getSSID(1), 1);
    myConfig.setWifiPass(myWifiManager->getPW(1), 1);

    // If the same SSID has been used, lets delete the second
    if (!strcmp(myConfig.getWifiSSID(0), myConfig.getWifiSSID(1))) {
      myConfig.setWifiSSID("", 1);
      myConfig.setWifiPass("", 1);
    }

    Log.notice(F("WIFI: Stored SSID1:'%s' SSID2:'%s'" CR),
               myConfig.getWifiSSID(0), myConfig.getWifiSSID(1));
    myConfig.saveFile();
  } else {
    Log.notice(
        F("WIFI: Could not find first SSID so assuming we got a timeout." CR));
  }

  Log.notice(F("WIFI: Exited wifi config portal. Rebooting..." CR));
  stopDoubleReset();
  delay(500);
  ESP_RESET();
}

//
// Call the wifi manager in loop
//
void WifiConnection::loop() { myDRD->loop(); }

//
// Connect to last known access point, non blocking mode.
//
void WifiConnection::connectAsync(int wifiIndex) {
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  if (strlen(userSSID)) {
    Log.notice(F("WIFI: Connecting to wifi using hardcoded settings %s." CR),
               userSSID);
    WiFi.begin(userSSID, userPWD);
  } else {
    Log.notice(F("WIFI: Connecting to wifi (%d) using stored settings %s." CR),
               wifiIndex, myConfig.getWifiSSID(wifiIndex));
    WiFi.begin(myConfig.getWifiSSID(wifiIndex),
               myConfig.getWifiPass(wifiIndex));
  }
}

//
// Blocks until wifi connection has been found
//
bool WifiConnection::waitForConnection(int maxTime) {
#if DEBUG_LEVEL == 6
  WiFi.printDiag(Serial);
#endif
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);

    if (i % 10) Serial.print(".");

    if (i++ > (maxTime * 10)) {
      Log.error(F("WIFI: Failed to connect to wifi %d" CR), WiFi.status());
      WiFi.disconnect();
      Serial.print(CR);
      return false;  // Return to main that we have failed to connect.
    }
  }
  Serial.print(CR);
  Log.notice(F("WIFI: Connected to wifi %s ip=%s." CR), WiFi.SSID().c_str(),
             getIPAddress().c_str());
  Log.notice(F("WIFI: Using mDNS name %s." CR), myConfig.getMDNS());
  return true;
}

//
// Connect to last known access point, blocking mode.
//
bool WifiConnection::connect() {
  // If success to seconday is successful this is used as standard
  int timeout = myConfig.getWifiConnectionTimeout();

  connectAsync(0);
  if (!waitForConnection(timeout)) {
    Log.warning(F("WIFI: Failed to connect to first SSID %s." CR),
                myConfig.getWifiSSID(0));

    if (strlen(myConfig.getWifiSSID(1))) {
      connectAsync(1);

      if (waitForConnection(timeout)) {
        Log.notice(F("WIFI: Connected to second SSID %s." CR),
                   myConfig.getWifiSSID(1));
        return true;
      }
    }

    Log.warning(F("WIFI: Failed to connect to any SSID." CR));
    return false;
  }

  return true;
}

//
// This will erase the stored credentials and forcing the WIFI manager to AP
// mode.
//
bool WifiConnection::disconnect() {
  Log.notice(F("WIFI: Erasing stored WIFI credentials." CR));
  // Erase WIFI credentials
  return WiFi.disconnect(true);
}

// EOF
