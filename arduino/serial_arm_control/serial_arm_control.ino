// VarSpeedServo - Version: Latest

/*************************************************************

 *************************************************************

/* Comment this out to disable prints and save space */
//#define BLYNK_PRINT DebugSerial
// You could use a spare Hardware Serial on boards that have it (like Mega)
#include <SoftwareSerial.h>
//SoftwareSerial DebugSerial(2, 3); // RX, TX

#include <VarSpeedServo.h>
// Do not use Servo.h and VarSpeedServo.h at the same time, causes conflicts.

VarSpeedServo servoBase;
VarSpeedServo servoElbow;
VarSpeedServo servoGrip;

int angleBase = 15; // 100 and 40 initialize the arm straight down position
int angleElbow = 30; // 180 and 180 put it in "stow" position
int angleGrip = 115; //initialize gripper open (~20 is closed)

byte servoBasePin = 9;
byte servoElbowPin = 8;
byte servoGripPin = 7;

byte servoMin = 10;
byte servoMax = 170;
byte servoPosBase = 150;
byte servoPosElbow = 100;
byte servoPosGrip = 100;
byte newServoPos = servoMin;

const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;

char messageFromPC[buffSize] = {0};
int servoCommand = 0; // goal position of servo to move to

byte queue[3] = {-1,-1,-1};

unsigned long curMillis;
unsigned long replyToPCinterval = 1000;

void setup()
{ // Debug console
  //DebugSerial.begin(9600);
  Serial.begin(9600);

  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(10, OUTPUT);
  
  servoBase.attach(servoBasePin);// attach the signal pin of servo to arduino
  servoBase.write(angleBase);
  servoElbow.attach(servoElbowPin);// attach the signal pin of servo to arduino
  servoElbow.write(angleElbow);
  servoGrip.attach(servoGripPin);// attach the signal pin of servo to arduino
  servoGrip.write(angleGrip);

  // tell the PC we are ready
  //Serial.println("<Arduino is ready>");

}

//=============

void loop() {

    curMillis = millis(); // current time in milliseconds since beginning of sketch
    getDataFromPC();
    updateServoPos();
    //replyToPC();
}

//=============

void serialFlush(){
  while(Serial.available() == SERIAL_RX_BUFFER_SIZE){
    char t = Serial.read();  
  }    
}

void getDataFromPC() {

    // receive data from PC and save it into inputBuffer
    // Expects '<' and '>' as start and end markers, respectively.
  if(Serial.available() > 0) {

    char x = Serial.read();

      // the order of these IF clauses is significant

    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      parseData();
    }

    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) {
      bytesRecvd = 0;
      readInProgress = true;
    }
  }
}


//=============

void parseData() {

    // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(inputBuffer,","); // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC

  strtokIndx = strtok(NULL, ",");
  servoCommand = atoi(strtokIndx);     // convert this part to an integer

}

//=============


void replyToPC() {
// creates a message & shows if data is parsed correctly
  if (newDataFromPC) {
    newDataFromPC = false;
    Serial.print("<Msg ");
    Serial.print(messageFromPC);
    Serial.print(" SrvCmd ");
    Serial.print(servoCommand);
    Serial.print(" SrvPos ");
    Serial.print(newServoPos);
    Serial.print(" Time ");
    Serial.print(curMillis >> 9); // divide by 512 is approx = half-seconds
    Serial.println(">");

    if(strcmp(messageFromPC, "Base") == 0){
      queue[0] = newServoPos;  
    }
    if(strcmp(messageFromPC, "Elbow") == 0){
      queue[1] = newServoPos;  
    }
    if(strcmp(messageFromPC, "Grip") == 0){
      queue[2] = newServoPos;  
    }
  }
}

//============

void updateServoPos() {

  byte servoRange = servoMax - servoMin;
  //if (servoFraction >= 0 && servoFraction <= 1) {
   // newServoPos = servoMin + ((float) servoRange * servoFraction);
 // }
  newServoPos = servoCommand;
  // this illustrates using different inputs to call different functions
  if (strcmp(messageFromPC, "Base") == 0) {
    moveBase();
  }
  if (strcmp(messageFromPC, "Elbow") == 0) {
    moveElbow();
  }
  if (strcmp(messageFromPC, "Grip") == 0) {
    moveGrip();
  }

}

//=============

void moveBase() {
  while(servoPosBase != newServoPos && queue[0]) {
    servoPosBase = newServoPos;
    digitalWrite(12, HIGH);
    servoBase.write(servoPosBase, 20);
    digitalWrite(12, LOW);
    queue[0] = -1;
  }
}

void moveElbow() {
  while(servoPosElbow != newServoPos) {
    servoPosElbow = newServoPos;
    digitalWrite(11, HIGH);
    servoElbow.write(servoPosElbow, 20);
    digitalWrite(11, LOW);
    queue[1] = -1;
  }
}

void moveGrip() {
  while(servoPosGrip != newServoPos) {
    servoPosGrip = newServoPos;
    digitalWrite(10, HIGH);
    servoGrip.write(servoPosGrip);
    digitalWrite(10, LOW);
    queue[2] = -1;
  }
}












