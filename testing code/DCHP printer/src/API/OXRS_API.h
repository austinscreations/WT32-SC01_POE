/*
 * OXRS_API.h
 */

#ifndef OXRS_API_H
#define OXRS_API_H

#include <OXRS_MQTT.h>
#include <ArduinoJson.h>
#include <aWOT.h>
#include <Ethernet.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#else
#include <WiFi.h>
#include <SPIFFS.h>
#include <Update.h>
#endif

// JSON Schema Version
#define JSON_SCHEMA_VERSION   "http://json-schema.org/draft-07/schema#"

// JSON payload maximum sizes
#define JSON_MQTT_MAX_SIZE    2048
#define JSON_CONFIG_MAX_SIZE  16384
#define JSON_COMMAND_MAX_SIZE 16384
#define JSON_ADOPT_MAX_SIZE   4096

class OXRS_API
{
  public:
    OXRS_API(OXRS_MQTT& mqtt);

    void begin(void);
    
    void checkEthernet(EthernetClient * client);
    void checkWifi(WiFiClient * client);

    void onAdopt(jsonCallback);
    JsonVariant getAdopt(JsonVariant json);

  private:
    Application _app;
    Router _api;

    void _initialiseRestApi(void);
    void _checkRestart(void);
};

#endif
