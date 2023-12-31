/**********************************************************************
Thermistor Code by Roman Sequeira Updated: Oct 11th 2023
code designed to read the resistance of a thermistor and calculate the temperautre for Sensing the Sea
Some content is from Arduino or Adafruit examples
Some content is from Dr. Gregory Gerbi's TloggerNoT.ino






***********************************************************************/
// included libraries
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

//SD card/clock/file set up
  File myFile;
  RTC_DS3231 rtc;
  char fname[] = "mmddhhnn.csv";

  const int chipSelect = 10; //change depending on sd reader
//variable floats
  float Vin=0; //Initial Voltage
  float V2=0; //Midpoint Voltage
  float VG=0; //End (ground) Voltage
  float RSTR=0; //Resistance of the thermistor
  float V2_minus_VG=0; //variable to store value of Midpoint Voltage minus End Voltage
  float R1=10000; //Known value of the second resistor
  float Vin_minus_V2=0; //variable to store value of Initial Voltage minus Midpoint Voltage
  float z=0; //variable to store Midpoint Voltage minus End Voltage divided by Initial Voltage minus Midpoint Voltage
  float OneOverTemp=0; //variable to store 1/T value from Steinhart-Hart equation
  float TEMP=0; //Temperature in Celcius measured from thermistor
  float A=0.009272495; //A coefficient
  float B=-0.001105718; //B coefficient
  float C=0.0000057243; //C coefficient
  float lnR=0; //natural log of Resistance
  float BlnR=0;
  float ClnR3rd=0;
// variable ints
  int inPin=A0;
  int Pin2=A1;
  int GPin=A2;

void setup() {
  // baud rate for communication with Arduino via Serial Monitor
  Serial.begin(9600);
  // activate the clock and set the time to the same time when the programw was compiled
  rtc.begin();
  // rtc.adjust(DateTime(2023,10,25,11,15,35));
  //if you press the power button,clock will reset back to that time
  // Check to see if SD card has connected
   Serial.println("Simple SD Card Demo");

   if (SD.begin(chipSelect)) //if card is present
    {
    Serial.println("SD card is present & ready");
    // make file with date
      makefname();
    //print the file header to serial
      Serial.println("year,month,day,hour,minute,second,clocktemperature,thermistorresistance,thermistortemperature");
    //print the file header to the file
      myFile = SD.open(fname, FILE_WRITE);
      myFile.println("Vin,V2,VG,thermistorresistance,thermistortemp,year,month,day,hour,minute,second");
      myFile.close();
      // serial print fname
        Serial.print("fname: ");
        Serial.println(fname);
    } 
    else
    {
    Serial.println("SD card missing or failure"); //if the card isn't read
    while(1);  //wait here forever
}

}
void loop() {
  // update the time every loop
  DateTime now = rtc.now();
  // compute the voltage at 3 points within the circuit and the resistance of the thermister
  computeVLTGE_RSTR(inPin, Pin2, GPin);
  //compute the temperature from resistance
  converttemp(RSTR);
  //print the data from the previous function to the serial monitor
  print2serial();
  //print the time stamp
  clock(now);
  //write both the time stamp and the recorded data to the SD card
  SDwrite(now);
  //delay for 1 second
  delay(1000);
}




/************
************/
// read the resistance from the thermistor and voltage from 3 points within the circuit
float computeVLTGE_RSTR (int inPin, int Pin2, int GPin){
  // converting analog number read into voltage
  Vin=0.00488*analogRead(inPin); 
  V2=0.00488*analogRead(Pin2);
  VG=0.00488*analogRead(GPin);
  // calculation to determine resistance
    V2_minus_VG=V2-VG;
    Vin_minus_V2=Vin-V2;
    z=V2_minus_VG/Vin_minus_V2;
    RSTR=R1*z;
  return RSTR;
}
/************
************/




// calculation to convert resistance into temperature using the Steinhart-Hart equation
float converttemp(int RSTR){ //input resistance
lnR=log(RSTR); // natural log of the resistance
BlnR=B*lnR; // B coefficient times natural log of the resistance
ClnR3rd=C*pow(lnR,3); // C coefficient times natural log of the resistance cubed
OneOverTemp=A+BlnR+ClnR3rd; //A coefficient plus the previous two values
TEMP=1/OneOverTemp; //convert previous value into temperature (Kelvin)
TEMP=TEMP-273.15; // conversion into celcius
return TEMP; //output temperature
}




/************
************/
void clock(DateTime now) {
  //Get time
  float clocktemperature = rtc.getTemperature();   //If using DS3231
  // DateTime now = rtc.now(); // THE CODE DOESN'T WORK without this
    Serial.print(now.year(), DEC);
    Serial.print(',');
    Serial.print(now.month(), DEC);
    Serial.print(',');
    Serial.print(now.day(), DEC);
    Serial.print(',');
    Serial.print(now.hour(), DEC);
    Serial.print(',');
    Serial.print(now.minute(), DEC);
    Serial.print(',');
    Serial.println(now.second(), DEC);
}




/************
************/
void SDwrite(DateTime now)
{ 
  myFile = SD.open(fname, FILE_WRITE);
      if (myFile) // it opened OK
      {
      Serial.println("Writing to file");
        myFile.print(Vin);
        myFile.print(',');
        myFile.print(V2);
        myFile.print(',');
        myFile.print(VG);
        myFile.print(',');
        myFile.print(RSTR);
        myFile.print(',');
        myFile.print(TEMP);
        myFile.print(',');  
        myFile.print(now.year(), DEC);
        myFile.print(',');
        myFile.print(now.month(), DEC);
        myFile.print(',');
        myFile.print(now.day(), DEC);
        myFile.print(',');
        myFile.print(now.hour(), DEC);
        myFile.print(',');
        myFile.print(now.minute(), DEC);
        myFile.print(',');
        myFile.print(now.second(), DEC);
        myFile.println(',');

      myFile.close(); 
      Serial.println("Done");
    }
  else 
    Serial.println("Error opening data.txt");
}




/************
************/
// make a filename using time stamp (limited to 8 characters plus extension)
//name format is MMDDHHNN.CSV (no year or seconds)
//assumes fname is a global variable and rtc is globally defined
void makefname()
{
  DateTime now = rtc.now();  //Get the time
  // uint16_t year = now.year();
  uint8_t month = now.month();
  uint8_t day = now.day();
  uint8_t hour = now.hour();
  uint8_t minute = now.minute();
  // uint8_t second = now.second();

  // char YYYY[5];
  char MM[3];
  char DD[3];
  char HH[3];
  char NN[3];
  // char SS[3];

  // sprintf(YYYY,"%04d",year);
  sprintf(MM,"%02d",month);
  sprintf(DD,"%02d",day);
  sprintf(HH,"%02d",hour);
  sprintf(NN,"%02d",minute);
  // sprintf(SS,"%02d",second);

  // strcpy(fname,"data");
  // strcat(fname,YYYY);
  strcpy(fname,MM);   //strcpy for first entry
  strcat(fname,DD);
  strcat(fname,HH);
  strcat(fname,NN);
  // strcat(fname,SS);
  strcat(fname,".csv");

  return fname;
}




/************
************/
// print recorded data to the serial monitor
void print2serial(){
  Serial.print(RSTR);
  Serial.print(",");
  Serial.print(Vin);
  Serial.print(",");
  Serial.print(V2);
  Serial.print(",");
  Serial.print(VG);
  Serial.print(",");
  Serial.print(TEMP);
  Serial.print(",");
}