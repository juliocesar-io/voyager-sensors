/*
* Project voyager-sensors
* Description: Read multiple sensors
* Author: Julio César
* Date: Apr 2018
*/

// #include "lib/Adafruit_GFX.h"
// #include "lib/Adafruit_DHT.h"
// #include "lib/SI114X.h"
#include "lib/Adafruit_SSD1306.h"
#include "lib/Adafruit_HTU21DF.h"
#include "Particle.h"



#define OLED_RESET D1
#define DHTPIN D2
#define DHTTYPE DHT11
#define LOG_PERIOD 15000
#define MAX_PERIOD 60000
#define RGB_A A1
#define RGB_B A2
#define RGB_C A3

unsigned long counts;
unsigned long cpm;
unsigned int multiplier;
unsigned long previousMillis;
int cell_bars = 0;
char jsonString[200];
int sample = 0;
int ReadUVintensityPin = A2;

// SI114X SI1145 = SI114X();
// DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(OLED_RESET);

Adafruit_HTU21DF htu = Adafruit_HTU21DF();


SYSTEM_THREAD(ENABLED);

STARTUP(RGB.mirrorTo(A4, A5, A7));
void setup() {
 Serial.begin(9600);
 // beginUVSensor();
 // beginTEMPSensor();
 beginHTUSensor();
 beginGeigerSensor();
 beginDisplay();

 // New UV sensor
 pinMode(ReadUVintensityPin, INPUT);


 //Particle.variable("onData", jsonString, STRING);
}

void setGeigerTubeImpulse(){
 counts++;
}

void beginGeigerSensor(){
 counts = 0;
 cpm = 0;
 multiplier = MAX_PERIOD / LOG_PERIOD;
 pinMode(D5, INPUT);
 attachInterrupt(D5, setGeigerTubeImpulse, FALLING);
}

/*
void beginTEMPSensor(){
 dht.begin();
}
*/

void beginHTUSensor(){
  htu.begin();
}

/*
void beginUVSensor(){
 while (!SI1145.Begin()) {
   display.setTextSize(1);
   display.setCursor(27,30);
   display.setTextColor(WHITE);
   display.println("Error sensor UV, esperando..");
   display.display();
   display.clearDisplay();
   delay(1000);
 }
}
*/

void beginDisplay(){
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 display.display();
 display.clearDisplay();
 display.setTextSize(3);
 display.setCursor(0,0);
 display.setTextColor(WHITE);
 display.println("VOYAGER");
 display.setTextSize(3);
 display.setCursor(27,30);
 display.setTextColor(BLACK,WHITE);
 display.println(" V3 ");
 display.display();
 delay(8000);
 display.clearDisplay();
}

/*
void displayCelullarBars(){
 CellularSignal sig = Cellular.RSSI();
 int rssi = sig.rssi;

 if (rssi < 0) {
   if (rssi >= -57) cell_bars = 5;
   else if (rssi > -82) cell_bars = 4;
   else if (rssi > -94) cell_bars = 3;
   else if (rssi > -116) cell_bars = 2;
   else if (rssi > -118) cell_bars = 1;
 }

 for (int b=0; b <= cell_bars; b++) {
   display.fillRect(59 + (b*5),33 - (b*5),3,b*5,WHITE);
 }
 display.display();
 delay(2000);
 display.clearDisplay();
}
*/

void collectData(){
 unsigned long currentMillis = millis();
 if(currentMillis - previousMillis > LOG_PERIOD){

     ++sample;
     previousMillis = currentMillis;
     cpm = counts * multiplier;
     counts = 0;

     float sv0 = cpm * 0.00884;
     float sv1 = cpm * 0.00662;

     // int vis = SI1145.ReadVisible();
     // int ir = SI1145.ReadIR();
     // float uv = (float)SI1145.ReadUV()/100;
     // int h = dht.getHumidity();
     // int t = dht.getTempCelcius();
     // int hi = dht.getHeatIndex();
     // int dp = dht.getDewPoint();
     int uvLevel = averageAnalogRead(ReadUVintensityPin);
     float outputVoltage = 5.0 * uvLevel/1024;
     float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
     float temp = htu.readTemperature();
     float rel_hum = htu.readHumidity();

     display.setTextSize(1.5);
     display.setCursor(0,0);
     display.setTextColor(WHITE);
     display.println("sv0: " + String(sv0) + " µSv/h" );

     display.setTextSize(1.5);
     display.setCursor(0,12);
     display.setTextColor(WHITE);
     display.println("CPM: " + String(cpm));


     display.setTextSize(1.5);
     display.setCursor(0,22);
     display.setTextColor(WHITE);
     display.println("HUM: " + String(rel_hum) + " % ");

     display.setTextSize(1.5);
     display.setCursor(0,32);
     display.setTextColor(WHITE);
     display.println("UV: " + String(uvIntensity) + " mW/cm^2");

     display.setTextSize(1.5);
     display.setCursor(0,42);
     display.setTextColor(WHITE);
     display.println("TEMP: " + String(temp) + " °C" );
     display.display();
     delay(2000);
     display.clearDisplay();
     //sprintf(jsonString,"{\"sample\": %d, \"cpm\": %d, \"sv0\": %f, \"sv1\": %f, \"vis\": %d, \"ir\": %d, \"uv\": %f, \"hum\": %d, \"temp\": %d, \"hi\": %d, \"dp\": %d, \"time\": %d}", sample, cpm, sv0, sv1, vis, ir, uv, h, t, hi, dp, Time.local());
     //Particle.publish("onData", jsonString);



 }
}

//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
 byte numberOfReadings = 8;
 unsigned int runningValue = 0;

 for(int x = 0 ; x < numberOfReadings ; x++)
   runningValue += analogRead(pinToRead);
 runningValue /= numberOfReadings;

 return(runningValue);

}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {


 collectData();

 //displayCelullarBars();
}
