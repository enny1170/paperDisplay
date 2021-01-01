// Generic tools for Config Management
#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>
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
// Init Filesystem
// in Case of ESP8266 we have to init liitlefs because SPIFFS is deprecated
// You must add board_build.filesystem = littlefs and board_build.ldscript = eagle.flash.4m3m.ld to your platform.ini
// in case of ESP32 no special flags needed SPIFFS will be initialized 
void initFileSystem()
{
#if defined ESP8266 && filesystem == littlefs
    Serial.println("Mounting Flash...");
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount file system. Format it");
        if(!LittleFS.format())
        {
            Serial.println("Failed to format file system");
        }
        if(!LittleFS.begin())
        {
            Serial.println("Failed to mount file system after format");
        }
        return;
    }
#else
    Serial.println("Mounting SPIFFS...");
    if (!SPIFFS.begin())
    {
        Serial.println("Failed to mount file system. Format it.");
        if(!SPIFFS.format())
        {
          Serial.println("Failedto format file system");
        }
        if(!SPIFFS.begin())
        {
            Serial.println("Failed to mount file system after format");
        }
        return;
    }
#endif
}

String getESPDevName()
{
  char devName[30];
  #ifdef ESP8266
  snprintf(devName,30,"ESP-%08X",ESP.getChipId());
  #else
  uint32_t chipId=0;
  for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
  snprintf(devName,30,"ESP-%08X",chipId);
  #endif
  return (String)devName;
}



#ifdef ESP8266
/***************************************************************************************************************************
 * Returns the reason for the last boot
 *  REASON_DEFAULT_RST      = 0,     normal startup by power on 
 *  REASON_WDT_RST          = 1,     hardware watch dog reset 
 *  REASON_EXCEPTION_RST    = 2,     exception reset, GPIO status won’t change 
 *  REASON_SOFT_WDT_RST     = 3,     software watch dog reset, GPIO status won’t change 
 *  REASON_SOFT_RESTART     = 4,     software restart ,system_restart , GPIO status won’t change 
 *  REASON_DEEP_SLEEP_AWAKE = 5,     wake up from deep-sleep 
 *  REASON_EXT_SYS_RST      = 6      external system reset 
 * *************************************************************************************************************************/
rst_reason getResetInfoReason()
{
    uint32_t reason=0;
    rst_info *resetInfo=ESP.getResetInfoPtr();
    reason=resetInfo->reason;
    return (rst_reason)reason;
}
#endif 

#ifdef ESP32
# include <rom/rtc.h>

/******************************************************************************************************************************
 * Returns the Reason for the last Reboot
 *  NO_MEAN                =  0,
 *  POWERON_RESET          =  1,    <1, Vbat power on reset*
 *  SW_RESET               =  3,    <3, Software reset digital core
 *  OWDT_RESET             =  4,    <4, Legacy watch dog reset digital core
 *  DEEPSLEEP_RESET        =  5,    <3, Deep Sleep reset digital core
 *  SDIO_RESET             =  6,    <6, Reset by SLC module, reset digital core
 *  TG0WDT_SYS_RESET       =  7,    <7, Timer Group0 Watch dog reset digital core
 *  TG1WDT_SYS_RESET       =  8,    <8, Timer Group1 Watch dog reset digital core
 *  RTCWDT_SYS_RESET       =  9,    <9, RTC Watch dog Reset digital core
 *  INTRUSION_RESET        = 10,    <10, Instrusion tested to reset CPU
 *  TGWDT_CPU_RESET        = 11,    <11, Time Group reset CPU
 *  SW_CPU_RESET           = 12,    <12, Software reset CPU
 *  RTCWDT_CPU_RESET       = 13,    <13, RTC Watch dog Reset CPU
 *  EXT_CPU_RESET          = 14,    <14, for APP CPU, reseted by PRO CPU
 *  RTCWDT_BROWN_OUT_RESET = 15,    <15, Reset when the vdd voltage is not stable
 *  RTCWDT_RTC_RESET       = 16     <16, RTC Watch dog reset digital core and rtc module
*********************************************************************************************************************************/
RESET_REASON getResetInfoReason()
{
    // the Loop works typicaly on CPU 1 so we ask for CPU 1
    return rtc_get_reset_reason(1);
}


#endif

/********************************************************************************************************************************
 * Returns a Number for reason of last reboot interpretation is different for differnt Hardware
 * ******************************************************************************************************************************/
uint32_t getResetReason()
{
    #ifdef ESP8266  
        return (uint32_t)getResetInfoReason();
    #endif
    #ifdef ESP32
        return (uint32_t)getResetInfoReason();
    #endif
    return 999;
}


/******************************************************************************
 * Display available Networks on Serial Output
*******************************************************************************/
void serial_print_Networks()
{
  int n = WiFi.scanNetworks();

  Serial.println("\nScan finished");
  if (n == 0)
    Serial.println("No WiFi Network found");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
#ifdef ESP8266
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " (open)" : "( closed)");
#else
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " (open)" : "( closed)");
#endif
    }
  }
  Serial.println("");
}

#endif