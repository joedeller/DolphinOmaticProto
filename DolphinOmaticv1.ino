/* Dolphin O Matic Fish orderer before rename
 * Convert the Dophin's order to Decimal
 * Using a 74HC165N shift register for INPUT
 * and a 74HC595N for OUTPUT
 * Finally 1306 OLED I2C display, SCL on A5, SDA on A4
 * Joe Deller 18th - 19th March 2017
 * V 0.1 - Inputs only
 * V 0.2 - LEDs to show current order state
 * V 0.3 - Wiring clean ups and reorder pins ready for matrix
 * V 0.4 - Started 7219 MAX matrix decimal display
 * V 0.5 - Switched to 1306 OLED, only 4 pins, less power and space
 * V 0.6 - Changed font sizes to better fit display
 * V 0.7 - Memory optimizations (strings to PROGMEM and smaller types) Still 1545 bytes (from 1654 bytes 80%) 
 * V 1.0 - Klunky ready to show
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "LedControl.h"
// Data, clock, Load, dispcount
LedControl lc=LedControl(13,11,12,1);

// Pulse delay to trigger the shift register read & latch
const byte PULSE_WIDTH_USEC =  5;

// Small delay to clean up switch debounce
const byte DEBOUNCE_WAIT =  20;

// 74HC595 pin connections, Arduino -> Chip
const byte dataOutPin  = 4; // DS    - pin 14
const byte latchOutPin = 5; // ST_CP - pin 12
const byte clockOutPin = 6; // SH_CP - pin 11

// 74HC195 pin connections, Arduino -> Chip
const byte clockInPin      = 7;  // Clock         - pin 2
const byte pLoadPin        = 8;  // Parallel Load - pin 1
const byte clockEnablePin  = 9;  // Clock Enable  - pin 13
const byte dataInPin       = 10; // Q7            - pin 8

// 1306 Display connects SDA Analog 4, SCL - Analog 5

unsigned int pinValues;
unsigned int oldPinValues;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);


// Read the current state of the buttons pressed feeding the single 74HC195 
unsigned int readInputShifter()
{
    byte bitVal;
    byte byteVal = 0;

    // Trigger a Parallel Load to latch the state of the data lines    
    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(pLoadPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(pLoadPin, HIGH);
    digitalWrite(clockEnablePin, LOW);

    // Step through each input line of the 74HC165N & store the total value in byteVal    
    for(byte i = 0; i < 8; i++)
    {
        bitVal = digitalRead(dataInPin);
        // Set the corresponding bit in bytesVal.
        byteVal |= (bitVal << ((7) - i));
        // Pulse the Clock to move to the next input line        
        digitalWrite(clockInPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockInPin, LOW);
    }
    return(byteVal);
}

// Show the current value of pinValues, NOT the current state of the buttons
void showCurrentFish(byte byteVal)
{
    Serial.println(F("Bit Status:"));
    for(byte i = 0; i < 8; i++)
    {
        Serial.print(F("  Bit-"));
        Serial.print(i);
        Serial.print(F(": "));

        //if((pinValues >> i) & 1)
        if((byteVal >> i) & 1)
            Serial.println(F("HIGH"));
        else
            Serial.println(F("LOW"));       
    }
    Serial.println();
}

void showLED(int value)
{
  lc.clearDisplay(0);
  char sval [3];
  sprintf(sval, "%03d", value);
  for (byte i = 0;i<3;i++)
  {
  
    lc.setChar(0,2-i,sval[i],false);
  }
}
void showLCD(int value)
{
  display.setTextSize(2);
  display.clearDisplay();     
  display.setCursor(0,0); 
  display.print (F("I wish for"));
  display.setCursor(0,18); 
  display.setTextSize(3);
  display.print(value);
  display.setTextSize(2);
  display.setCursor(0,44);
  display.print (F("fish"));
  display.display(); 
}

void showLeds(int value)
{
    Serial.print(F("Writing "));
    Serial.println(value);    
    digitalWrite(latchOutPin, LOW);
    // shift out the bits to our LEDs
    shiftOut(dataOutPin, clockOutPin, MSBFIRST, value);  
    //take the latch pin high so the LEDs will light up
    digitalWrite(latchOutPin, HIGH);     
}


void setup()
{
    Serial.begin(9600);
    // Initialize digital pins
    pinMode(dataInPin, INPUT);
    
    pinMode(pLoadPin, OUTPUT);
    pinMode(clockEnablePin, OUTPUT);
    pinMode(clockInPin, OUTPUT);  
    pinMode(latchOutPin, OUTPUT);
    pinMode(clockOutPin, OUTPUT);
    pinMode(dataOutPin, OUTPUT);

    digitalWrite(clockInPin, LOW);
    digitalWrite(pLoadPin, HIGH);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Init screen

    display.clearDisplay();     
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0); 
    display.print(F("DolphinOMatic V1.0!"));
    display.display();
    lc.shutdown(0,false);
    lc.setIntensity(0,8);
     lc.clearDisplay(0);
     lc.setDigit(0,0,0,false);
    delay (1200);
    showLCD(0);
          
    showLeds(0);  // Clear the LEDs at start up
    //pinValues = readInputShifter(); // Read in and display the pin states at startup.
    //oldPinValues = pinValues;
    showLeds(0);
}

void loop()
{    
    // Read current state of the buttons    
    pinValues = readInputShifter();
    // We only care if any are currently pressed
    if(pinValues != 0)
    {
        Serial.println(F("*Button press detected*"));
        // XOR the previous pins, so if we are high and previous was high, go low, otherwise go high if high
        pinValues = (pinValues ^ oldPinValues);                
        showCurrentFish(pinValues);
        showLeds(pinValues);
        showLCD(pinValues);
        showLED(pinValues);
        oldPinValues = pinValues;
        delay (DEBOUNCE_WAIT); 
    }
    delay(DEBOUNCE_WAIT);
}
