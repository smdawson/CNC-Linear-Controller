/*
A program for controlling a single stepper motor driving a rotary table.
Uses a 4x4 matrix keypad for entry of degrees and direction or number of divisions to move the table.
Serial I2C display, Pololu stepper driver.
*/


#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'.','0','#','D'}
};

byte rowPINS[ROWS] = {11,10,9,8};
byte colPINS[COLS] = {7,6,5,4};

Keypad kpd = Keypad(makeKeymap(keys),rowPINS,colPINS, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x20 for a 16 chars and 2 line display

 //setup vars
const int stp = 12;  //connect pin 12 to step
const int dir = 13;  // connect pin 13 to dir
const int StepsPerRotation = 400; //Set Steps per rotation of stepper
const int TableRatio = 72; //ratio of rotary table
const int Multiplier = (StepsPerRotation * TableRatio)/360;
const int stepdelay = 1;
float Degrees = 0;                //Degrees from Serial input
float ToMove = 0;                 //Steps to move
float Divisions;
float current = 0;
int Mode = 0; 
 

void setup()
{
  lcd.begin();                      // initialize the lcd 
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT); 
 
  // Print welcome message to the LCD.
  lcd.backlight();
  lcd.print("Rotarye Control");
  lcd.setCursor(0,2);
  lcd.print("  CrankyTechGuy CNC");
  lcd.setCursor(0,3);
  lcd.print("   Copyright 2014");
  delay(2000);
  lcd.begin();
  Mode = GetMode();
}

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
asm volatile ("  jmp 0");  
} 

void rotation(float tm, int d)
{   
  if(d == 0)
  {
    digitalWrite(dir, LOW);
  }
  else
  {
    digitalWrite(dir, HIGH);
  }
  
  for(int i = 0; i < tm; i++)  
   {    
    digitalWrite(stp, HIGH);   
    delay(stepdelay);               
    digitalWrite(stp, LOW);  
    delay(stepdelay);              
   }
}

float GetNumber()
{
   float num = 0.00;
   float decimal = 0.00;
   float decnum = 0.00;
   int counter = 0;
   char key = kpd.getKey();
   lcd.setCursor(0,0);lcd.print("Enter degrees then");lcd.setCursor(0,1);lcd.print("    press [#].");
   lcd.setCursor(0,3);lcd.print("Reset [D]");
   lcd.setCursor(8,2);
   bool decOffset = false;

   while(key != '#')
   {
      switch (key)
      {
         case NO_KEY:
            break;
            
         case '.':
           if(!decOffset)
           {
             decOffset = true;
           }
            lcd.print(key);
            break;   
           
         case '0': case '1': case '2': case '3': case '4':
         case '5': case '6': case '7': case '8': case '9':
           if(!decOffset)
           {
            num = num * 10 + (key - '0');
            lcd.print(key);
           }
           else if((decOffset) && (counter <= 1))
           {
            num = num * 10 + (key - '0');
            lcd.print(key);
            counter++;
           }
           break;

         case 'D':
           software_Reset();
         break;
      }

      decnum = num / pow(10, counter);
      key = kpd.getKey();
   }
  return decnum;
}

float GetDivisions()
{
   float num = 0.00;
   char key = kpd.getKey();
   lcd.clear();
   lcd.setCursor(0,0);lcd.print("Enter Divisions then");lcd.setCursor(0,1);lcd.print("     press [#].");
   lcd.setCursor(0,3);lcd.print("Reset [D]");
   lcd.setCursor(8,2);

   while(key != '#')
   {
      switch (key)
      {
         case NO_KEY:
            break;
            
         case '0': case '1': case '2': case '3': case '4':
         case '5': case '6': case '7': case '8': case '9':
            num = num * 10 + (key - '0');
            lcd.print(key);
            break;
        
        case 'D':
          software_Reset();
          break;
      }
      key = kpd.getKey();
     // num = 360/num;
   }
  return num;
}

int GetMode()
{
  int mode = 0;
  lcd.setCursor(0,1);lcd.print("Select Op Mode");
  
  lcd.setCursor(0,3);
  lcd.print("    DIV[A] DEG[B]");
  while(mode == 0)
  {
  char key = kpd.getKey();
  if(key == 'A')
  {
    mode = 1;
  }
  else if(key == 'B')
  {
    mode = 2;
  }
  }
     
lcd.clear();  
return mode;
}

void loop()
{
  if(Mode == 1)
  {
    Divisions = GetDivisions();
    Degrees = (360/Divisions);
  }
  if(Mode == 2)
  {
    Degrees = GetNumber();
  }
    lcd.clear();
    lcd.setCursor(0,3);
    lcd.print("FWD[A] REV[B] CAN[C]");
    char key = kpd.getKey();
    while(key != 'C')
    {
      lcd.setCursor(0,0);lcd.print("POS:");lcd.print(current);lcd.setCursor(0,1);lcd.print("DPM:");lcd.print(Degrees);
      key = kpd.getKey();
      if(key == 'A')
      {
       if(current >= 360)
       {
         current = (current + Degrees)-360;
       } else {
       current = current + Degrees;
       }
       ToMove = Degrees*Multiplier;
       lcd.setCursor(0,2);
       lcd.print("       Moving       ");
       rotation(ToMove,2);
       lcd.setCursor(0,2);lcd.print("                    ");
       lcd.setCursor(4,0);lcd.print("      ");
      }
      if(key == 'B')  
      {
       if(current <= 0)
       {
         current = 360+(current - Degrees);
       } else {
       current = current - Degrees;
       }
       ToMove = Degrees*Multiplier;
       lcd.setCursor(0,2);
       lcd.print("       Moving       ");
       rotation(ToMove,0);
       lcd.setCursor(0,2);lcd.print("                    ");
       lcd.setCursor(4,0);lcd.print("      ");
       }
     }
     lcd.clear();    
}
