//Simple CO2 monitor - no wifi
//Still testing...
//The ambient light sensor doesnt work well and shoild be a right angle type
//Touchscreen test is good
//To do - Have ambient light control backlight brightness

#include <TFT_eSPI.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"      //Adafruit one
#include <BH1750.h>              //Christopher Laws version
#include "ClosedCube_HDC1080.h"  //Closed Cube version
#include <CST816S.h>             //https://github.com/fbiego/CST816S
#include "NotoSansMonoSCB20.h"
#include "NotoSansBold36.h"
#include "Final_Frontier_28.h"


#define White     0xFFFF
#define Green     0x07E0
#define Black     0x0000
#define Yellow    0xFFE0
#define DarkGrey  0x4208 
#define Brown     0x9A60
#define Orange    0xFDA0
#define LightBlue 0x867D
#define Red       0xF800

#define temperaure_offset 16.1  //13.5
#define TFT_BL 26

CST816S touch(13, 14, 33, 9);  // sda, scl, rst, int)
BH1750 lightMeter;
Adafruit_SGP30 sgp;
ClosedCube_HDC1080 hdc1080;

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); 
  return absoluteHumidityScaled;
}

const long interval = 5000;
unsigned long previousMillis = 0;


// added by Kassim
#define notActiveColor    DarkGrey // when bar not lit
#define greenColor        Green   // good CO2 value
#define yellowColor       Yellow  // not so good CO2 value
#define redColor          Red     // worse CO2 value

const byte totalSpriteBars = 24; // total bars

int colorArray[totalSpriteBars + 10] = {notActiveColor};
int heightArray[totalSpriteBars + 10] = {0};

const byte barMaxHeight = 30,
           barMinHeight = 2,
           barWidth = 6,
           barSpacing = 10,
           startX = 10,
           startY = 120,  //160
           none = 78;

void setup()
{
  Serial.begin(115200);
  delay(250);
  Wire.begin(13, 14);
  hdc1080.begin(0x40);
  lightMeter.begin();
  sgp.begin();
  touch.begin();
  tft.init();
  //tft.setSwapBytes(true);  //For icons
  tft.setRotation(0);
  sprite.createSprite(240, 240);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 120);  //40,80,120,160,200 Brightness levels
  inActiveState();
}

void inActiveState() 
{
  sprite.fillRect(20, 120, 240, 40, Black);  //20, 120, 240, 40
  for ( byte x = 0; x < totalSpriteBars; x++ )
  {
   sprite.fillRect(startX + (x * barSpacing), startY, barWidth, barMaxHeight, notActiveColor);
  }

  sprite.pushSprite(0, 0);
}


void drawCO2_Graph() 
{ 
  static uint32_t prev = 0;
  if ( millis() - prev < 60000 ) {  // update every second?
    return;
  }
  prev = millis();
 
  static int minSensorValue = 400, maxSensorValue = 1200;
  sgp.IAQmeasure();  //Grab latest sample 
  int sensorValue = sgp.eCO2;
  if(sensorValue>1200) sensorValue=1200; 
  Serial.println(sensorValue);
  int barColor = 0; // based on sensor value
  
  if ( sensorValue > 800 && sensorValue < maxSensorValue ) 
  {
   //Serial.println("Red");
   barColor = greenColor;  //redColor
  }
  else if ( sensorValue > 600 && sensorValue <= 800 )
  {
   //Serial.println("Yellow");
   barColor = greenColor;  //yellowColor
  }

  else if ( sensorValue <= 600 ) 
  {
   //Serial.println("Green");
   barColor = greenColor;
  }

  else 
  {
  }

  static int barCounter = 0; 

  if ( barCounter >= totalSpriteBars - 1 )
  {  
   int len = totalSpriteBars - 1;

   for ( byte x = 0; x < len; x++ ) 
   {
    colorArray[x] =  colorArray[x+1];
    heightArray[x] =  heightArray[x+1];
  }

  colorArray[len] = barColor;
  heightArray[len] = map(sensorValue,minSensorValue,maxSensorValue,barMinHeight,barMaxHeight); // 

 }

 else 
 {
  colorArray[barCounter] = barColor;
  heightArray[barCounter] = map(sensorValue,minSensorValue,maxSensorValue,barMinHeight,barMaxHeight); //  
  barCounter++; // increment this
 }

  for ( byte x = 0; x < totalSpriteBars; x++ ) 
  {
   int getHeight = heightArray[x];
   if  ( getHeight == 0 ) {
     continue;
   }

   int color_ = colorArray[x];
   int startingY = startY + barMaxHeight - getHeight; 
   int newHeight = barMaxHeight-getHeight;
   sprite.fillRect(startX+(x*barSpacing), startY,barWidth,barMaxHeight,notActiveColor);  // remove any previous color 
   sprite.fillRect(startX+(x*barSpacing), startingY,barWidth,getHeight,color_); // update the new color
  }
 sprite.pushSprite(0,0); 
}


void drawText()
{
  sprite.setTextColor(White, Black); 
  sprite.drawString("Desktop Sensor", 60, 25, 2);
  sprite.pushSprite(0, 0);
}

void drawCO2_Text()
{
  unsigned long value;
  String co2_Value;

  sgp.IAQmeasure();  //Grab latest sample
  value = sgp.eCO2;


  
  if (value > 999) value = 999;
  int sum = 0;
  //co2_Value = value;

  //Shows down the updates
  
  for(int i=0; i<100; i++)
  {
   sum = sum + value;
   delay(2);
  }
  int result = sum/100;
  co2_Value = result;
  

  sprite.fillRect(50, 45, 150, 60, Black); //Black out old CO2 value
  sprite.setTextColor(White, Black);
    sprite.loadFont(NotoSansBold36);
    sprite.drawString(co2_Value, 90, 50);
  //sprite.drawString(co2_Value, 75, 50, 6);
    sprite.unloadFont(); 
  
  sprite.drawString("ppm", 165, 75, 2);
  sprite.setTextColor(LightBlue, Black);
  sprite.drawString("eCO2", 100, 95, 2);
  sprite.pushSprite(0, 0);

  /*
  float get_battery_voltage(){
  
  delay(2);
  int sum = 0;
  for(int i=0; i<1000; i++){
    sum = sum + analogRead(ADC_PIN);
  }
  int result = sum/1000;
  return float(result) * (1.437);
  }
  */
}



void drawLightMeter_Dot()
{
  float lux = lightMeter.readLightLevel();
  lux = map(lux, 0, 100, 1, 10);
  //sprite.pushImage(96,0,48,48,LightBulb);  //Testing icons
  sprite.fillCircle(120, 18, 10, Black); //Black out old dot
  sprite.fillCircle(120, 18, lux, Yellow);
  sprite.pushSprite(0, 0);
}

void drawTemp_Hum()
{
  String Temperature;
  String Humidity;
  Temperature = (hdc1080.readTemperature() - temperaure_offset);
  Humidity = (hdc1080.readHumidity());
  sprite.setTextColor(White, Black);
  
    sprite.fillRect(40, 180, 160, 20, Black); //Black out old CO2 value
    sprite.loadFont(NotoSansMonoSCB20);
  //sprite.drawString(Temperature, 40, 180, 4);
  //sprite.drawString(Humidity, 140, 180, 4);
    sprite.drawString(Temperature, 40, 180);
    sprite.drawString(Humidity, 140, 180);   
  
    sprite.unloadFont();
  
  sprite.drawString("'C", 65, 205, 2);
  sprite.drawString("%H", 165, 205, 2);
}

void drawTouch()
{
  if (touch.available())
  {
   sprite.drawLine(touch.data.x - 25, touch.data.y, touch.data.x + 75, touch.data.y, Red);
   sprite.drawLine(touch.data.x + 25, touch.data.y - 50, touch.data.x + 25, touch.data.y + 50, Red);
   sprite.pushSprite(0, 0);
  }
  sprite.drawLine(touch.data.x - 25, touch.data.y, touch.data.x + 75, touch.data.y, Black);
  sprite.drawLine(touch.data.x + 25, touch.data.y - 50, touch.data.x + 25, touch.data.y + 50, Black);
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
