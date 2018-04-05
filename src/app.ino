/*
 * Project voyager-sensors
 * Description: Read multiple sensors
 * Author: Julio César
 * Date: Apr 2018
 */
#include "lib/Adafruit_SSD1306.h"
#include "lib/Adafruit_GFX.h"
#include "lib/Adafruit_DHT.h"
#include "lib/SI114X.h"


#define OLED_RESET D1
#define DHTPIN D2
#define DHTTYPE DHT11
#define LOG_PERIOD 15000
#define MAX_PERIOD 60000

unsigned long counts;
unsigned long cpm;
unsigned int multiplier;
unsigned long previousMillis;
int cell_bars = 0;
char jsonString[200];
int sample = 0;

SI114X SI1145 = SI114X();
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  Serial.begin(9600);
  beginUVSensor();
  beginTEMPSensor();
  beginGeigerSensor();
  beginDisplay();

  Particle.variable("onData", jsonString, STRING);
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

void beginTEMPSensor(){
  dht.begin();
}

void beginUVSensor(){
  while (!SI1145.Begin()) {
    delay(1000);
  }
}

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

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > LOG_PERIOD){

      ++sample;
      previousMillis = currentMillis;
      cpm = counts * multiplier;
      counts = 0;

      int vis = SI1145.ReadVisible();
      int ir = SI1145.ReadIR();
      float uv = (float)SI1145.ReadUV()/100;
      int h = dht.getHumidity();
      int t = dht.getTempCelcius();
      int hi = dht.getHeatIndex();
      int dp = dht.getDewPoint();

      sprintf(jsonString,"{\"sample\": %d, \"cpm\": %d, \"vis\": %d, \"ir\": %d, \"uv\": %f, \"hum\": %d, \"temp\": %d, \"hi\": %d, \"dp\": %d}", sample, cpm , vis, ir, uv, h, t, hi, dp);
      Particle.publish("onData", jsonString);
  }
}
