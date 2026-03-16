/*
Ttile: Burglar Alarm Project
Author: David Ross
Date: Friday Nov 14th, 2025
Description: This code uses the Infrared Receiver Module and a TV Remote to
             select Options in the Burglar Alarm Program:
             1 - Turn on the Alarm System
             2 - ARM the Alarm System
             3 - DisARM the Alarm System
             4 - Shut off the ALARM system

             GLOBAL Variable SELECTOR will take on values from 0-4

             SELECTOR = 0   - Alarm is off since no buttons were pushed on Remote
             SELECTOR = 1   - Button 1 has been pressed and the ALARM is ON
             SELECTOR = 2   - Button 2 was pressed to ARM the Alarm
             SELECTOR = 3   - Button 3 disARMS the ALARM
             SELECTOR = 4   - Button 4 was pressed to turn OFF the Alarm

             Throughout this program we will be using FUNCTIONS and GLOBAL
             Variables to simplify coding.

            
*/
// FUNCTION PROTOTYPE SECTION
void BUTTON1_ALARM_ON(void);        // Button1 ALARM ON  Function
void BUTTON2_ALARM_ARM(void);       // Button2 ALARM_ARM Function
void BUTTON3_ALARM_DISARM(void);    // Button3 ALARM_DISARM Function
void BUTTON4_ALARM_OFF(void);       // Button4 ALARM_OFF Function
void MOTION_SENSING(void);          // Motion_Sensing Function
void INTRUDER_DETECTED_WHITE(void); // Intruder Detected WHITE Function
void INTRUDER_DETECTED_RED(void);   // Intruder Detected RED Funcion
void ALARM_SOUNDING(void);          // Alarm Sounding LCD Function
void seg_counter(void);             // counts from 10 to 0 on 4-digit display
void DHT(void);

// INCLUDE SECTION
#include "arduino.h"        // add arduino library
#include <IRremote.h>       // add IRremote library
#include <Wire.h>           // include Wire.h library
#include "rgb_lcd.h"        // include rgb_lcd library
#include <TM1637Display.h>  // include 4 Digit Display library
#include <dht11.h>                   // include dht11.h library

// DEFINE SECTION
#define RED_LED 25          // RGB RED LED connected to PIN 25
#define GREEN_LED 26        // RGB GREEN LED connected to PIN 26
#define BLUE_LED 27         // RGB BLUE LED connected to PIN 27
#define CLK  33                // The ESP32 pin GPIO33 connected to CLK
#define DIO  32                // The ESP32 pin GPIO32 connected to DIO
#define Colon_On 0x40          // mask value to turn ON Colon on display
#define Colon_Off 0x00         // mask value to turn OFF Colon on display
#define DHT11PIN 4                   // DHT11 Data pin connected to Pin 4


// Hardware Definitions
rgb_lcd lcd;                      // make lcd an instance of rgb_lcd
int RECV_PIN = 2;                 // Infrared receiver pin
TM1637Display segment = TM1637Display(CLK, DIO);
int sensorPin =19 ;               // Pin # of the infrared motion sensor pin
int ledPin = 25;                  // Pin # of the BUILTIN LED 
dht11 DHT11;                         // declare DHT11 to be of class dht11


// Global Variable Section
long decoded_value;         // decoded value from IR Remote
decode_results results;     // Create a decoding results class object
int SELECTOR=0;               // Used to SELECT which Alarm Action to take
int count;                    // variable used for 4 dig display count down
int motion_detected=0;        // used for PIR motion Detector       
short read_DHT11;                    // GLOBAL read_DHT11 used to start conversion
float humid,temp,fahr;               // GLOBAL float humid, temp, and fahr
char buff[16];                       // used by sprintf for 16 char LCD display


// INTERRUPT SERVICE ROUTINE SECTION    
IRrecv irrecv(RECV_PIN);    // Create a class object used to receive class 

static portMUX_TYPE my_mutex = portMUX_INITIALIZER_UNLOCKED;                       

void IRAM_ATTR isr()              // puts isr code into RAM for fast response
{
  portENTER_CRITICAL(&my_mutex);  // Block any processes accessing sensorPin
                                  // since IRQ PB_PIN access is MUTually EXclusive.
  motion_detected=1;              // make motion_detected = 1 to indicate isr() has executed
  portEXIT_CRITICAL(&my_mutex);   // allow other processes to access sensorPin  
}
    
void setup()
{
  Serial.begin(9600);       // Initialize the serial port and set the baud rate to 9600
  irrecv.enableIRIn();      // Start the receiver
  Wire.begin(SDA, SCL);              // attach to the IIC pin
  // set RGB LEDS to OUTPUT
  pinMode(RED_LED, OUTPUT);  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT); 

  // Shut OFF all RGB LEDS
  digitalWrite(RED_LED,1);
  digitalWrite(GREEN_LED,1);
  digitalWrite(BLUE_LED,1);
  Wire.begin();                     // initialize WIRE system
  lcd.begin(16,2);                  // start up the LCD
  lcd.display();                    // turn on LCD display
  segment.setBrightness(4);   // set the brightness to 4 in range (0:dimmest, 7:brightest)
  segment.clear();            // clear the 4 digit display
  

}

void loop() 
{ 

for(;;)
 {
  while(SELECTOR==0)
  { 
    DHT();
    if ((irrecv.decode(&results)))           // if ir results are NOT received
    {
    decoded_value=(results.value);            // Get results.value
    if((decoded_value)!=0xFF30CF)             // if decoded value is NOT 1 from TV Remote
    {
       irrecv.resume();                       // set to Receive the next value
    }
    else
    {
       digitalWrite(GREEN_LED,0);             // turn on GREEN LED
       digitalWrite(RED_LED,1);               // turn OFF RED LED
       digitalWrite(BLUE_LED,1);              // turn OFF BLUE LED
       BUTTON1_ALARM_ON();                    // Alarm On Message
       //delay(200);
       SELECTOR=1;
       irrecv.resume();                       //Receive the next value
    }
   }
  } 
 while(SELECTOR==1)
 {
  while (!(irrecv.decode(&results)));        // while ir results are NOT received
   decoded_value=(results.value);            // received results go into decoded.value
   // If decoded value is NOT 2-key or 4-key
   if(((decoded_value)!=0xFF18E7)&&((decoded_value)!=0xff10EF))
   {
     irrecv.resume();                         // Receive the next value
   }
   else
   {
     if(decoded_value==0xFF18E7)              // if decoded value is 2-key
     {
       BUTTON2_ALARM_ARM();                   // put up ALARM_ARM message on LCD
       digitalWrite(GREEN_LED,1);             // TURN OFF GREEN and BLUE LEDS
       digitalWrite(BLUE_LED,1);
       digitalWrite(RED_LED,0);               // Turn ON RED led
       SELECTOR=2;
     }
      else                                    // if value not 2-key it must be 4-key
      {
        BUTTON4_ALARM_OFF();                  // put up ALARM GOING OFF LCD message
        digitalWrite(RED_LED,0);              // TURN ON RED LED
        digitalWrite(BLUE_LED,0);             // TURN ON BLUE LED (makes purple)
        digitalWrite(GREEN_LED,1);            // turn OFF Green LED
        SELECTOR=0;                           // set SELECTOR to 0 to start over
        delay(3000);                          // wait 3 seconds
        digitalWrite(RED_LED,1);              // turn off RED and BLUE LEDS (Purple)
        digitalWrite(BLUE_LED,1);
        irrecv.resume();                      // Receive the next value
        lcd.setRGB(0,0,0);                    // turn off all lcd colours
        lcd.clear();                          // clear text on LCD
        
      }
    }
  }
  while(SELECTOR==2)
  {
    BUTTON2_ALARM_ARM();                      // Alarm Arming Message on LCD
    seg_counter();                            // count from 10 to 0 on 4 digit display
    MOTION_SENSING();                         // Alarm Armed Motion Sensing on LCD
    digitalWrite(RED_LED,1);                  // Turn OFF RED LED
    digitalWrite(GREEN_LED,0);                // Turn ON GREEN LED
    digitalWrite(BLUE_LED,1);                  // Turn OFF BLUE LED
    SELECTOR=3;
  } 
  while(SELECTOR ==3)
  {
    attachInterrupt(digitalPinToInterrupt(sensorPin),&isr,RISING);
    while(motion_detected==0)
    {
      delay(100);
    }
   detachInterrupt(digitalPinToInterrupt(sensorPin));
   motion_detected=0;
    for(count=1; count <=5; ++count)
    {
      INTRUDER_DETECTED_WHITE();    // LCD gets INTRUDER DETECTED WHITE
      digitalWrite(RED_LED,0);      // TURN ON RED LED
      digitalWrite(GREEN_LED,0);    // TURN ON GREEN LED
      digitalWrite(BLUE_LED,0);     // TURN ON BLUE LED (WHITE)
      delay(500);                   // Wait 0.5 seconds
      INTRUDER_DETECTED_RED();      // LCD gets INTRUDER DETECTED RED
      digitalWrite(GREEN_LED,1);    // TURN OFF GREEN LED
      digitalWrite(BLUE_LED,1);     // TURN OFF BLUE LED
      delay(500);
    }
    SELECTOR=4;
    digitalWrite(RED_LED,1);        // TURN OFF RED LED
    digitalWrite(GREEN_LED,1);      // TURN OFF GREEN LED
    digitalWrite(BLUE_LED,1);       // TURN OFF BLUE LED
    irrecv.resume();                         // Receive the next value
  }
  while(SELECTOR==4)
  {
    ALARM_SOUNDING();               // LCD gets ALARM SOUNDING MESSAGE
    digitalWrite(RED_LED,0);
    digitalWrite(BLUE_LED,0);
    while (!(irrecv.decode(&results)));       // while ir results are NOT received
    decoded_value=(results.value);            // Get results.value
    if((decoded_value)!=0xFF7A85)          // while decoded value is NOT 3 from TV Remote
    {
     irrecv.resume();                         // Receive the next value
    }
   else
   {
    digitalWrite(RED_LED,1);
    digitalWrite(BLUE_LED,1);
    BUTTON1_ALARM_ON();
     SELECTOR=1;
   }
  
   }
  }
}


// FUNCTION DEINITIONS
void BUTTON1_ALARM_ON(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0,0xff,0);
  lcd.print("ARM ALARM      2");
  lcd.setCursor(0,1);
  lcd.print("ALARM OFF      4");
  digitalWrite(RED_LED,1);
  digitalWrite(GREEN_LED,0);
  digitalWrite(BLUE_LED,1);
}
void BUTTON2_ALARM_ARM(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0xff,0,0);
  lcd.print("ALARM is ARMING ");
  lcd.setCursor(0,1);
  lcd.print("Please Leave now");
}
void BUTTON3_ALARM_DISARM(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0,0xff,0);
  lcd.print("  ARM Disarmed  ");
  lcd.setCursor(0,1);
  lcd.print("2-ARM      4-OFF");
}
void BUTTON4_ALARM_OFF(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0xff,0,0xff);
  lcd.print("ALARM is ABOUT  ");
  lcd.setCursor(0,1);
  lcd.print("  to TURN OFF   ");
}
void MOTION_SENSING(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0,0xff,0);
  lcd.print("  ALARM ARMED   ");
  lcd.setCursor(0,1);
  lcd.print(" MOTION SENSING ");
}
void INTRUDER_DETECTED_WHITE(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0xff,0xff,0xff);
  lcd.print("   INTRUDER     ");
  lcd.setCursor(0,1);
  lcd.print("   DETECTED     ");
}
void INTRUDER_DETECTED_RED(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0xff,0,0);
  lcd.print("   INTRUDER     ");
  lcd.setCursor(0,1);
  lcd.print("   DETECTED     ");
}
void ALARM_SOUNDING(void)
{
  lcd.setCursor(0,0);
  lcd.setRGB(0xff,0,0xff);
  lcd.print("ALARM SOUNDING  ");
  lcd.setCursor(0,1);
  lcd.print("3- Disable Alarm");
}
void seg_counter(void)
{
  
  for (count=10;count > 0; --count)
  {
    segment.showNumberDecEx(count);
    delay(1000);
  }
    segment.showNumberDecEx(count);
    delay(1000);
    segment.clear();
 }
void DHT(void)
{
  lcd.setRGB(0,0x80,0x80);
  lcd.print("Humid% TmpC TmpF");     // print Title
  read_DHT11 = DHT11.read(DHT11PIN);  // reads 40 bit format
                                      // high 8 bits humid & low 8 bits humid 
                                      // high 8 bits temp & low 8 bits temp
                                      // plus 8 bit parity given by adding
                                      // all 32 bits together 8 bits at a time
                                      // typically low 8 bits humid
                                      // and low 8 low 8 bits temp are 0
                                      // since the DHT11 has accuracy only
                                      // to the nearest % for humidity and
                                      // to the nearest degree for temp
  humid=(float)DHT11.humidity;        // humid gets float of humidity value
  temp=(float)DHT11.temperature;      // temp gets float of deg C temperature
  fahr=(9.0/5.0)*(temp)+32.0;         // fahr is calculated from temp
  printf("\e[13;25H");                // Put cursor at (13,32)                                           
  printf("%4.1f\t%4.1f\t%4.1f",       // print humidity temp(C) temp(F)) to 1 dec place 
            humid,temp,fahr);
                                      // sprintf formats data into string array buff
                                      // 0xdf is the degree symbol for LCD
   sprintf(buff,"%3.1f%c %3.0f%c%c%3.0f%c%c", 
               humid,'%',temp,0xdf,
               'C',fahr,0xdf,'F');
   lcd.setCursor(0,1);                // set lcd cursor to col 1, row a
   lcd.print(buff);                   // print humid,temp,fahr on lcd
   lcd.setCursor(10,1);
   delay(2000);                       // wait 2 seconds between each reading
}