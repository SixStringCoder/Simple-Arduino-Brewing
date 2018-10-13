//by Eric Stricklin
//This is a super simple BIAB (brew in a bag) Arduino sketch.  
//It's called the "Bobcat" version because of my friend, Bob Barnes and his Bobcat themed brewery,
//he uses BIAB to make awesome beer and his electric system inspired me to build this simpler system than what I originally dreamed up.
//The serial monitor is used to send commands to the Arduino, when the Arduino receives certain commands, it calls the corresponding function.

//This sketch is in the public domain

/*------------------ WARNING-------------------------------------------- */
//Do not use pins 0 and 1!  They are needed for serial com

//Libraries
//This one is for the one wire bus system that is used with the temp sensors
#include <OneWire.h>
//Dallas temp sensor library
#include <DallasTemperature.h>
//PID library, the standard one
#include <PID_v1.h>

//Using pin 7 on the one wire bus, in other words the temp sensor data wire is connected to pin 7
//There is a 4.7k pullup resistor on the 5v supply from the Arduino, the data and V++ wires on connected together at the sensor
//The only other wire that needs connected on the sensor is ground
#define ONE_WIRE_BUS 7

//Calling a new instance of the one wire bus
OneWire oneWire(ONE_WIRE_BUS);

//Pass the one wire reference to the Dallas Library so that the one wire bus can "see" the sensors
DallasTemperature sensors(&oneWire);

//This next bit of code will be setting up the PID
//Here are the variables the PID will need
//Setpoint will be changed later it is the desired temperature
double Setpoint;
//Input is simply the current temperature, this will be read later and the variable will be given a value at that time.
double Input;
//Output is the PWM value between 0-255, this pin will be connected to the SSR.  
double Output;
//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,2,5,1, DIRECT);

//This variable sets the number of loops the mash function's main while loop will cycle through before updating the serial monitor with new data
//Setting it faster just speeds up how fast the info is displayed but doesn't have any effect on how the controller is actually working
int const mash_loop_speed = 10000;
//same thing as above but for the boil
int const boil_loop_speed = 10000;
//same for mashout
int const mashout_loop_speed = 10000;
//same for temp only
int const temp_only_loop_speed = 10000;

//mashout temp is a constant value
const int mashout_temp = 170;

//for displaying temp
double temp_display;

void setup() {
  //Start serial coms
   Serial.begin(9600); 

   //Start the sensors 
    sensors.begin(); 

  //Call the menu function that will give the user options to start
  menu();

}//end setup

void loop() {
  //No code needed here!

}//end loop

void menu () {
  //variable for the while loop, when it becomes true the user's option has been read 
  bool serial_bool = true;

  //variable to store the user's option that they typed
   char user_option;
   
  //Gives the user a list of options
  Serial.print("\n");
  Serial.print("Bobcat V1.0 BIAB Simple Brewing Software");
  Serial.println("\n By Eric Stricklin \nwww.homebrewtalk.com user ID -- estricklin");
  Serial.println("This software is freeware and in the public domain.");
  Serial.println("\nPlease enter one of the following options: \n");
  Serial.println("Type \"M\" to heat strike water for mash");
  Serial.println(" \n Type \"B\" to boil or for manual control");
  Serial.println("\n Type \"O\" for mashout");
  Serial.println("\n Type \"T\" for temp readings only");

  //Read the serial port and determine which function to call
  //This will loop until the user types an option, there is a delay in there for stability, I'm not sure if it's needed or not.
  while(serial_bool) {
    if(Serial.available() > 0) {
      user_option = Serial.read();
      serial_bool = false;
      delay(100);
    }
  }//end while
  Serial.println(user_option);
  switch (user_option) {
    case 'M':
    mash();
    break;
    case 'm':
    mash();
    break;
    case 'B':
    boil();
    break;
    case 'b':
    boil();
    break;
    case 'O':
    mashout();
    break;
    case 'o':
    mashout();
    break;
    case '0':
    mashout();
    break;
    case 'T':
    temp_only();
    break;
    case 't':
    temp_only();
    break;
    default:
    temp_only();
  
  }

}//end menu

void mash () {
  //variable for the while loops
bool user_bool = true;
bool mash_bool = true;

// Set Setpoint to 0 to start with
Setpoint = 0;

//to display temp to user
float display_pid;
int display_pid_converted;

//Mash splash screen
  Serial.println("Mash/Strike Selected");
  Serial.println("To exit mash control and return to the main menu, at any time press \"0\"");
  Serial.println("\n Please enter your desired temperature in F");

    while(user_bool) {
      if(Serial.available() > 0){
        Setpoint = Serial.parseInt();
        if(Setpoint == 0){
          user_bool = false;
          Serial.print("\nExiting Mash..........");
          delay(2000);
          menu();
        }
          else if(Setpoint > 0 && Setpoint < 213){
            user_bool = false;
          }
      
    }
  }//end while
  //start the PID
  myPID.SetMode(AUTOMATIC);
  Serial.print("\nTarget Strike Temp Set to: ");
  Serial.print(Setpoint);
  Serial.print("\nInitializing PID.........................");
  
    while(mash_bool) {
      sensors.requestTemperatures();
      delay(10);
      Input = sensors.getTempFByIndex(0);
      delay(10);
      // taking this out for now Input = sensors.getTempFByIndex(0);
      myPID.Compute();
      analogWrite(3,Output);
      delay(mash_loop_speed);
      Serial.print("\n\n\nTarget Temp: ");
      Serial.print(Setpoint);
      Serial.print("\nKettle Temp: ");
      Serial.print(Input);
      Serial.print("\nPID: ");
       display_pid = Output/255;
      display_pid = display_pid * 100;
      display_pid_converted = display_pid;
      Serial.print(display_pid_converted);
      Serial.print("%");
          if(Serial.available() > 0) {
            if(Serial.parseInt() == 0){
              mash_bool = false;
            }
          }
    }//end while
    Serial.print("\nExiting Mash..........");
    delay(2000);
    Serial.print("\n\n\n");
    heat_off();
    menu();
}

void boil() {
  //variables for loops
  bool menu_loop = true;
  bool menu_loop2 = true;

  //variable for user choice
  int pid_level = 0;
  int pid_level_check = 0;
  int menu_key;
  
  //variable for displaying the PID level user friendly
  int pid_display = 0;
  int update_display = 0;
  
  Serial.println("\nBoil Selected");
  Serial.print("\nPress \"0\" at any time to exit to the main menu");
  Serial.print("\nPress \"1\" to start, then type a % value between 0-100 for the PID power level at any time.");
    while(menu_loop) {
      if(Serial.available() > 0){
        menu_key = Serial.parseInt();
          if(menu_key == 0){
            menu_loop = false;
            heat_off();
            Serial.print("\nExiting Boil......");
            delay(2000);
            menu();
          }
            else if(menu_key == 1){
              menu_loop = false;
            }
      }
    }
    while(menu_loop2) {
      if(Serial.available() > 0) {
        pid_level_check = Serial.parseInt();
          if(pid_level_check > 0 && pid_level_check < 101) {
            pid_level = pid_level_check;
            pid_level = pid_level * 2.55;
            pid_display = pid_level_check;
            }
              else if(pid_level_check == 0){
                    menu_loop2 = false;
                    heat_off();
                    Serial.print("\nExiting Boil......");
                    delay(3000);
                    menu();
              }
          }
          
     //only display every few seconds
      delay(boil_loop_speed);
      sensors.requestTemperatures();
      delay(30);
      sensors.getTempFByIndex(0);
      delay(30);
      temp_display = sensors.getTempFByIndex(0);
      analogWrite(3, pid_level);
      Serial.print("\nBoil Mode");
      Serial.print("\nPID: ");
      Serial.print(pid_display);
      Serial.print("%");
      Serial.print("\nKettle Temp -- ");
      Serial.print(temp_display);
      Serial.print("F\n");
    }
  
}
    

void mashout() {
  //assign PID Setpoint to mashout_temp const variable
  Setpoint = mashout_temp;
  //variable for menu selection
  int menu_choice;

//variable for mashout main loop
bool mashout_loop = true;
bool mashout_loop2 = true;

  Serial.print("Mashout Selected");
  Serial.print("\nPress \"0\" at any time to exit to main menu");
  Serial.print("\nPress \"1\" to begin mashout.");
  Serial.print("\nTarget temp will be set to: ");
  Serial.print(mashout_temp);
  Serial.print("F");
    while(mashout_loop){
      if(Serial.available() > 0){
        menu_choice = Serial.parseInt();
          if(menu_choice == 0){
            mashout_loop = false;
            Serial.print("\nExiting Mashout.......");
            delay(2000);
            heat_off();
            menu();
          }
          else if(menu_choice == 1){
            mashout_loop = false;
            //start PID
            myPID.SetMode(AUTOMATIC);
          }
      }
      
    }//end mashout while loop
    while(mashout_loop2){
       sensors.requestTemperatures();
       delay(10);
       sensors.getTempFByIndex(0);
       delay(10);
       Input = sensors.getTempFByIndex(0);
       myPID.Compute();
       analogWrite(3,Output);
       temp_display = sensors.getTempFByIndex(0);
       Serial.print("\nMashout Mode");
       Serial.print("\nMash Temp: ");
       Serial.print(Input);
       Serial.print("\nPID: ");
       Serial.print(Output/255*100);
       Serial.print("\nTarget Temp: ");
       Serial.print(mashout_temp);
       Serial.print("F\n\n");
       delay(mashout_loop_speed);
        if(Serial.available() > 0){
          menu_choice = Serial.parseInt();
            if(menu_choice == 0){
              mashout_loop2 = false;
              Serial.print("\nExiting Mashout.......");
              delay(2000);
              heat_off();
              menu();
            }
        }
    }
}

void temp_only() {
//variable for loop
bool temp_only_loop = true;

//variable for menu selection
int menu_choice;

  while(temp_only_loop){
    Serial.print("\nTemp Only Selected");
    sensors.requestTemperatures();
    delay(10);
    sensors.getTempFByIndex(0);
    delay(10);
    temp_display = sensors.getTempFByIndex(0);
    Serial.print("\nTemp: ");
    Serial.print(temp_display);
    Serial.print("F\n\n");
    delay(temp_only_loop_speed);
      if(Serial.available() > 0){
        menu_choice = Serial.parseInt();
          if(menu_choice == 0){
            temp_only_loop = false;
            //just for safety even though it wasn't on
            heat_off();
            Serial.print("\n\nExiting Temp Only.....");
            delay(200);
            menu();
            
          }
      }
  }
  
}

//Turn the element off when this function is called
void heat_off(){
   analogWrite(3,0);
}

