#include <Arduino.h>
// include library, include base class, make path known
#include <GxEPD.h>
#include "SD.h"
#include "SPI.h"
#include <Adafruit_I2CDevice.h>
//#include <SI7021.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <tools.h>
#include <config.h>
#include <mqttconf.h>
#include <ESPAsyncWebServer.h>
#include <WebServerImpl.h>
#include <measurment.h>
//! There are three versions of the 2.13 screen,
//  if you are not sure which version, please test each one,
//  if it is successful then it belongs to the model of the file name
//  关于v2.3版本的显示屏版本,如果不确定购买的显示屏型号,请每个头文件都测试一遍.

//#include <GxGDE0213B1/GxGDE0213B1.h>      // 2.13" b/w
//#include <GxGDEH0213B72/GxGDEH0213B72.h>  // 2.13" b/w new panel
#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w newer panel

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <SansSerifBold72.h>
#include <gear.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#define SPI_MOSI 23
#define SPI_MISO -1
#define SPI_CLK 18

#define ELINK_SS 5
#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17

#define SDCARD_SS 13
#define SDCARD_CLK 14
#define SDCARD_MOSI 15
#define SDCARD_MISO 2

#define BUTTON_PIN 39
#define SDA_PIN 21
#define SCL_PIN 22
#define VBAT_PIN 35

#define MAX_BAT_VOLTAGE 3.7
#define MIN_BAT_VOLTAGE 2.3
#define ROUND_2_INT(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5))) 

GxIO_Class io(SPI, /*CS=5*/ ELINK_SS, /*DC=*/ ELINK_DC, /*RST=*/ ELINK_RESET);
GxEPD_Class display(io, /*RST=*/ ELINK_RESET, /*BUSY=*/ ELINK_BUSY);

SPIClass sdSPI(VSPI);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

RTC_DATA_ATTR int recordCounter = 0;
RTC_DATA_ATTR float setPoint=0;
RTC_DATA_ATTR float maxVoltage=0;
int wifiConnectCounter=0;
bool configMode;

void drawTemperature(float tempValue,int humidityPercent)
{
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&SansSerif_bold_72);
    display.setCursor(0,display.height()/2);
    display.printf("%.1f ",tempValue);
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(display.width()/2,display.height()-25);
    display.printf("%i%%",humidityPercent);
}

float getBatVoltage()
{
    int measurment=analogRead(VBAT_PIN);
    float volts=(3.3/4096)*measurment*2;
    if(volts>maxVoltage && volts > MAX_BAT_VOLTAGE)
    {
      maxVoltage=volts;
    }
    else
    {
      if(maxVoltage<MAX_BAT_VOLTAGE)
        maxVoltage=MAX_BAT_VOLTAGE;
    }
    Serial.printf("Max Voltage: %f \n",maxVoltage);
    return volts;
}

int calcBatPercent(float value)
{
    float loadFactor=maxVoltage-MIN_BAT_VOLTAGE;
    float valueFactor=value-MIN_BAT_VOLTAGE;
    float fbatPercent=(valueFactor/loadFactor)*100;
    int batPercent=ROUND_2_INT(fbatPercent);
    Serial.printf("\nLoad Factor: %f, Value Factor: %f, Bat Percent: %f\n",loadFactor,valueFactor,fbatPercent);
    return batPercent;
}

void drawBattState(float value,int batPercent)
{
    //draw outline
    // calculate Border points
    int xMin=display.width()-30;
    int xMax=xMin+30;
    int yMin=5; //make sure you habe 3 Pixels left for the Bat Head
    int yMax=display.height()-17;
    display.drawRect(xMin,yMin,xMax-xMin,yMax-yMin,GxEPD_BLACK);
    display.drawLine(xMin+5,yMin-1,xMax-5,yMin-1,GxEPD_BLACK);
    display.drawLine(xMin+5,yMin-2,xMax-5,yMin-2,GxEPD_BLACK);
    display.drawLine(xMin+5,yMin-3,xMax-5,yMin-3,GxEPD_BLACK);
    
    int maxLines=yMax-yMin;
    int runLines=((float)maxLines/100)*(batPercent);
    Serial.printf("Max Lines: %i, Percent Lines: %i, Bat Percent: %i",maxLines,runLines,batPercent);
    //fill Bat
    for (size_t i = 0; i < runLines; i++)
    {
      display.drawLine(xMin,yMax-i,xMax,yMax-i,GxEPD_BLACK);
    }
    // Set Markers
    int _25percentLine=((float)maxLines/100)*(25);
    int _50percentLine=((float)maxLines/100)*(50);
    int _75percentLine=((float)maxLines/100)*(75);
    if(batPercent<25)
    {
      display.drawLine(xMin+2,yMax-_25percentLine,xMax-2,yMax-_25percentLine,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_50percentLine,xMax-2,yMax-_50percentLine,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_75percentLine,xMax-2,yMax-_75percentLine,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_25percentLine-1,xMax-2,yMax-_25percentLine-1,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_50percentLine-1,xMax-2,yMax-_50percentLine-1,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_75percentLine-1,xMax-2,yMax-_75percentLine-1,GxEPD_BLACK);
    }
    else if(batPercent>25 && batPercent<50)
    {
      display.drawLine(xMin+2,yMax-_25percentLine,xMax-2,yMax-_25percentLine,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_50percentLine,xMax-2,yMax-_50percentLine,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_75percentLine,xMax-2,yMax-_75percentLine,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_25percentLine-1,xMax-2,yMax-_25percentLine-1,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_50percentLine-1,xMax-2,yMax-_50percentLine-1,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_75percentLine-1,xMax-2,yMax-_75percentLine-1,GxEPD_BLACK);
    }
    else if(batPercent>50 && batPercent<75)
    {
      display.drawLine(xMin+2,yMax-_25percentLine,xMax-2,yMax-_25percentLine,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_50percentLine,xMax-2,yMax-_50percentLine,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_75percentLine,xMax-2,yMax-_75percentLine,GxEPD_BLACK);
      display.drawLine(xMin+2,yMax-_25percentLine-1,xMax-2,yMax-_25percentLine-1,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_50percentLine-1,xMax-2,yMax-_50percentLine-1,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_75percentLine-1,xMax-2,yMax-_75percentLine-1,GxEPD_BLACK);
    }
    else
    {
      display.drawLine(xMin+2,yMax-_25percentLine,xMax-2,yMax-_25percentLine,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_50percentLine,xMax-2,yMax-_50percentLine,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_75percentLine,xMax-2,yMax-_75percentLine,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_25percentLine-1,xMax-2,yMax-_25percentLine-1,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_50percentLine-1,xMax-2,yMax-_50percentLine-1,GxEPD_WHITE);
      display.drawLine(xMin+2,yMax-_75percentLine-1,xMax-2,yMax-_75percentLine-1,GxEPD_WHITE);
    }
    
    
    //printout Value
    display.setFont(&FreeSans9pt7b);
    display.setCursor(xMin -6 ,yMax+15);
    display.printf("%.1fV",value);
}

void drawIpAddress(IPAddress address)
{
    display.setFont(&FreeSans9pt7b);
    display.setCursor(2,display.height()-1);
    display.print(address.toString());   
}

void drawBottomLine(char * content)
{
    display.setFont(&FreeSans9pt7b);
    display.setCursor(2,display.height()-1);
    display.print(content);   
}

void drawBottomLine(const char * content)
{
    display.setFont(&FreeSans9pt7b);
    display.setCursor(2,display.height()-1);
    display.print(content);   
}

void drawInfoLine(char * content)
{
    display.setFont(&FreeSans9pt7b);
    display.setCursor(1,display.height()-20);
    display.printf(content);   
}

void drawInfoLine(const char * content)
{
    display.setFont(&FreeSans9pt7b);
    display.setCursor(1,display.height()-20);
    display.printf(content);   
}

void connectMqtt() {
  String prefix=mqttPrefix;
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(getESPDevName().c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish((prefix+"/id").c_str(),getESPDevName().c_str());
      mqttClient.publish((prefix+"/ip").c_str(),WiFi.localIP().toString().c_str());
      // this will be used as Output to the Heater Device
      mqttClient.publish((prefix+"/output").c_str(),0);
      // ... and resubscribe
      // this value will be used as Destination Temperature
      mqttClient.subscribe((prefix+"/input").c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

void publishMqttValues(float temp,int humidity,float batVoltage,int batPercent)
{
  String prefix=mqttPrefix;
  if(mqttClient.connected())
  {
      Serial.printf("\nMQTT pub %f to %s\n",temp,(prefix+"/temperature").c_str());
      mqttClient.publish((prefix+"/temperature").c_str(),String(temp).c_str());
      Serial.printf("MQTT pub %i to %s\n",humidity,(prefix+"/humidity").c_str());
      mqttClient.publish((prefix+"/humidity").c_str(),String(humidity).c_str());
      Serial.printf("MQTT pub %f to %s\n",batVoltage,(prefix+"/voltage").c_str());
      mqttClient.publish((prefix+"/voltage").c_str(),String(batVoltage).c_str());
      Serial.printf("MQTT pub %i to %s\n",batPercent,(prefix+"/battery").c_str());
      mqttClient.publish((prefix+"/battery").c_str(),String(batPercent).c_str());
  }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String value;
  for (int i=0;i<length;i++) {
    value=value+(char)payload[i];
  }
  Serial.println(value);
  setPoint=value.toFloat();
}

bool detectConfigMode()
{
   bool btnPressed;
   if(digitalRead(BUTTON_PIN)==LOW)
   {
     btnPressed=true;
     delay(1000);
     if(digitalRead(BUTTON_PIN)==LOW)
     {
       Serial.println("Config Button pressed more than 1s");
       btnPressed=true;
     }
     else
     {
       btnPressed=false;
     }
   }
   else
   {
     btnPressed=false;
   }
   return btnPressed;
}

void refreshDisplay()
{
    char tmpString[35];

    display.setTextColor(GxEPD_BLACK);
    display.fillScreen(GxEPD_WHITE);
    if(configMode)
    {
      display.drawBitmap(image_data_Gear,display.width()-30-32-5,2,32,32,GxEPD_WHITE);
    }
    tempValue=(float)sensor.getCelsiusHundredths();
    humidityValue=sensor.getHumidityPercent();
    bat= getBatVoltage();
    Serial.printf("\nBat. Voltage: %f V\n",bat);
    ibat=calcBatPercent(bat);
    tempValue=tempValue/100;
    Serial.printf("\nTemperature: %f , Humidity: %i\n",tempValue,humidityValue);
    publishMqttValues(tempValue,humidityValue,bat,ibat);
    drawTemperature(tempValue,humidityValue);
    drawBattState(bat,ibat);
    if(WiFi.getMode()==WIFI_AP_STA)
    {
      sprintf(tmpString,"AP: %s",getESPDevName().c_str());
      drawInfoLine(IPAddress(192,168,0,4).toString().c_str());
      drawBottomLine(tmpString);
    }
    else
    {
      sprintf(tmpString,"Wakeups: %i",recordCounter);
      drawInfoLine(tmpString);
      drawIpAddress(WiFi.localIP());
    }
    
    display.update();
}

void setupAP(void)
{
  WiFi.mode(WIFI_AP_STA);

  //serial_print_Networks();

  Serial.println("Setup Soft AP...");
  IPAddress apIP(192, 168, 0, 4);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(getESPDevName().c_str());
  //WiFi.softAP(ssidAP, passwordAP, 3, false);
  delay(100);
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, LOW);
#endif
  Serial.printf("Soft AP '%s' online\n",getESPDevName().c_str());
}

void setup()
{
    recordCounter++;
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");
    sensor.begin(SDA_PIN,SCL_PIN);
    pinMode(VBAT_PIN,INPUT);
    pinMode(BUTTON_PIN,INPUT);
    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, ELINK_SS);
    initFileSystem();
    checkConfig();
    checkMqttConfig();
    readConfig();
    readMqttConfig();
    configMode=detectConfigMode();
    Serial.println("Start WiFi");
    if(!ssid.equals("."))
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(),passwd.c_str());
      while (WiFi.status()!= WL_CONNECTED)
      {
        if(!configMode)
        {
          if(wifiConnectCounter<10)
          {
            Serial.print("-");
            wifiConnectCounter++;
            delay(500);
          }
          else
          {
            //to much trys reboot
            ESP.restart();
          }
        }
        else 
        {
          if(wifiConnectCounter<10)
          {
            Serial.print("-");
            wifiConnectCounter++;
            delay(500);
          }
          else
          {
          //setup Ap configured Wifi not available
          setupAP();
          break;
          }
        }
      }
    }
    else
    {
      setupAP();
    }
    
    mqttClient.setServer(mqttServer.c_str(),mqttPort);
    mqttClient.setCallback(onMqttMessage);
    if(WiFi.isConnected() && WiFi.getMode()==WIFI_STA)
    {
      connectMqtt();
    }
    else
    {
      Serial.println("No Wifi suppress MQTT");
    }
    
    display.init(); // enable diagnostic output on Serial

    display.setRotation(1);

    refreshDisplay();

    // goto sleep
    if(!configMode)
    {
      Serial.println("\nGo to sleep");
      mqttClient.disconnect();
      WiFi.disconnect();
      esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);
      esp_sleep_enable_timer_wakeup(600000000);
      esp_deep_sleep_start();
    }
    else
    {
      Serial.println("\nConfig Mode active");
      //configure Web-Server
      configureWebServer();
    }
    
}


void loop()
{
  delay(30000);
  if(WiFi.getMode()==WIFI_STA && mqttClient.connected())
  {
    refreshDisplay();
  }
}