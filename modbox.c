/************************************************
    ____                 __                     __   __             __________  __  ___
   / __ \___ _   _____  / /___  ____  ___  ____/ /  / /_  __  __   / ____/ __ \/  |/  /
  / / / / _ \ | / / _ \/ / __ \/ __ \/ _ \/ __  /  / __ \/ / / /  / / __/ /_/ / /|_/ /
 / /_/ /  __/ |/ /  __/ / /_/ / /_/ /  __/ /_/ /  / /_/ / /_/ /  / /_/ / ____/ /  / /
/_____/\___/|___/\___/_/\____/ .___/\___/\__,_/  /_.___/\__, /   \____/_/   /_/  /_/
                            /_/                        /____/


**********************************************/
//Required Libraries
#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

//Pin Definitions
const int USER_INPUT_BUTTON         = 29; //Puff Switch
const int CHARGER_SIGNAL            = 17;
const int BATTERY_VOLTAGE_INPUT     =  6;
const int HEATER_PIN                = 25;
const int USER_ON_INDICATOR         = 20; //DEV VAR
const int VOLTAGE_OVERAGE_INDICATOR = 21; 
const int CHARGE_BATTERY_INDICATOR  = 24;
  /*************** To Do 
                        DEFINE LED PIN
const int LED = XX;
  *******************************************************************************/


//const int XR = 15;//Purple
//const int XL = 16;//Brown
//const int YU = 18;//Red
//const int YD = 19;//Yellow
#define TFT_DC    9
#define TFT_CS    10
#define TFT_RST   13
#define STMPE_CS  10

#define LIGHT_BLUE 0x03FD

//Touch Screen definitions
#define Y1 3//x+
#define Y2 16//x-
#define X1 4//y+
#define X2 19//y-
//Define your screen resolution as per your Touch screen (Max: 1024)
#define Yresolution 240 //64
#define Xresolution 320 //128

//************************************************************************************************************//
//************************************************************************************************************//
//************************************************************************************************************//
//**********                                 WARNING HIDDEN PIN DEFINITIONS                              *****//
//**********                           REQUIRED SPI PINS SCK, SS/CS, MOSI & MISO                         *****//
//**********                         Are defined by default AND HAVE BEEN CHANGED                        *****//
//**********                            See file variant.h ~lines 88-82 in                               *****//
//**********       ./Arduino/portable/packages/Simblee/hardware/Simblee/1.0.3/variants/Simblee/variant.h *****//
//******************************              GPM (Who da man, you da man)                ********************//
//************************************************************************************************************//
//************************************************************************************************************//

const unsigned char myBitmap [] PROGMEM = {
 0x07,0xff,0xff,0xff,0xff,0xff,0xff,0xfe
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x02
,0x07,0xff,0xff,0xff,0xff,0xff,0xff,0xfe
};

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//Global Variable Definitions
int userInputButtonState = 0;
int * buttonActiveZone[4] = {};

int buttonIncreaseWatts[4];
int buttonDecreseWatts[4];
int buttonSlider[4];
int buttonSliderOff[4];
int sliderActive = 0;

char * screenText = "";

float totalPuffTime = 0;
float currentVoltage;
float wattage = 1;

void setup(){
  Serial.begin(9600);
  Serial.println("====== Setup Begin ======");
  pinMode(USER_INPUT_BUTTON, INPUT_PULLUP);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(USER_ON_INDICATOR, OUTPUT);
  pinMode(CHARGE_BATTERY_INDICATOR, OUTPUT);
  pinMode(VOLTAGE_OVERAGE_INDICATOR, OUTPUT);
  pinMode(CHARGER_SIGNAL, INPUT);

  digitalWrite(HEATER_PIN, LOW);
  Simblee_pinWake(USER_INPUT_BUTTON, LOW);

  //**** Set up for TFT display & display hardware reset
  pinMode (TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);
  delay(100);
  digitalWrite(TFT_RST, LOW);
  delay(500);
  digitalWrite(TFT_RST, HIGH);
  delay(500);
  tft.begin();  
  tft.setRotation(2);//set to 4 for production
}

void loop() {

  checkCharger(CHARGER_SIGNAL); //Is plugged in
  currentVoltage        = checkVoltage(BATTERY_VOLTAGE_INPUT); //Is Voltage Acceptable
  userInputButtonState  = digitalRead(USER_INPUT_BUTTON);

  if(userInputButtonState == LOW) userInterupt();

  //tft.fillScreen(ILI9341_BLACK);
  tft.fillScreen(ILI9341_BLACK);
  delay(100);

  //Sleep & wait for user input
  Simblee_resetPinWake(USER_INPUT_BUTTON);
  Simblee_ULPDelay(INFINITE);
}

//Runs when the Input Button is pushed.
void userInterupt(){
  checkCharger(CHARGER_SIGNAL);
  /*************** To Do 
                        LED ON
  *******************************************************************************/
  int timeInterupted  = millis()/1000; //Time device has been on in seconds. Resets every ~50 days
  int timeInLoop      = timeInterupted;
  int userOnClick     = 0;

  //Setup Display
  //User Ready Screen
  screenText = "";
  drawOutputs(screenText);

  tft.drawLine(10, 260, 230, 260,ILI9341_DARKGREY);
  tft.fillRect( 0, 261, 240, 320, ILI9341_BLACK);
  tft.setCursor(50, 250);
  tft.println("Click 5x to Start");

  //Check for 5 button clicks
  //Times out after 5 seconds
  userOnClick = timerFuction(timeInLoop, 10, 6); 
  
  if(userOnClick >= 5){
    userOn();
  } else if(userOnClick < 5){
    userOff();
    /*************** To Do 
                          LED OFF
    *******************************************************************************/
  }  
}
 
void userOn(){
  int userInputButtonState  = digitalRead(USER_INPUT_BUTTON);
  int startTime             = millis()/1000;
  int userOnTime            = startTime;
  int newWattage            = wattage;
  int * touchScreenCoords;


  unsigned long heaterOnTime = 0;

  checkCharger(CHARGER_SIGNAL);
  float currentVoltage = checkVoltage(BATTERY_VOLTAGE_INPUT); 
  float newVoltage     = currentVoltage;

  /*************** To Do 
                    Add a check to ensure the user inteded to push button 5x.
                    Listen for a 6th unintnetional click.
                    Maybe wait for a click and hold.
  *******************************************************************************/

//BEGIN
//Main User on Loop
  //We hangout here until the user clicks 5x or 30 Minutes as passed 
  //Then we exit loop turn off & power down
  int userOnClick = 0;
  while( (userOnTime - startTime) < 1800){ //Time out after 30 minutes of inactivity, Steve M 7/6
    newVoltage = checkVoltage(BATTERY_VOLTAGE_INPUT);


    //Setup Display 
    //User On
    if(screenText != "LAST PUFF:"){
      screenText = "";
      tft.fillRect(0, 235, 240, 269, ILI9341_BLACK);
      screenText = "LAST PUFF:";
      drawInputs();
    }
    

    //Change Display
    //Rewrite voltage level
    if(currentVoltage != newVoltage){
      currentVoltage = newVoltage;
      delay(40);
    }


  //Begin
  //User Input Listen
    userInputButtonState = digitalRead(USER_INPUT_BUTTON);
    touchScreenCoords    = touchXY();

    if(userInputButtonState == LOW){
      int timeInterupted = millis()/1000; //Time device has been on in seconds. Resets every ~50 days
      int timeInLoop     = timeInterupted;
      
      heaterOnTime = heaterOn();
      if(heaterOnTime < 150){
        ++userOnClick;
        displayUserClick(userOnClick, 1);
      } else {
        userOnClick = 0;
        displayUserClick(999, 1);
        totalPuffTime = heaterOnTime;
        tft.fillRect(5, 25, 90, 15, ILI9341_BLACK);
        tft.setCursor(5, 37);
        drawOutput("SEC", 2, ILI9341_GREEN, 1, totalPuffTime/1000);
        if(heaterOnTime > 14500){
          delay(1000);// If last heater usage was greater than 14.5 seconds, enforce 1 second delay;
        }
      }
    } else { //If heater buton is pressed disable touch screen
      touchScreenCoords = touchXY();
      checkTouchScreenButton(buttonIncreaseWatts, touchScreenCoords, 1);
      checkTouchScreenButton(buttonDecreseWatts, touchScreenCoords, 2);
      checkTouchScreenButton(buttonSlider, touchScreenCoords, 3); 
      checkTouchScreenButton(buttonSliderOff, touchScreenCoords, 4); 
    }//END if
  //END
  //User Input Listen
    
    if(userOnClick == 5){
      break;
    }
    userOnTime = millis()/1000;
  } //END while
//END
//Main User on Loop

  userOff();
  return;

} //END userOn

void userOff(){

  tft.fillCircle(195,220,25,ILI9341_BLACK);
  tft.fillCircle(55,220,25,ILI9341_BLACK);
  tft.fillRect(0, 225, 240, 320, ILI9341_BLACK);
  tft.drawLine(10,260,230,260,ILI9341_DARKGREY);
  tft.setCursor(50, 250);
  setText("Powering Down", 4, ILI9341_WHITE, 1);
  delay(2500);
  /*************** To Do
                        LED OFF
  *******************************************************************************/
}

float checkVoltage(float inputPin){
  int timeInOverageStart;
  int timeInOverage;
  int i           = 0;
  int currentFlip = 0;

  float newVoltageReading;
  float batteryVoltage;

  batteryVoltage    = analogRead(inputPin) * (5 / 1023.0);
  newVoltageReading = batteryVoltage;

//BEGIN
//CHECK Low Voltage
  //Check if battery needs to charge, 
  //Do not allow other functions until battery has suficient power 
  if(newVoltageReading < 3){
    while(newVoltageReading < 3.7){
      newVoltageReading = analogRead(inputPin) * (5.0 / 1023.0);
      Serial.println(newVoltageReading);
      if(i == 0){
        screenText = "Warning\nCharge\nBattery";
        tft.setFont(&FreeSans24pt7b);
        tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
        tft.setCursor(0, 55);
        tft.setTextColor(ILI9341_RED);
        tft.println("Warning\nCharge\nBattery");
        //clearScreen(screenText, ILI9341_RED, 1, ILI9341_BLACK, 1);
      }

      i++;
            /*************** To Do
                                Drop into low power mode and poll input pin less often due to low votage state???
            *******************************************************************************/
     } //END while
    drawOutputs("");
  } //END if
//END
//CHECK Low Voltage

  i = 0;
//BEGIN
//Check High Voltage
  //Check for voltage overage
  //If voltage overage lasts longer the 5 sec require a special code to renable device.
  //Current special code is holding User Interupt button for 5 sec.  
  if(newVoltageReading > 4.5){
    if(i == 0){
        screenText = "Warning\nVoltage\nOverage";
        tft.setFont(&FreeSans24pt7b);
        tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
        tft.setCursor(0, 55);
        tft.setTextColor(ILI9341_RED);
        tft.println("Warning\nVoltage\nOverage");
      }

    timeInOverage = millis()/1000;
    timeInOverageStart = timeInOverage;
    while( newVoltageReading > 4.5 ){
    //BEGIN
    //Countdown timer for overage
    tft.setCursor(0, 280);
    tft.setTextSize(2);
      if( (timeInOverageStart - timeInOverage + 1)%2 == 0) {
          if(currentFlip != 1){
            currentFlip = 1;
            screenText = "";
            tft.setTextColor(ILI9341_RED);
            tft.fillRect(0, 200, 50, 100, ILI9341_BLACK);
            tft.printf("%d\n", timeInOverageStart - timeInOverage);
          }
      }
      
      if( (timeInOverageStart - timeInOverage + 1)%2 == 1){
        if(currentFlip != 2){
          currentFlip = 2;
          screenText = "";
          tft.setTextColor(ILI9341_GREEN);
          tft.fillRect(0, 200, 50, 100, ILI9341_BLACK);
          tft.printf("%d\n", timeInOverageStart - timeInOverage);
        }
      }
    //END
    //Countdown timer for overage


      newVoltageReading = analogRead(inputPin) * (5.0 / 1023.0);
      timeInOverageStart = millis()/1000;
      
      if(timeInOverageStart - timeInOverage > 5){ //5 second wait time for voltage overage death specified by John N. 7/5/16
        overageDeath();
        break;
      }      

    }//END while
    drawOutputs("");
  }//END if
    return newVoltageReading;
}

void overageDeath(){
  int timeInReanimate = 0;
  int reanimateStart;
  int reanimate = 0;
  int i = 0;
  int y = 0;

  screenText = "Overage\nDeath!!\nContact Customer Support";
  clearScreen(screenText, ILI9341_RED, 5, ILI9341_BLACK, 1);
  tft.setFont(&FreeSans9pt7b);
  tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
  tft.setCursor(0, 55);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  tft.println("Warning\nVoltage\nDEATH\nContact");
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_GREEN);
  tft.println("Whitecloud Customer support\n1 800-491-CIGS (2447)");
  tft.println("customersupport@whitecloudcigs.com");
  while(reanimate == 0){
    userInputButtonState = digitalRead(USER_INPUT_BUTTON);
    if(userInputButtonState == LOW){
      timeInReanimate = reanimateStart = millis()/1000;
      while(userInputButtonState == LOW){
        if(timeInReanimate - reanimateStart > 5){
          reanimate = 1;
          y = 0;
          break;
        } else {
          y = 1;
        }

        timeInReanimate = millis()/1000;

        if(i == 0 ){
          tft.setTextSize(1);
          tft.println("REANIMATING!");
        }

     
        i++;
         userInputButtonState = digitalRead(USER_INPUT_BUTTON);


      } //END while

      i = 0;
      if(y == 1){
        tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
        tft.setCursor(0, 55);
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_GREEN);
        tft.println("Reanimating\nFailed!\nOverage Death!!");
      }

      //y++;
    }//END if
              /*************** To Do
                              Define escape from death.
            *******************************************************************************/
  }//END while

  tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
  tft.setCursor(0, 55);
  tft.setTextColor(ILI9341_GREEN);
  tft.println("It's\nALIVE!\nRe-initializing");
  screenText = "It's\nALIVE!\nRe-initializing";
  clearScreen(screenText, ILI9341_GREEN, 5, ILI9341_BLACK, 1);
  delay(1500);
}

void checkCharger(int chargerSignal){
  int chargerState = digitalRead(chargerSignal);
  if( chargerState == 1){
        tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
        tft.setCursor(0, 55);
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_GREEN);
        tft.println("Charging, wait to vape");
        screenText = "Charging, wait to vape";
    
    while( chargerState == 1){
      //wait here while charger is plugged in.
      chargerState = digitalRead(chargerSignal);
    }
  }
}

int timerFuction(int timeInterupted, int waitTime, int maxClicks){
  int timeInLoop = timeInterupted;
  int userOnClick = 1;

  while( (timeInLoop - timeInterupted) < waitTime && (userOnClick < maxClicks) ){
     /*************** To Do
                      Add voltage check here. 
                      Issue: Doing so screws up the display flow.
      *******************************************************************************/

    timeInLoop = millis()/1000;
    delay(40); //Dealy is to prevents reading extra clicks. Kind of a debounce effect.
    if(digitalRead(USER_INPUT_BUTTON) == LOW){
      displayUserClick(userOnClick, 0);
      userOnClick++;
    } 
    while(digitalRead(USER_INPUT_BUTTON) == LOW){
      //Wait for user to release button
          /*************** To Do
                           Add a check to prevent user from pressing and holding button for eternity.
          *******************************************************************************/
    }
  } //END while

  return userOnClick;
}

float heaterOn(){
  unsigned long startTime = millis();
  unsigned long heaterOnTime = startTime;
  checkCharger(CHARGER_SIGNAL);
  //Shutdown heater after 15 seconds
  while( ( ( heaterOnTime - startTime ) < 15000 )  && digitalRead(USER_INPUT_BUTTON) == LOW ) {
    checkVoltage(BATTERY_VOLTAGE_INPUT);
    digitalWrite(HEATER_PIN, HIGH);
    heaterOnTime = millis();
  }
  
  digitalWrite(HEATER_PIN, LOW);
  return heaterOnTime - startTime;

}


/******* 
*  Functions for LCD display
*/

//Changes color of power icon depending on user clicks.
void displayUserClick(int userClicks, int isOn){

  //Reverse color sequence for On & Off
  if(isOn == 1 && userClicks < 50){
    userClicks = 6 - userClicks;
  }
  
  switch (userClicks) {
      case 1:
        if(isOn == 0){
          tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_MAGENTA);
        } else {
          tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_RED);
        }
        break;
      case 2:
        tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_YELLOW);
        break;
      case 3:
        tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_PURPLE);
        break;
      case 4:
        tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_BLUE);
        break;
      case 5:
        if(isOn == 1){
          tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_PINK);
        }
        break;
      case 999://RESET case
        tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_GREEN);
        break;
      default:
        tft.println('ERROR');
        
  }
}

void clearScreen(char * text, int textColor, int textSize, int backgroundColor, int resetCursor){

  if(resetCursor == 1){
    tft.setCursor(0, 0);
  }  

  tft.fillScreen(backgroundColor);
  tft.setTextColor(textColor);
  tft.setTextSize(textSize);
}

void clearRect(char * text, int textColor, int textSize, int backgroundColor, int x, int y, int width, int height){
  int xCenter = x + (width/3);
  int yCenter = y + (height/2); 

  tft.fillRect(x, y, width, height, backgroundColor);
  tft.setTextColor(textColor);
  tft.setTextSize(textSize);
  tft.setCursor(xCenter, yCenter);
  tft.println(text);     
}

void clearCircle(char * text, int textColor, int textSize, int backgroundColor, int x, int y, int width, int height){
  tft.fillRect(x, y, width, height, backgroundColor);
  tft.setTextColor(textColor);
  tft.setTextSize(textSize);
  tft.setCursor(x, y);
  tft.println(text);     
}

void drawOutput(char * text, int fontType, uint16_t color, int size, float value){
  if (fontType == 0){
    tft.setFont(&FreeSans24pt7b);
  } else if (fontType == 1){
    tft.setFont(&FreeSansBold9pt7b);
  } else if( fontType == 2){
    tft.setFont(&FreeSerifItalic9pt7b);
  } else if( fontType == 4){
    tft.setFont(&FreeSans9pt7b);
  }

  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.printf("%f %s ", value, text);
}

void drawWattage(float currentWattage, float lastWattage){

  int intpart           = (int)currentWattage;
  int lastIntpart       = (int)lastWattage;
  
  int decpart           = 10 * (currentWattage - intpart);
  int onesPlace         = intpart % 10;
  int tensPlace         = (int)(intpart/10);
  int hundredsPlace     = (int)(intpart/100);

  int lastDecpart       = 10 * (lastWattage - lastIntpart);
  int lastOnesPlace     = lastIntpart % 10;
  int lastTensPlace     = (int)(lastIntpart/10);
  int lastHundredsPlace = (int)(lastIntpart/100);

  int cursorPosition    = 65;

  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(1);
  tft.setFont(&FreeSans24pt7b);
  
  if(tensPlace > 9){
      tensPlace = tensPlace - 10; 
      if(tensPlace == 10) tensPlace = 0;
  } 

  if(lastTensPlace > 9){
      lastTensPlace = lastTensPlace - 10; 
      if(lastTensPlace == 10) lastTensPlace = 0;
  }

   
  
  if(hundredsPlace == 0) cursorPosition = 85;
  if(hundredsPlace == 0 && tensPlace == 0) cursorPosition = 90;

  if(lastWattage == NULL){
    if(hundredsPlace == 0 && tensPlace == 0){
       tft.fillRect(cursorPosition - 20, 122, 115, 35, ILI9341_BLACK);
       tft.setCursor(cursorPosition, 155);
       tft.printf("%d.%d", onesPlace, decpart);
       return;
    } else if(hundredsPlace == 0 && tensPlace > 0){
      tft.fillRect(cursorPosition, 122, 90, 35, ILI9341_BLACK);
      tft.setCursor(cursorPosition, 155);
      tft.printf("%d%d.%d", tensPlace, onesPlace, decpart);
      return;
    } else if(hundredsPlace > 0){
      tft.setCursor(cursorPosition, 155);
      tft.fillRect(cursorPosition, 122, 115, 35, ILI9341_BLACK);
      tft.printf("%d%d%d.%d", hundredsPlace, tensPlace, onesPlace, decpart);
      return;
    }
  } else {
    if(hundredsPlace == 0 && tensPlace == 0){
        if(lastTensPlace!= tensPlace){
           tft.fillRect(cursorPosition - 20, 122, 115, 35, ILI9341_BLACK);
           tft.setCursor(cursorPosition, 155);
           tft.printf("%d.%d", onesPlace, decpart);
           return;
        }
      tft.setCursor(cursorPosition, 155);
      if(onesPlace != lastOnesPlace){
        tft.fillRect(cursorPosition, 122, 25, 35, ILI9341_BLACK);
        tft.setCursor(cursorPosition, 155);
        tft.printf("%d", onesPlace);
      }
      if(lastDecpart != decpart){
        tft.fillRect(cursorPosition + 40, 122, 25, 35, ILI9341_BLACK);
        tft.setCursor(cursorPosition + 40, 155);
        tft.printf("%d", decpart);
      }
    } else if(hundredsPlace == 0 && tensPlace > 0){
          if(lastHundredsPlace != hundredsPlace){
            tft.fillRect(cursorPosition - 20, 122, 115, 35, ILI9341_BLACK);
        }

      if(lastTensPlace != tensPlace){
        tft.fillRect(cursorPosition, 122, 90, 35, ILI9341_BLACK);
        tft.setCursor(cursorPosition, 155);
        tft.printf("%d%d.%d", tensPlace, onesPlace, decpart);
        return;
      }


      if(lastDecpart != decpart){
        tft.fillRect(cursorPosition + 65, 122, 25, 35, ILI9341_BLACK);
        tft.setCursor(cursorPosition + 65, 155);
        tft.printf("%d", decpart);
      }

      if(onesPlace != lastOnesPlace){
        tft.fillRect(cursorPosition + 25, 122, 25, 35, ILI9341_BLACK);
        tft.setCursor(cursorPosition + 25, 155);
        tft.printf("%d", onesPlace);
      }


    } else if(hundredsPlace > 0){
        Serial.printf("CurrentWattage = %f | LastWattage = %f \n", currentWattage, lastWattage);
        if(lastHundredsPlace != hundredsPlace){
          tft.setCursor(cursorPosition, 155);//+25
          tft.fillRect(cursorPosition, 122, 115, 35, ILI9341_BLACK);
          tft.printf("%d%d%d.%d", hundredsPlace, tensPlace, onesPlace, decpart);
          return;
        }

        if(lastDecpart != decpart){
          tft.fillRect(cursorPosition + 90, 122, 25, 35, ILI9341_BLACK);
          tft.setCursor(cursorPosition + 90, 155);
          tft.printf("%d", decpart);
        }

        if(onesPlace != lastOnesPlace){
          tft.fillRect(cursorPosition + 52, 122, 25, 35, ILI9341_BLACK);
          tft.setCursor(cursorPosition + 52, 155);
          tft.printf("%d", onesPlace);

        if(lastTensPlace != tensPlace){
          tft.fillRect(cursorPosition + 25, 122, 25, 35, ILI9341_BLACK);
          tft.setCursor(cursorPosition + 25, 155);
          tft.printf("%d", tensPlace);
        }
      }
    }
  }
}

void setText(char * text, int fontType, uint16_t color, int size){
   if (fontType == 0){
    tft.setFont(&FreeSans24pt7b);
  } else if (fontType == 1){
    tft.setFont(&FreeSansBold9pt7b);
  } else if( fontType == 2){
    tft.setFont(&FreeSerifItalic9pt7b);
  } else if( fontType == 4){
    tft.setFont(&FreeSans9pt7b);
  }

  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.printf(text);
}


void drawOutputs(char *screenText){
  clearScreen(screenText, ILI9341_GREEN, 2, ILI9341_BLACK, 1);
  
  tft.setCursor(5, 15);
  setText("LAST PUFF:", 1, ILI9341_WHITE, 1);

  tft.setCursor(5, 37);
  drawOutput("SEC", 2, ILI9341_GREEN, 1, totalPuffTime/1000);

  tft.setCursor(5, 60);
  setText("OHMS:", 1, ILI9341_WHITE, 1);

  tft.setCursor(5, 75);
  drawOutput("V", 2, ILI9341_GREEN, 1, currentVoltage);

  tft.setCursor(5, 100);
  setText("VOLTS:", 1, ILI9341_WHITE, 1);

  tft.setCursor(5, 120);
  drawOutput("", 2, ILI9341_GREEN, 1, currentVoltage);

  tft.fillCircle(124,150,68,LIGHT_BLUE);
  tft.fillCircle(124,150,65,ILI9341_BLACK);
  
  tft.setCursor(90, 185);
  setText("WATTS", 1, ILI9341_WHITE, 1);

  drawWattage(wattage, NULL);
  tft.setCursor(168, 23);
  drawOutput("%", 4, ILI9341_WHITE, .5, (int)currentVoltage);
  tft.drawBitmap(160, 5, myBitmap, 63, 25, ILI9341_GREEN);
  tft.setCursor(170, 7);
  tft.setCursor(100, 75);
 
 }



void drawInputs(){

  //Wattage Circles
  drawButton("", ILI9341_WHITE, 1, ILI9341_DARKGREY, 40, 205, 33, 35, &buttonDecreseWatts[0]);
  tft.fillCircle(55,220,25,LIGHT_BLUE);
  tft.fillCircle(55,220,23,ILI9341_BLACK);
  tft.drawLine(40, 220, 70, 220, ILI9341_DARKGREY);
  tft.drawLine(40, 221, 70, 221, ILI9341_DARKGREY);
  tft.drawLine(40, 222, 70, 222, ILI9341_DARKGREY);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(43, 213);
  tft.setTextSize(5);

  drawButton("", ILI9341_WHITE, 1, ILI9341_DARKGREY, 180, 205, 33, 35, &buttonIncreaseWatts[0]);
  tft.fillCircle(195,220,25,LIGHT_BLUE);
  tft.fillCircle(195,220,23,ILI9341_BLACK);
  tft.drawLine(180, 220, 210, 220, ILI9341_DARKGREY);
  tft.drawLine(180, 221, 210, 221, ILI9341_DARKGREY);
  tft.drawLine(180, 222, 210, 222, ILI9341_DARKGREY);
  tft.drawLine(194, 205, 194, 238, ILI9341_DARKGREY);
  tft.drawLine(195, 205, 195, 238, ILI9341_DARKGREY);
  tft.drawLine(196, 205, 196, 238, ILI9341_DARKGREY);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(163, 213);
  tft.setTextSize(5);


  tft.drawLine(10,260,230,260,ILI9341_DARKGREY);


  //Slider Circles
  drawButton("", ILI9341_WHITE, 1, ILI9341_BLACK, 0, 261, 59, 80, &buttonSlider[0]);
  tft.fillCircle(30,290,20,ILI9341_DARKGREY);
  tft.fillCircle(30,290,18,ILI9341_BLACK);
  tft.drawLine(35, 285, 40, 290, ILI9341_DARKGREY); //Top Arrow
  tft.drawLine(20, 290, 40, 290, ILI9341_DARKGREY); //Main Line
  tft.drawLine(35, 295, 40, 290, ILI9341_DARKGREY); //bottom arrow
  drawButton("", ILI9341_WHITE, 1, ILI9341_BLACK, 185, 261, 59, 80, &buttonSliderOff[0]);
  tft.fillCircle(210,290,20,ILI9341_DARKGREY);
  tft.fillCircle(210,290,18,ILI9341_BLACK);
}

void drawButton(char * text, int textColor, int textSize, int backgroundColor, int x, int y, int width, int height, int* button){

  clearRect(text, textColor, textSize, backgroundColor, x, y, width, height);
  button[0] = x;
  button[1] = y;
  button[2] = x + width;
  button[3] = y + height;
}

/******* 
*  Functions for touch screen
*/


int * touchXY(){
  int X,Y; //Touch Coordinates are stored in X,Y variable
  static int coords[2];
   pinMode(Y1,INPUT);
   pinMode(Y2,INPUT);  
   digitalWrite(Y2,LOW);
   pinMode(X1,OUTPUT);
   digitalWrite(X1,HIGH);
   pinMode(X2,OUTPUT);
   digitalWrite(X2,LOW);
   X = (analogRead(Y1))/(1024/Xresolution); //Reads X axis touch position
  
   pinMode(X1,INPUT);
   pinMode(X2,INPUT);
   digitalWrite(X2,LOW);
   pinMode(Y1,OUTPUT);
   digitalWrite(Y1,HIGH);
   pinMode(Y2,OUTPUT);
   digitalWrite(Y2,LOW);
   Y = Yresolution - ((analogRead(X1))/(1024/Yresolution)); //Reads Y axis touch position

   coords[0] = X;
   coords[1] = Y;
   return coords;
}

          /*************** To Do
                           Ask Jon how to increase wattage.
          *******************************************************************************/
void checkTouchScreenButton(int * button, int * coords, int buttonId){
  int x = 0;
  int lastX = coords[1];
  int y = 0;
  int holdingButton = 0;
  int * newCoords;

  float currentWattage = wattage;
  float newWattage = currentWattage;

  if(button[0] < coords[1] && coords[1] < button[2]) x = 1;
  if(button[1] < coords[0] && coords[0] < button[3]) y = 1;

//BEGIN
//Check for button 1, increse wattage. 
  if(x == 1 && y == 1 && buttonId == 1 && sliderActive == 1){
    holdingButton = 1;
    while(holdingButton == 1){
      //If the user increseses wattage by more then 3, begin to increment up by 1.0  
      if(newWattage - currentWattage > 3){
        newWattage = increaseWattage(1);
      } else {
        newWattage = increaseWattage(.1);
      }

      newCoords = touchXY();
      (button[0] < newCoords[1] && newCoords[1] < button[2]) ? x = 1 : x = 0;
      (button[1] < newCoords[0] && newCoords[0] < button[3]) ? y = 1 : y = 0;
      if(x != 1 || y != 1){
        holdingButton = 0;
      }
    }
  }
//END
//Check for button 1, increse wattage.

//BEGIN
//Check for button 2, decrese wattage.
  if(x == 1 && y == 1 && buttonId == 2 && sliderActive == 1){
    holdingButton = 1;
    while(holdingButton == 1){
      //If the user increseses wattage by more then 3, begin to decrement up by 1.0  
      if(currentWattage - newWattage < 3){
        newWattage = decreaseWattage(.1);
      } else {
        newWattage = decreaseWattage(1);
      }
      newCoords = touchXY();
      (button[0] < newCoords[1] && newCoords[1] < button[2]) ? x = 1 : x = 0;
      (button[1] < newCoords[0] && newCoords[0] < button[3]) ? y = 1 : y = 0;
      if(x != 1 || y != 1){
        holdingButton = 0;
      }
    }
  }
//END
//Check for button 2, decrese wattage.
//BEGIN
//Check for button 3, slider.
  if(x == 1 && y == 1 && buttonId == 3){
    while(y == 1){
      newCoords = touchXY();
      (button[1] < newCoords[0] && newCoords[0] < button[3]) ? y = 1 : y = 0;
      if(newCoords[1] >= 170){
        //Wattage -
        tft.fillCircle(55,220,23,ILI9341_BLACK);
        tft.drawLine(40, 220, 70, 220, ILI9341_WHITE);
        tft.drawLine(40, 221, 70, 221, ILI9341_WHITE);
        tft.drawLine(40, 222, 70, 222, ILI9341_WHITE);
              
        //Wattage +
        tft.fillCircle(195,220,23,ILI9341_BLACK);
        tft.drawLine(180, 220, 210, 220, ILI9341_WHITE);
        tft.drawLine(180, 221, 210, 221, ILI9341_WHITE);
        tft.drawLine(180, 222, 210, 222, ILI9341_WHITE);
        tft.drawLine(194, 205, 194, 238, ILI9341_WHITE);
        tft.drawLine(195, 205, 195, 238, ILI9341_WHITE);
        tft.drawLine(196, 205, 196, 238, ILI9341_WHITE);

        sliderActive = 1;
        break;
      }
      if(abs(lastX - newCoords[1]) > 15){
        break;
      }
      lastX = newCoords[1];
    }
  }
//END
//Check for button 3, slider..

//BEGIN
//Check for button 4, sliderOff.
  if(x == 1 && y == 1 && buttonId == 4){
    while(y == 1){
      newCoords = touchXY();
      
      (button[1] < newCoords[0] && newCoords[0] < button[3]) ? y = 1 : y = 0;
      if(newCoords[1] <= 45){
        //Wattage -
        tft.drawLine(40, 220, 70, 220, ILI9341_DARKGREY);
        tft.drawLine(40, 221, 70, 221, ILI9341_DARKGREY);
        tft.drawLine(40, 222, 70, 222, ILI9341_DARKGREY);

        //Wattage +
        tft.drawLine(180, 220, 210, 220, ILI9341_DARKGREY);
        tft.drawLine(180, 221, 210, 221, ILI9341_DARKGREY);
        tft.drawLine(180, 222, 210, 222, ILI9341_DARKGREY);
        tft.drawLine(194, 205, 194, 238, ILI9341_DARKGREY);
        tft.drawLine(195, 205, 195, 238, ILI9341_DARKGREY);
        tft.drawLine(196, 205, 196, 238, ILI9341_DARKGREY);

        sliderActive = 0;
        break;
      }
      if(abs(lastX - newCoords[1]) > 10){
        break;
      }
      lastX = newCoords[1];
    }
  }
//END
//Check for button 4, sliderOff..
}

float increaseWattage(float increment){
  float oldWattage;
  if(wattage <= 200){
    oldWattage = wattage;
    wattage = wattage + increment;
    if(wattage >= 200) wattage = 200;
    drawWattage(wattage, oldWattage);
  }
  delay(50);
  return wattage;
}

float decreaseWattage(float increment){ 
  float oldWattage;
  if(wattage > 1.1){
    oldWattage = wattage;
    wattage = (wattage - increment);
    if(wattage < 1) wattage = 1;
    drawWattage(wattage, oldWattage);
  }
  delay(50);
  return wattage;
}
