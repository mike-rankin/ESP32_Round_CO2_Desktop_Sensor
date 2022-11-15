//Still testing...
//The ambient light sensor doesnt work well and shoild be a right angle type
//The eCO2 graph definitely doesn't work yet
//Possibly every few seconds, draw a green graph and then sifit over to right and do another
//Touchscreen test is good
//To do - Have ambient light control backlight brightness


#include <TFT_eSPI.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"      //Adafruit one
#include <BH1750.h>              //Christopher Laws version
#include "ClosedCube_HDC1080.h"  //Closed Cube version
#include <CST816S.h>             //https://github.com/fbiego/CST816S
#include "Icons.h"

#define White  0xFFFF
#define Green  0x07E0  
#define Black  0x0000
#define Yellow 0xFFE0   
#define DarkGrey 0x7BEF 
#define Brown 0x9A60  
#define Orange 0xFDA0 
#define LightBlue 0x867D 
#define Red 0xF800 

#define temperaure_offset 13.5
#define TFT_BL 26

CST816S touch(13, 14, 33, 9);  // sda, scl, rst, int)
BH1750 lightMeter;
Adafruit_SGP30 sgp;
ClosedCube_HDC1080 hdc1080;

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0; 

TFT_eSPI tft = TFT_eSPI();  
TFT_eSprite sprite= TFT_eSprite(&tft);

uint32_t getAbsoluteHumidity(float temperature, float humidity) 
{
 const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
 const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
 return absoluteHumidityScaled;
}

const long interval = 1000; 
unsigned long previousMillis = 0; 


void setup() 
{
  Serial.begin(115200);
  delay(250);
  Wire.begin(13,14);
  hdc1080.begin(0x40);
  lightMeter.begin();
  sgp.begin();
  touch.begin();
  tft.init();
  //tft.setSwapBytes(true);  //For icons
  tft.setRotation(0);
  sprite.createSprite(240,240); 

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 120);  //40,80,120,160,200 Brightness levels
}

void drawText()
{ 
  sprite.setTextColor(White,Black);   //Black (0x0000) with White(0xFFFF) text
  sprite.drawString("Desktop Sensor",60,25,2);
  sprite.pushSprite(0,0);
}

void drawCO2_Text()
{ 
  String co2_Value;
  co2_Value = sgp.eCO2;
  sgp.IAQmeasure();  //Grab latest sample 
  //Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  sprite.setTextColor(White,Black);
  sprite.drawString(co2_Value,75,50,6);
  sprite.drawString("ppm",165,75,2);
  sprite.setTextColor(LightBlue,Black); 
  sprite.drawString("eCO2",100,95,2);
  sprite.pushSprite(0,0);
}

void drawCO2_Graph()
{ 
  //sprite.fillRect(20,100,200,80,Black);    //Black out only graph
  //sprite.fillRect(20,100,200,80,0x000F);   //Black out graph test
  
  int sum=0;
  int values[7]={0};

  sprite.drawLine(20,170,220,170,DarkGrey);  //Grey across
  sprite.drawLine(120,170,120,240,DarkGrey); //Grey down
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval)
  {
     
   for(int i=0;i<7;i++)
   {
    previousMillis = currentMillis;
    sgp.IAQmeasure();  //Grab latest sample 
    values[i]=map(sgp.eCO2, 400, 1000, 1, 80);  //map the 400ppm to 60,000ppm to a smaller height
    sprite.fillRect(20+(i*30),165-values[i],16,values[i],Green);
    sprite.pushSprite(0,0);
    //delay(2000);
   }
  
  }
  sprite.pushSprite(0,0);
}

void drawLightMeter_Dot()
{ 
 float lux = lightMeter.readLightLevel();
 lux = map(lux, 0, 100, 1, 10);
 //sprite.pushImage(96,0,48,48,LightBulb);  //Testing icons
 sprite.fillCircle(120,18,10,Black);   //Black out old dot
 sprite.fillCircle(120,18,lux,Yellow); 
 sprite.pushSprite(0,0);
 Serial.println(lux);
 //ledcWrite(pwmLedChannelTFT, 120);  //Ambient light to control screen brightness 
}

void drawTemp_Hum()
{ 
 String Temperature;
 String Humidity;
 Temperature = (hdc1080.readTemperature()-temperaure_offset);
 //Temperature =  Temperature - 5;
 Humidity = (hdc1080.readHumidity());
 sprite.setTextColor(White,Black);
 sprite.drawString(Temperature,40,180,4);
 sprite.drawString(Humidity,140,180,4);
 sprite.drawString("'C",65,205,2);
 sprite.drawString("%H",165,205,2);
}

void drawTouch()
{ 
 if (touch.available())
 {
  sprite.drawLine(touch.data.x-25,touch.data.y,touch.data.x+75,touch.data.y,Red); 
  sprite.drawLine(touch.data.x+25,touch.data.y-50,touch.data.x+25,touch.data.y+50,Red);  
  sprite.pushSprite(0,0);
 } 
  sprite.drawLine(touch.data.x-25,touch.data.y,touch.data.x+75,touch.data.y,Black);
  sprite.drawLine(touch.data.x+25,touch.data.y-50,touch.data.x+25,touch.data.y+50,Black);
}


void loop()
{
  //drawText();
  drawTouch();
  drawCO2_Text();
  drawCO2_Graph();
  drawLightMeter_Dot();
  drawTemp_Hum();
}
