#ifndef MQTTCONF_H
#define MQTTCONF_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <tools.h>

#ifndef ESP32
#if filesystem==littlefs
    #include <LittleFS.h>
#else
    #include <FS.h>
    #define SPIFFS_USE_MAGIC
#endif
#else
    #include <FS.h>
    #include <SPIFFS.h>
    #define SPIFFS_USE_MAGIC
#endif

#define MQTT_SIZE 256
#define MQTT_FILE_NAME "/mqtt.json"

// global variables for MQTTClient
File mqttFile;
String mqttServer=".";
uint16_t mqttPort=1833;
String mqttPrefix=getESPDevName();
String mqttUser="";
String mqttPass="";

/**********************************************************************************************************
 * Config File Helper Functions
***********************************************************************************************************/
void writeMqttConfig(String server=".",uint16_t port=1883,String prefix=getESPDevName(),String user="",String pass="")
{
  DynamicJsonDocument mdoc(256);

#if defined ESP8266 && filesystem == littlefs
  mqttFile=LittleFS.open(MQTT_FILE_NAME,"w");
#else
  mqttFile=SPIFFS.open(MQTT_FILE_NAME,"w");
#endif

  if(server.length()>1 )
  {
    mdoc["mqttServer"]=server;
    mdoc["mqttPort"]=port;
    mdoc["mqttPrefix"]=prefix;
    mdoc["mqttUser"]=user;
    mdoc["mqttPass"]=pass;
  }
  else
  {
    mdoc["mqttServer"]=".";
    mdoc["mqttPort"]=1883;
    mdoc["mqttPrefix"]=getESPDevName();
    mdoc["mqttUser"]="";
    mdoc["mqttPass"]="";
  }
  
  serializeJson(mdoc,mqttFile);
  mqttFile.flush();
  mqttFile.close();
}

/*************************************************************************************************************************
 * Read MQTT Config from File
 * ***********************************************************************************************************************/
void readMqttConfig()
{
  DynamicJsonDocument mdoc(256);
  Serial.println("Try to load MQTT-Config from file");

#if defined ESP8266 && filesystem == littlefs
  mqttFile=LittleFS.open(MQTT_FILE_NAME,"r");
#else
  mqttFile=SPIFFS.open(MQTT_FILE_NAME,"r");
#endif
  DeserializationError err = deserializeJson(mdoc, mqttFile);
  mqttFile.close();
  if(err)
  {
    Serial.println("Unable to read Config Data (Json Error)");
    Serial.println(err.c_str());
  }
  else
  {
    mqttServer= mdoc["mqttServer"].as<String>();
    mqttPort=mdoc["mqttPort"].as<uint16_t>();
    mqttPrefix=mdoc["mqttPrefix"].as<String>();
    mqttUser=mdoc["mqttUser"].as<String>();
    mqttPass=mdoc["mqttPass"].as<String>();
  }
}

/*********************************************************************************************************************
 * Check Mqtt Config File exists
 * *******************************************************************************************************************/
void checkMqttConfig()
{
  //check Config File is exists, or create one

#if defined ESP8266 && filesystem == littlefs
  if(!LittleFS.exists(MQTT_FILE_NAME))
  {
    Serial.println("Try to create Config File");
    writeMqttConfig("");
  }
#else
  if(!SPIFFS.exists(MQTT_FILE_NAME))
  {
    Serial.println("Try to create Config File");
    writeMqttConfig("");
  }
#endif


}

#endif