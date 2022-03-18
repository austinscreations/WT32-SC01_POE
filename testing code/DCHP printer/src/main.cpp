#include <Arduino.h>
#include <WiFi.h> 
#include <SPI.h>
#include <Ethernet.h>
#include <OXRS_MQTT.h>                // For MQTT
#include <OXRS_API.h>                 // For REST API
// #include <EthernetSPI2.h>

/*--------------------------- Version ------------------------------------*/
#define FW_NAME       "OXRS-AC-WT32_SC01_ETH_TEST-ESP32-FW"
#define FW_SHORT_NAME "ETH TEST"
#define FW_MAKER      "Austin's Creations"
#define FW_VERSION    "0.0.1"


#define       ETH_SCLK                  32
#define       ETH_MISO                  27
#define       ETH_MOSI                  33
#define       ETHERNET_CS_PIN           26
#define       WIZNET_RESET_PIN          25
#define       DHCP_TIMEOUT_MS           15000
#define       DHCP_RESPONSE_TIMEOUT_MS  4000

#define       DISPLAY_CS_PIN            15

// Serial
#define       SERIAL_BAUD_RATE          115200

// client && server
EthernetClient client;
EthernetServer server(80);

// MQTT client
PubSubClient mqttClient(client);
OXRS_MQTT mqtt(mqttClient);

// API connection
OXRS_API api(mqtt);


void initialiseEthernet(byte * mac)
{
  // Get ESP32 base MAC address
  WiFi.macAddress(mac);
  
  // Ethernet MAC address is base MAC + 3
  // See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html#mac-address
  mac[5] += 3;

  // Display the MAC address on serial
  char mac_display[18];
  sprintf_P(mac_display, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(F(" mac address: "));
  Serial.println(mac_display);

  // Initialise ethernet library
  Ethernet.init(ETHERNET_CS_PIN);

  // Reset Wiznet W5500
  pinMode(WIZNET_RESET_PIN, OUTPUT);
  digitalWrite(WIZNET_RESET_PIN, HIGH);
  delay(250);
  digitalWrite(WIZNET_RESET_PIN, LOW);
  delay(50);
  digitalWrite(WIZNET_RESET_PIN, HIGH);
  delay(350);

  // Get an IP address via DHCP and display on serial
  Serial.print(F("ip address: "));
  SPI.begin(ETH_SCLK, ETH_MISO, ETH_MOSI, ETHERNET_CS_PIN);
  if (Ethernet.begin(mac, DHCP_TIMEOUT_MS, DHCP_RESPONSE_TIMEOUT_MS))
  {
    Serial.println(Ethernet.localIP());
  }
  else
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
  }
}

/**
  MQTT
*/
void mqttCallback(char * topic, uint8_t * payload, unsigned int length) 
{
  // Pass this message down to our MQTT handler
  mqtt.receive(topic, payload, length);
}


void getFirmwareJson(JsonVariant json)
{
  JsonObject firmware = json.createNestedObject("firmware");

  firmware["name"] = FW_NAME;
  firmware["shortName"] = FW_SHORT_NAME;
  firmware["maker"] = FW_MAKER;
  firmware["version"] = FW_VERSION;
}

void getNetworkJson(JsonVariant json)
{
  byte mac[6];
  WiFi.macAddress(mac);
  
  char mac_display[18];
  sprintf_P(mac_display, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  JsonObject network = json.createNestedObject("network");


  network["ip"] = Ethernet.localIP();

  network["mac"] = mac_display;
}

void getConfigSchemaJson(JsonVariant json)
{
  JsonObject configSchema = json.createNestedObject("configSchema");
  
  // Config schema metadata
  configSchema["$schema"] = "http://json-schema.org/draft-07/schema#";
  configSchema["title"] = FW_NAME;
  configSchema["type"] = "object";



}

void getCommandSchemaJson(JsonVariant json)
{
  JsonObject commandSchema = json.createNestedObject("commandSchema");
  
  // Command schema metadata
  commandSchema["$schema"] = "http://json-schema.org/draft-07/schema#";
  commandSchema["title"] = FW_NAME;
  commandSchema["type"] = "object";


}


/**
  API callbacks
*/
void apiAdopt(JsonVariant json)
{
  // Build device adoption info
  getFirmwareJson(json);
  getNetworkJson(json);
  getConfigSchemaJson(json);
  getCommandSchemaJson(json);
}

/**
  MQTT callbacks
*/
void mqttConnected() 
{
  // Build device adoption info
  // Build device adoption info
  DynamicJsonDocument json(JSON_ADOPT_MAX_SIZE);
  mqtt.publishAdopt(api.getAdopt(json.as<JsonVariant>()));

  // Log the fact we are now connected
  Serial.println("[TLC] mqtt connected");
}

void mqttDisconnected(int state) 
{
  // Log the disconnect reason
  // See https://github.com/knolleary/pubsubclient/blob/2d228f2f862a95846c65a8518c79f48dfc8f188c/src/PubSubClient.h#L44
  switch (state)
  {
    case MQTT_CONNECTION_TIMEOUT:
      Serial.println(F("[TLC] mqtt connection timeout"));
      break;
    case MQTT_CONNECTION_LOST:
      Serial.println(F("[TLC] mqtt connection lost"));
      break;
    case MQTT_CONNECT_FAILED:
      Serial.println(F("[TLC] mqtt connect failed"));
      break;
    case MQTT_DISCONNECTED:
      Serial.println(F("[TLC] mqtt disconnected"));
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      Serial.println(F("[TLC] mqtt bad protocol"));
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      Serial.println(F("[TLC] mqtt bad client id"));
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      Serial.println(F("[TLC] mqtt unavailable"));
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      Serial.println(F("[TLC] mqtt bad credentials"));
      break;      
    case MQTT_CONNECT_UNAUTHORIZED:
      Serial.println(F("[TLC] mqtt unauthorised"));
      break;      
  }
}

void jsonConfig(JsonVariant json) // config payload
{

}

void jsonCommand(JsonVariant json) // do something payloads
{  


  if (json.containsKey("restart") && json["restart"].as<bool>())
  {
    ESP.restart();
  }
}





/*
 MQTT
 */
void initialiseMqtt(byte * mac)
{
  // Set the default client id to the last 3 bytes of the MAC address
  char clientId[32];
  sprintf_P(clientId, PSTR("%02x%02x%02x"), mac[3], mac[4], mac[5]);  
  mqtt.setClientId(clientId);
  
  // Register our callbacks
  mqtt.onConnected(mqttConnected);
  mqtt.onDisconnected(mqttDisconnected);
  mqtt.onConfig(jsonConfig);
  mqtt.onCommand(jsonCommand);  

  // Start listening for MQTT messages
  mqttClient.setCallback(mqttCallback);  
}


/*
 REST API
 */
void initialiseRestApi(void)
{
  // NOTE: this must be called *after* initialising MQTT since that sets
  //       the default client id, which has lower precendence than MQTT
  //       settings stored in file and loaded by the API

  // Set up the REST API
  api.begin();

  // Register our callbacks
  api.onAdopt(apiAdopt);

  server.begin();
}


void setup() {

  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);
  Serial.println();
  Serial.println(F("\n==============================="));
  Serial.println(FW_NAME);
  Serial.print  (F("            v"));
  Serial.println(FW_VERSION);
  Serial.println(F("==============================="));

  pinMode(DISPLAY_CS_PIN, OUTPUT);
  digitalWrite(DISPLAY_CS_PIN, HIGH);

  // SPI.begin (SPI_SCK, SPI_MISO, SPI_MOSI);

  // Set up ethernet and obtain an IP address
  byte mac[6];
  initialiseEthernet(mac);

  initialiseMqtt(mac);   // Set up MQTT (don't attempt to connect yet)

  initialiseRestApi();   // Set up the REST API

}

void loop() {
  Ethernet.maintain();

  EthernetClient client = server.available();
  api.checkEthernet(&client);

}