//Still testing...
//The ambient light sensor doesnt really work and shoild be a right angle type
//The eCO2 graph definitely doesn't work yet
//Possibly every few seconds, draw a green graph and then sifit over to right and do another


#include <TFT_eSPI.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"      //Adafruit one
#include <BH1750.h>              //Christopher Laws version
#include "ClosedCube_HDC1080.h"  //Closed Cube version

#define White  0xFFFF
#define Green  0x07E0  
#define Black  0x0000
#define Yellow 0xFFE0   
#define DarkGrey 0x7BEF 
#define Brown 0x9A60  
#define Orange 0xFDA0 
#define LightBlue 0x867D 

#define temperaure_offset 13.5

BH1750 lightMeter;
Adafruit_SGP30 sgp;
ClosedCube_HDC1080 hdc1080;

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
  tft.init();
  tft.setRotation(0);
  sprite.createSprite(240,240); 
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
  sprite.drawString(co2_Value,75,40,6);
  sprite.drawString("ppm",165,65,2);
  sprite.setTextColor(LightBlue,Black); 
  sprite.drawString("eCO2",110,80,2);
  sprite.pushSprite(0,0);
}

void drawCO2_Graph()
{ 
  //sprite.fillRect(20,100,200,80,Black);    //Black out only graph
  //sprite.fillRect(20,100,200,80,0x000F);   //Black out graph test
  
  int sum=0;
  int values[7]={0};

  sprite.drawLine(20,160,220,160,DarkGrey);  //Grey across
  sprite.drawLine(120,160,120,240,DarkGrey); //Grey down
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval)
  {
     
   for(int i=0;i<7;i++)
   {
    previousMillis = currentMillis;
    sgp.IAQmeasure();  //Grab latest sample 
    values[i]=map(sgp.eCO2, 400, 1000, 1, 80);  //map the 400ppm to 60,000ppm to a smaller height
    sprite.fillRect(20+(i*30),150-values[i],16,values[i],Green);
    sprite.pushSprite(0,0);
    //delay(2000);
   }
  
  }
  sprite.pushSprite(0,0);
}

void drawLightMeter_Dot()
{ 
 float lux = lightMeter.readLightLevel();
 map(lux, 0, 10, 5, 10);
 sprite.fillCircle(120,10,15,Black);   //Black out old dot
 sprite.fillCircle(120,10,lux,Yellow);  
 sprite.pushSprite(0,0);
}

void draw_Temp_Hum()
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


void loop()
{
  //drawText();
  drawCO2_Text();
  drawCO2_Graph();
  drawLightMeter_Dot();
  draw_Temp_Hum();
}
