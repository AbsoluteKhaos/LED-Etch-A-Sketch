#include "FastLED.h"                                          // FastLED library  https://github.com/FastLED/FastLED
#define COLOR_ORDER GRB                                       // GRB for WS2812B 
#define LED_TYPE WS2812B                                      // WS2812B LED Matrix
#define NUM_LEDS 256                                          // Number of LED's

struct CRGB leds[NUM_LEDS];

#define potPin A5 // Hue
#define potPin1 A0 //Saturation
#define buttonPin1 12
#define buttonPin2 10
#define buttonPin4 8
#define buttonPin5 7


// The LED Matrix is laid out in a zigzag pattern, to utilize an x,y format, use a 2d array that maps it to the proper coordinates 
//NOTE: Because of how it is laid out x is actually y and y is x where x=0, y=0 the array value/LED effected is 0
//But on x=1 y= 0, LED is 31
byte matrix[16][16] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
                 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
                 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50 ,49, 48,
                 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83 ,82, 81, 80,
                 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
             127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112,
             128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
             159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,
             160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
             191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176,
             192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 
             223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208,
             224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
             255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, };

//Bools for interrupt    
volatile boolean firedL;
volatile boolean upL;
volatile boolean firedR;
volatile boolean upR;

#define encA 2
#define encB 4
#define INTERRUPT 0  // Pin 2

#define encC 3
#define encD 5
#define INTERRUPT1 1  // Pin 3

/* isrL() and isrR() are both adapted from https://www.gammon.com.au/forum/?id=11130
 *  which uses one rotary encoder, this code uses two
*/
//Interrupt Service Routine for a change to encoder pin A
void isrL ()
{
 if (digitalRead (encA))
   upL = digitalRead (encB);
 else
   upL = !digitalRead (encB);
 firedL = true;
}  // end of isr

void isrR ()
{
 if (digitalRead (encC))
   upR = digitalRead (encD);
 else
   upR = !digitalRead (encD);
 firedR = true;
}  // end of isr

void setup() {
  Serial.begin (115200);
  //Setting up the led matrix
  FastLED.addLeds<NEOPIXEL,11>(leds, NUM_LEDS);
  set_max_power_in_volts_and_milliamps( 5, 500); //Power management set at 5V, 500mA
  FastLED.setBrightness(8);  

  //Set up rotary encoder interupt reading
  digitalWrite (encA, HIGH);
  digitalWrite (encB, HIGH);
  attachInterrupt (INTERRUPT, isrL, CHANGE);   // interrupt is pin 2
  digitalWrite (encC, HIGH);
  digitalWrite (encD, HIGH);
  attachInterrupt (INTERRUPT1, isrR, CHANGE);   // interrupt 1 is pin 3
  
  //Set up 4 buttons
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);
  pinMode(buttonPin5, INPUT_PULLUP);
  
  delay(100);
}

//x and y are set as (-1, -1), but it wil always be forced to 0,0 at startup
int x = -1;
int y = -1;

//Bools to detect whether the left/right encoder was turned left(up) or right(down)
bool LU= false;
bool LD= false;
bool RU= false;
bool RD= false;

//ints to keep track of color settings
int color=0;
int hue=0;
int sat=0;

//Records the previous values of x and y
int oldx=0;
int oldy=0;

void loop() {  
  delay(200);
  //Read buttons and activated the related functions
  int buttonValue1 = digitalRead(buttonPin1);
  if (buttonValue1 == LOW){
     color++; //Changes color setting
     delay(10);
  }
  int buttonValue2 = digitalRead(buttonPin2);
  if (buttonValue2 == LOW){
     rainbowclear(); //Clears the matrix and resets cursor to 0,0
  }
  int buttonValue4 = digitalRead(buttonPin4);
  if (buttonValue4 == LOW){
     eraseprev(); //Erases the previous led
  }
  int buttonValue5 = digitalRead(buttonPin5);
  if (buttonValue5 == LOW){
     FastLED.clear(); //Clears the LED Matrix
  }
  
 //Lines 139-157 are both from https://www.gammon.com.au/forum/?id=11130 as well, but are modified
 // Based on interuppt, it will detect whether the left/right encoder was turned left(up) or right(down)
 //For the left encoder
  if (firedL){
    if (upL){
      LU= true;
    }
    else{
      LD= true;
    }
    firedL = false;  
  }  // end if fired
  //For the right encoder
  if (firedR){
     if (upR){
        RU= true;
     }
     else{
        RD= true;
     }
     firedR = false;
  }  // end if fire

  //If the left encoder was increased, increment y 
  //NOTE: y on the remapped 2D array acts like the x on a normal graph, 
  if (LU && !LD){
      oldx=x;
      oldy=y;
      y++;
  }
  //Decrement y otherwise
  else if(LD && !LU){
       oldx=x;
       oldy=y;
       y--;
  }
  //If the right encoder was increased, increment x
  //NOTE: x on the remapped 2D array acts like the y on a normal graph,
  if (RU && !RD){
       oldx=x;
       oldy=y;
       x++;
  }
  //Decrement x otherwise
  else if (RD && !RU){
       oldx=x;
       oldy=y;
       x--;
  }
  //Error checking, due to rotary encoders being able to increase x and y infinitely, beyond their range 0-15
  //Forces x and y to remain within 0-15
    if(x<0){
        x=0;
        oldx=x+1;
    }
     if(x>15){
        x=15;
        oldx=x-1;
    }
    if(y<0){
        y=0;
        oldy=y+1;
    }
     if(y>15){
      y=15;   
      oldy=y-1;
    }
    
   //Apply changes on LED
   displaymode(hue, sat);
   //Reset detection bools 
   LU= false;
   LD= false;
   RU= false;
   RD= false; 
   
}
//fadeall() and rainbowclear are from https://github.com/FastLED/FastLED/blob/master/examples/Cylon/Cylon.ino
//Slightly adapted to clear the matrix

//Fades the brightness of the current leds
void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

//Clears the LED matrix in a color fashion and resets the cursor to 0,0
void rainbowclear(){ 
  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = NUM_LEDS; i >0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    fadeall();
  }
  FastLED.clear(); 

  //Reset cursor and all values to 0
  x=0;
  y=0;
  oldx=0;
  oldy=0;
}

//Erases the previous x and y coordinates
void eraseprev(){
  leds[matrix[oldx][oldy]] = CRGB::Black;
}

//Displays the changes made in the loop
void displaymode(int hue, int sat){
   if(color==10){ //Reset color to 0, if color was changed by buttonPin1
    color=0;
   }
   if(color==0){
    hue = analogRead(potPin)/4;    // Analog read to get the hue value
    sat = analogRead(potPin1)/4;    // Analog read to get the saturation value
    leds[matrix[x][y]].setHSV( hue, sat, 255); //Set LED at x and y to color based on hue and sat
   }
   if(color==9){
    leds[matrix[x][y]].setHue(random(256)); //Set LED at x and y to color based on random int
   }
   else if(!color==0){
    leds[matrix[x][y]].setHue((color-1)*32); //Set LED at x and y to color based on the current value of color, which was changed by buttonPin1
   }
   FastLED.show(); //Display
}
