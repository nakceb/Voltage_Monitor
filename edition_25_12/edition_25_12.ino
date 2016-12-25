/*Voltage monitor for 4 cell Lithium 3,65-2,5  Voltage batteries in series
V.01.00 Filip Stenbeck
Display: Set A4 SDA, A5 SCL for the arduino nano
Cell_1 is the first cell in the series, common ground to the arduino
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LCD.h>

//////////////////////////////////////////////////////////////////////////
//Set your inputs on the according inputs(A0-A4)
#define Cell_1 A0
#define Cell_2 A1
#define Cell_3 A2
#define Cell_4 A3
//////////////////////////////////////////////////////////////////////////
//buzzer output pin
#define BUZZERPIN 7
#define TRANSISTORPIN 9
#define DISPLAYPIN 3
#define PULLUP 4

//////////////////////////////////////////////////////////////////////////
//Set dangerous low and top value
float LOW_VALUE = 3.0;
float HIGH_VALUE = 3.60;

//////////////////////////////////////////////////////////////////////////
//Put in your resistances in ohms for more accurate readings, 
/*
int R21 = 983;     
int R22 = 976;   
int R31 = 1488;
int R32 = 976;
int R41 = 1969;
int R42 = 976;
*/
double R21 = 997;     
double R22 = 1000;   
double R31 = 1002;
double R32 = 469;
double R41 = 1000;
double R42 = 475;

int i=0;           // counter

//int DelayTime=10;     //delaytime for transistors in milliseconds;
/////////////////////////////////////////////////////////////////////////
//introducing arrays and pointers to certain arrays 

float Voltage_Array[4]={0,0,0,0};
float Voltage_Real_Array[4]={0,0,0,0};
int readArray[4]={Cell_1,Cell_2,Cell_3,Cell_4};

static float *VAptr=Voltage_Array;     //Pointer to the voltage array;
static float *VRAptr=Voltage_Real_Array; //Pointer to the voltage Real array;
static int *RAptr=readArray;

boolean DisplayFlag=0;
boolean ALARMFLAG=0;

//////////////////////////////////////////////////////////////////////////
//for the display

#define I2C_ADDR 0x27 // <<- I2C adress might change for each display(prob not).
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

////////////////////////////////////////////////////////////////////////////

void setup() {
  
  TCCR1A=0;
  TCCR1A|=(0<<COM1A1);
  TCCR1B=0;
  TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS12)|(1<<CS10);
  ICR1=0xFFFF;                        //sets the counter top value, approx 4 sec beetween readings
                 
  initLCD();                          //initiate the display
  initPins();                         //initiate the inputs
  TIMSK1|=(1<<OCIE1A);                //enables interupts on chanel A 
    
  Serial.begin(9600);               //for debugging purposes
  digitalWrite(PULLUP,HIGH);
  
  ReadVoltages(VAptr,RAptr);          //Dispaly right away
  CalculateVoltages(VAptr,VRAptr);
   lcd.setCursor (0, 0);
   lcd.print("LiFePo4 Meeter");
   lcd.setCursor (0, 1);
   lcd.print("version 1.0");
   delay(2000);
   lcd.clear();
  
  
  
}

void loop() {
  
  DisplayFlag=digitalRead(DISPLAYPIN);         //Display ON if pin 2 is HIGH
  
  if(DisplayFlag==1 || ALARMFLAG==1){// && ALARMFLAG!=1
      lcd.display();
      lcd.setBacklight(HIGH);
      tone(BUZZERPIN,1000);
      Display_Voltages(VRAptr);           //displays the true voltages from the VRAptr pointer
    }
    
  if(DisplayFlag!=1 && ALARMFLAG!=1){
      lcd.noDisplay();
      lcd.setBacklight(LOW);
    }
  
  if (ALARMFLAG && DisplayFlag!=1){
    //SOUNDTHEALARM(VRAptr);
    tone(BUZZERPIN,1000);
    //digitalWrite(BUZZERPIN,HIGH);
    lcd.display();
    lcd.setBacklight(HIGH);
    Display_Voltages(VRAptr);
  }
  else if(ALARMFLAG!=1 || DisplayFlag==1){
    digitalWrite(BUZZERPIN,LOW);
  }
  
}

//INTERUPT FUNCTION!!

ISR(TIMER1_COMPA_vect){
  ReadVoltages(VAptr,RAptr);          //Reads the voltages on the analog pins saves to the array VAptr points at
  CalculateVoltages(VAptr,VRAptr);    //Calculates the voltages for each cell saves to the array VRAptr points at
}


void initLCD() {
  lcd.begin (16, 2);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
}

void initPins() {
  pinMode(Cell_1, INPUT);
  pinMode(Cell_2, INPUT);
  pinMode(Cell_3, INPUT);
  pinMode(Cell_4, INPUT);
  pinMode(BUZZERPIN,OUTPUT);
  pinMode(TRANSISTORPIN,OUTPUT);
  pinMode(PULLUP,OUTPUT);
}

void ReadVoltages(float* ptr1, int* ptr2){   //increments the pointer and updates the voltage_Array
  //digitalWrite(TRANSISTORPIN,HIGH);
  //delay(DelayTime);
  for (i=0;i<4;i++){
    *ptr1=analogRead(*ptr2);
    ptr1++,ptr2++;
  }
  //digitalWrite(TRANSISTORPIN,LOW);
}

void CalculateVoltages(float* ptr1,float* ptr2) {    //stegar fram i pointersen som pekar på spänningsvärderna, sparar i V_A, räknas ut till V_R_A som skrivs till platserna i VRAptr
  float Voltage_Real_Array[4];
  float Voltage_Array[4];
  for (i=0;i<4;i++){                            
      Voltage_Array[i]=*ptr1;
      ptr1++;
    }
  Voltage_Real_Array[0] = Voltage_Array[0] * 5 / 1024;
  Voltage_Real_Array[1] = Voltage_Array[1] * (R21 + R22) / R22 * 5 / 1024 - Voltage_Array[0] * 5 / 1024;
  Voltage_Real_Array[2] = Voltage_Array[2] * (R31 + R32) / R32 * 5 / 1024 - Voltage_Array[1] * (R21 + R22) / R22 * 5 / 1024;
  Voltage_Real_Array[3] = Voltage_Array[3] * (R41 + R42) / R42 * 5 / 1024 - Voltage_Array[2] * (R31 + R32) / R32 * 5 / 1024;
  
 
  
  //Voltage_Real_Array[2] = Voltage_Array[2] * (R31 + R32) / R32 * 5 / 1024 - Voltage_Real_Array[1] - Voltage_Real_Array[0];
  //Voltage_Real_Array[3] = Voltage_Array[3] * (R41 + R42) / R42 * 5 / 1024 - Voltage_Real_Array[2] - Voltage_Real_Array[1] - Voltage_Real_Array[0];
  
  for (i=0;i<4;i++){
    *ptr2=Voltage_Real_Array[i];
    ptr2++;
  }
  if (Voltage_Real_Array[0]<LOW_VALUE||Voltage_Real_Array[1]<LOW_VALUE||Voltage_Real_Array[2]<LOW_VALUE||Voltage_Real_Array[3]<LOW_VALUE){//>HIGH_VALUE
    ALARMFLAG=1;
  }
  else{//(LOW_VALUE<Voltage_Real_Array[0]||Voltage_Real_Array[1]||Voltage_Real_Array[2]||Voltage_Real_Array[3]<HIGH_VALUE)
    ALARMFLAG=0;
  }
    Serial.println(ALARMFLAG);
}

void Display_Voltages(float* ptr2) {
  for (i=0;i<4;i++){
    Voltage_Real_Array[i]=*ptr2;
    ptr2++;
  }
  //lcd.clear();
  lcd.setCursor (0, 0);
  lcd.print("1:");
  lcd.setCursor (2, 0);
  lcd.print(Voltage_Real_Array[0],3);
  lcd.setCursor (6, 0);
    //lcd.print("v");
  
  lcd.setCursor (8, 0);
  lcd.print("2:");
  lcd.setCursor (10, 0);
  lcd.print(Voltage_Real_Array[1],3);
  lcd.setCursor (14, 0);
    //lcd.print("v");
  
  lcd.setCursor (0, 1);
  lcd.print("3:");
  lcd.setCursor (2, 1);
  lcd.print(Voltage_Real_Array[2],3);
  lcd.setCursor (6, 1);
  //  lcd.print("v");
  
  lcd.setCursor (8, 1);
  lcd.print("4:");
  lcd.setCursor (10, 1);
  lcd.print(Voltage_Real_Array[3],3);
  lcd.setCursor (14, 1);
  //lcd.print("v");

}
void SOUNDTHEALARM(float* ptr){
  for(i=0;1<4;i++){
      Voltage_Real_Array[i]=*ptr;
      ptr++;
  }
  lcd.display();
  lcd.clear();
  lcd.setBacklight(HIGH);
  //Serial.print(Voltage_Real_Array[0]);
  if (LOW_VALUE>Voltage_Real_Array[0]||Voltage_Real_Array[0]>HIGH_VALUE){
    lcd.setCursor (0, 0);
    lcd.print("CELL 1, DANGER");
    lcd.setCursor (0, 1);
    lcd.print("1:");
    lcd.setCursor (2, 1);
    lcd.print(Voltage_Real_Array[0]);
    lcd.setCursor (6, 1);
    lcd.print("v");
  }
    if (LOW_VALUE>Voltage_Real_Array[1]||Voltage_Real_Array[1]>HIGH_VALUE){
    lcd.setCursor (0, 0);
    lcd.print("CELL 2, DANGER");
    lcd.setCursor (0, 1);
    lcd.print("2:");
    lcd.setCursor (2, 1);
    lcd.print(Voltage_Real_Array[1]);
    lcd.setCursor (6, 1);
    lcd.print("v");
  }
    if (LOW_VALUE>Voltage_Real_Array[2]||Voltage_Real_Array[2]>HIGH_VALUE){
    lcd.setCursor (0, 0);
    lcd.print("CELL 3, DANGER");
    lcd.setCursor (0, 1);
    lcd.print("3:");
    lcd.setCursor (2, 1);
    lcd.print(Voltage_Real_Array[2]);
    lcd.setCursor (6, 1);
    lcd.print("v");
  }
    if (LOW_VALUE>Voltage_Real_Array[3]||Voltage_Real_Array[3]>HIGH_VALUE){
    lcd.setCursor (0, 0);
    lcd.print("CELL 4, DANGER");
    lcd.setCursor (0, 1);
    lcd.print("4:");
    lcd.setCursor (2, 1);
    lcd.print(Voltage_Real_Array[3]);
    lcd.setCursor (6, 1);
    lcd.print("v");
  }
  digitalWrite(BUZZERPIN,HIGH);
  
  
}
