#include <TM1637Display.h>

#define CLK 9
#define DIO 7

#define NEG 0b01000000
#define DOT 0b10000000

#define HOLD_DIFF 5 //0.5 PSI drop will kick it the holding
#define HOLD_THRESH 80 //10 = 1 psi 100 = 10 psi
#define HOLD_TIME 1500 //milliseconds

TM1637Display display = TM1637Display(CLK, DIO);

long count;
int pressure, prev_pressure;

void setup()
{
  display.clear();
  delay(1000);
  pressure = prev_pressure = 0;
  Serial.begin(9600);
  analogReference(DEFAULT);
}

void displayValue(int val)
{
  uint8_t data[4] = {0};
  //Set the brightness:
  display.setBrightness(0x0, true);
  if(val >= 0)
    data[0] = 0;
  else
  {
    val *= -1;
    data[0] = NEG;
  }
  for(int i = 3; i > 0; i--)
  {
    data[i] = display.encodeDigit(val % 10);
    val /= 10;
  }  
  data [2] |= DOT;
  display.setSegments(data);
}

int maximum(int a, int b)
{
  if(a > b)
    return a;
}

int readPressurePSI()
{
  float d2a = (float(analogRead(A0)/1024.0)*3.244);
  int val = (float((d2a-0.3244)/2.5952)*36.26) * 10;
  return val;
}


void loop()
{
  //Get PSI value
  pressure = readPressurePSI();
  /*
  Hold value if the previous pressure is greater than the current pressure, if the previous pressure is above 
  the threshold and the difference between the current and previous pressure is greater than a certain value
  */
  if((prev_pressure > pressure) && (prev_pressure > HOLD_THRESH) && ((prev_pressure-pressure) > HOLD_DIFF))
  {
    //The below loop takes roughly 24 milliseconds to carry out, so 1500ms pause is what is shown here.
    int hold = (HOLD_TIME/24);
    while(hold)
    {
      pressure = readPressurePSI();
      if(prev_pressure > pressure)
      {
        displayValue(prev_pressure);
      }
      else
      {
        displayValue(pressure);
        prev_pressure = pressure;
        hold = 64;
      }
      hold--;
    }
  }
  else
  {
    displayValue(pressure);
  }
  prev_pressure = pressure;
  delay(150);
}