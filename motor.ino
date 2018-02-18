#include <PS2X_lib.h>  //for v1.6
#include <AFMotor.h>

#define MAX_SPEED_1 255/2
#define MAX_SPEED_2 255
#define MIN_SPEED_OFFSET 0

AF_DCMotor waterMotor(1,MOTOR12_1KHZ);
AF_DCMotor sweepMotor(2,MOTOR12_1KHZ);

AF_DCMotor rearLeftWheel(3,MOTOR12_1KHZ);
AF_DCMotor rearRightWheel(4,MOTOR12_1KHZ);

int currentMaxSpeed = MAX_SPEED_2;

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you conect the controller, 
//or call config_gamepad(pins) again after connecting the controller.
int error = 0; 
byte type = 0;
byte vibrate = 0;

boolean sweeperRunning = false;
boolean sweeperClockWiseDirection = false;

void triggerSweeper() {
  if ( sweeperRunning ) {
    sweepMotor.setSpeed(255);
    sweepMotor.run(sweeperClockWiseDirection?FORWARD:BACKWARD);
  } else {
    sweepMotor.run(RELEASE);
  }
}

boolean waterReleaseFlag = false;
void releaseWater(boolean flag) {
  if (flag) {
    waterReleaseFlag = true;
    waterMotor.setSpeed(255);
    waterMotor.run(FORWARD);
    delay(250);
/*    delay(50);
    waterMotor.run(BACKWARD);
    delay(1000);   */
    waterReleaseFlag = false;
    waterMotor.run(RELEASE);
  } else {
    if ( waterReleaseFlag ) {
      waterReleaseFlag = false;
      waterMotor.run(RELEASE);
      delay(50);
      waterMotor.run(BACKWARD);
      delay(2000);   
    }
    waterMotor.run(RELEASE);
  }
}

void setSpeed(int newSpeed) {
  Serial.println("New Speed "+ newSpeed); //prints the hex value a a button press

  currentMaxSpeed = newSpeed;
}

void updateMotor(AF_DCMotor motor,int speed,boolean direction) {
    /*Serial.print(" ");
    Serial.print(speed,DEC);
    Serial.println(" ");*/
  if ( speed > 255 ) {
    speed = 255;
    /*Serial.print(" ");
    Serial.print(speed,DEC);*/
  } else if (speed <= 0 ) {
    motor.run(RELEASE);
/*    Serial.print(" ");
    Serial.print(0,DEC);
    Serial.print(" STOP");
    Serial.println(" ");*/
    return;
  }

  motor.setSpeed(255);

  if ( direction ) {
    Serial.print(" FORWARD");
    motor.run(FORWARD);
  } else {
    Serial.print(" BACKWARD");
    motor.run(BACKWARD);
  }
  Serial.println(" ");

}

void driveVehicle(int speed,int direction) {
  // speed ranges from 255 to 0. 255 is backward, 127 is no movement, 0 is forward
  // direction ranges from 0 to 255. 0 is left, 127 is no direction, 255 is right

  int forwardOffset = 127 - speed;
  int directionOffset = 128 - direction;

  int forwardSpeedOffset = 255/(127 - MIN_SPEED_OFFSET);
  int directionSpeedOffset = 255/(128 - MIN_SPEED_OFFSET);

/*
  Serial.print("Values : ");
  Serial.print(forwardOffset * forwardSpeedOffset,DEC);
  Serial.print(" , ");
  Serial.print(directionOffset * directionSpeedOffset,DEC);
  Serial.println(" ");
*/
  
  if ( forwardOffset <= MIN_SPEED_OFFSET && forwardOffset >= (-1 * MIN_SPEED_OFFSET) ) {  // no movement
    Serial.println("No Movement");
    updateMotor(rearLeftWheel,0,true);
    updateMotor(rearRightWheel,0,true);

  } else if ( forwardOffset > MIN_SPEED_OFFSET ) { // forward direction
    if ( directionOffset > MIN_SPEED_OFFSET ) {  // left direction
      Serial.println("Forward Left Direction");
      updateMotor(rearLeftWheel,directionSpeedOffset * directionOffset,true);
      updateMotor(rearRightWheel,forwardSpeedOffset * forwardOffset,false);

    } else if ( directionOffset < (-1 * MIN_SPEED_OFFSET) ) { // right direction
      Serial.println("Forward Right Direction");
      updateMotor(rearLeftWheel,forwardSpeedOffset * forwardOffset,false);
      updateMotor(rearRightWheel,directionSpeedOffset * directionOffset * -1,true);

    } else {
      Serial.println("Forward Direction");
      updateMotor(rearLeftWheel,forwardSpeedOffset * forwardOffset,true);
      updateMotor(rearRightWheel,forwardSpeedOffset * forwardOffset,true);

    }
  } else if ( forwardOffset < ( -1 * MIN_SPEED_OFFSET ) ) {  // backward direction
    if ( directionOffset > MIN_SPEED_OFFSET ) {  // left direction
      Serial.println("Backward Left Direction");
      updateMotor(rearLeftWheel,directionSpeedOffset * directionOffset,false);
      updateMotor(rearRightWheel,forwardSpeedOffset * forwardOffset * -1,true);

    } else if ( directionOffset < ( -1 * MIN_SPEED_OFFSET) ) { // right direction
      Serial.println("Backward Right Direction");
      updateMotor(rearLeftWheel,forwardSpeedOffset * forwardOffset * -1,true);
      updateMotor(rearRightWheel,directionSpeedOffset * directionOffset * -1,false);

    } else {
      Serial.println("Backward Direction");
      updateMotor(rearLeftWheel,forwardSpeedOffset * forwardOffset * -1,false);
      updateMotor(rearRightWheel,forwardSpeedOffset * forwardOffset * -1,false);

    }
  } else {
    Serial.println("No Movement");
    updateMotor(rearLeftWheel,0,true);
    updateMotor(rearRightWheel,0,true);
  }
}

void setup(){
 Serial.begin(57600);

 //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  
 error = ps2x.config_gamepad(13,9,2,10, true, true);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
// error = ps2x.config_gamepad(16,14,17,15, true, true);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
 
 if(error == 0){
   Serial.println("Found Controller, configured successful");
   Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
  Serial.println("holding L1 or R1 will print out the analog stick values.");
  Serial.println("Go to www.billporter.info for updates and to report bugs.");
 }
   
  else if(error == 1)
   Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
   Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
   
  else if(error == 3)
   Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
   
   //Serial.print(ps2x.Analog(1), HEX);
   
   type = ps2x.readType(); 
     switch(type) {
       case 0:
        Serial.println("Unknown Controller type");
       break;
       case 1:
        Serial.println("DualShock Controller Found");
       break;
       case 2:
         Serial.println("GuitarHero Controller Found");
       break;
     }
  
}

void loop(){
   /* You must Read Gamepad to get new values
   Read GamePad and set vibration values
   ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
   if you don't enable the rumble, use ps2x.read_gamepad(); with no values
   
   you should call this at least once a second
   */
   
   
   
 if(error == 1) //skip loop if no controller found
  return; 
  
 if(type == 1) { //DualShock Controller
  
    ps2x.read_gamepad(false, vibrate);          //read controller and set large motor to spin at 'vibrate' speed
    
    if(ps2x.Button(PSB_START)) {                  //will be TRUE as long as button is pressed
         Serial.println("Start is being held");
         sweeperRunning = !sweeperRunning;
    }
    if(ps2x.Button(PSB_SELECT)) {
         Serial.println("Select is being held");
         sweeperClockWiseDirection = !sweeperClockWiseDirection;
    }
    triggerSweeper();
    
      vibrate = ps2x.Analog(PSAB_BLUE);        //this will set the large motor vibrate speed based on 
                                              //how hard you press the blue (X) button    
//         releaseWater(vibrate>0);
    if (ps2x.NewButtonState())               //will be TRUE if any button changes state (on to off, or off to on)
    {
     
       
         
        if(ps2x.Button(PSB_L3)) {
         Serial.println("L3 pressed");
        }
        if(ps2x.Button(PSB_R3))
         Serial.println("R3 pressed");
        if(ps2x.Button(PSB_L2)) {
         Serial.println("L2 pressed");
         setSpeed(MAX_SPEED_2);
        }
        if(ps2x.Button(PSB_R2))
         Serial.println("R2 pressed");
        if(ps2x.Button(PSB_GREEN))
         Serial.println("Triangle pressed");
         
    }   
         
    
    if(ps2x.ButtonPressed(PSB_RED))             //will be TRUE if button was JUST pressed
         Serial.println("Circle just pressed");
         
    if(ps2x.ButtonReleased(PSB_PINK))             //will be TRUE if button was JUST released
         Serial.println("Square just released");     
    
    if(ps2x.NewButtonState(PSB_BLUE)) {           //will be TRUE if button was JUST pressed OR released
         Serial.println("X just changed"); 
         releaseWater(true);   
    }

    if(ps2x.Button(PSB_L1)) {
       Serial.println("L1 pressed");
        driveVehicle(ps2x.Analog(PSS_LY),ps2x.Analog(PSS_LX));
    } else {
      updateMotor(rearLeftWheel,0,true);
      updateMotor(rearRightWheel,0,true);

    }
    
    if(ps2x.Button(PSB_R1)) {
       Serial.println("R1 pressed");
    } 

    /*
    Serial.print("Stick Values:");
    Serial.print(ps2x.Analog(PSS_LY), DEC); //Left stick, Y axis. Other options: LX, RY, RX  
    Serial.print(",");
    Serial.print(ps2x.Analog(PSS_LX), DEC); 
    Serial.print(",");
    Serial.print(ps2x.Analog(PSS_RY), DEC); 
    Serial.print(",");
    Serial.println(ps2x.Analog(PSS_RX), DEC);

    */
    

 }
 
 
 delay(50);
     
}

