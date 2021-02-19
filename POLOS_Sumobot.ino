// SOME BACKGROUND about U-sonic sensors
//There are echo sensors that send a signal with a trigger (all the variables w a T) and receive the reflected signal (all the E's)
//There are infrared sensors which only have an input, and it is denoted w IR
int E_FRONT_LEFT = 9; 
int T_FRONT_LEFT = 8;

int E_FRONT_RIGHT= 12;
int T_FRONT_RIGHT = 13;

int E_LEFT = 3;
int T_LEFT = 2;

int E_RIGHT = 4;
int T_RIGHT = 7;


int IR_FRONT_LEFT = A0;
int IR_FRONT_RIGHT = A1;
int IR_MID_LEFT = A2;
int IR_MID_RIGHT = A3;
int IR_BACK = A4;

//So above is me declaring all of the pins for the ultrasonic sensor (trigger and echo), and the infra red sensor. The variable names denote the position of the sensor relative to bot
//Below is the same thing for the two motors, except the motors have two pins each, one to reverse and one to go forward
//another important fact is that the pins the motors are connected to allow us to give a range of voltages as opposed to on or off (PWM pins) 

int motor_LEFT_FORWARD = 0; 
int motor_LEFT_REVERSE = 1;
int motor_RIGHT_FORWARD = 10;
int motor_RIGHT_REVERSE = 11;
int motor_LEFT_SPEED = 5;
int motor_RIGHT_SPEED = 6;


//These velocity variables simply tell each motor how fast to go (negative values imply going in reverse), and it goes from -1 to 1
// these are just some constants that I used a lot, and the duration variable is used to decipher the sensor values
int  max_distance = 75, lock_on_distance = 20, attack_distance = 10, duration; 

bool IR_DEFENSE_OFFENSE = false;
//So there are two IR sensors in the front of our robot, 2 in the middle and one in the back, we do this in case our robot is pushing the opponent off stage, 
//we dont want our robot to reverse if it sees a line
//So in essence, when the robot is attacking we ignore the front two sensors and use the middle two, otherwise we do the opposite (the back sensor is always used though)
//This boolean simply tells us which pair on sesnors we are using
//false IMPLIES WE WILL USE THE FRONT SENSORS, true MEANS MIDDLE SENSORS

//so this pretty much takes all of the ultrasonic sensors and deciphers them, and puts all the distances in an array

int ECHO[4]={E_FRONT_LEFT, E_FRONT_RIGHT, E_RIGHT, E_LEFT};
int TRIG[4]={T_FRONT_LEFT, T_FRONT_RIGHT, T_RIGHT, T_LEFT};
int IR[5] = {IR_FRONT_LEFT, IR_FRONT_RIGHT, IR_MID_LEFT, IR_MID_RIGHT, IR_BACK};
int MOTOR[6] = {motor_LEFT_FORWARD, motor_LEFT_REVERSE, motor_RIGHT_FORWARD, motor_RIGHT_REVERSE, motor_LEFT_SPEED, motor_RIGHT_SPEED};
float VELOCITY[2] = {0,0};

int* ECHO_DECIPHER(int*,int*,int*);
bool* INFR_DECIPHER(int*, bool*);

int ATTACK(float DISTANCE_ARRAY[4]);
int LOCK_ON(float DISTANCE_ARRAY[4]);
int PATROL(float DISTANCE_ARRAY[4]);
int DEFEND(bool DETECTION_ARRAY[5]);
void SET_VELOCITY();

float DISTANCE[4] = {0,0, 0, 0};
bool DETECTION[5] ={LOW,LOW,LOW,LOW,LOW};
 //this does something similar but all the values are boolean (sees a white line or not)

//Now to be able to manipulate and access all of these variables more effeciently (and elegantly)


void setup() {
//simply setting all of our outputs and inputs to their respective variables
    pinMode(E_FRONT_LEFT, INPUT);
    pinMode(E_FRONT_RIGHT, INPUT);
    pinMode(E_LEFT, INPUT);
    pinMode(E_RIGHT, INPUT);

    pinMode(IR_FRONT_LEFT, INPUT);
    pinMode(IR_FRONT_RIGHT, INPUT);
    pinMode(IR_BACK, INPUT);
    
    pinMode(T_FRONT_LEFT, OUTPUT);
    pinMode(T_FRONT_RIGHT, OUTPUT);
    pinMode(T_LEFT, OUTPUT);
    pinMode(T_RIGHT, OUTPUT);
    
    pinMode(motor_LEFT_FORWARD, OUTPUT);
    pinMode(motor_LEFT_REVERSE, OUTPUT);
    pinMode(motor_RIGHT_FORWARD, OUTPUT);
    pinMode(motor_RIGHT_REVERSE, OUTPUT);
    pinMode(motor_LEFT_SPEED, OUTPUT);
    pinMode(motor_RIGHT_SPEED, OUTPUT);
    Serial.begin(9600);
    ECHO_DECIPHER(ECHO, TRIG, DISTANCE);
    INFR_DECIPHER(IR, DETECTION);

    delay(5000); //so now we have all the initial values set and we have stored the positioning of the other robot, this all happens as soon as the robot turns on, afterwards the rules say we must wait 5 seconds before starting
}

void loop() {

 MODE_SELECT(DETECTION, DISTANCE);//this function is the brains of the whole operation, and it decides when to attack, defend etc
 ECHO_DECIPHER(ECHO, TRIG, DISTANCE);//after it decides what to do, it will take new data with the same functions we used before and changes them
 INFR_DECIPHER(IR, DETECTION);
}
int* ECHO_DECIPHER(int ECHO_ARRAY[4], int TRIG_ARRAY[4], float DISTANCE_ARRAY[4]){//So this function is pretty much just taking readings and storing them in an array

  for(int i =0; i<4; i++){//using loop to access all array
    digitalWrite(TRIG_ARRAY[i], LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_ARRAY[i], HIGH);//sends a signal
    delayMicroseconds(10);//waits
    digitalWrite(TRIG_ARRAY[i], LOW);//turns off signal
    duration = pulseIn(ECHO_ARRAY[i], HIGH);//reads signal
    DISTANCE_ARRAY[i]= duration*0.017;//converts signal to distance
  }
} 
bool* INFR_DECIPHER(int IR_ARRAY[5], bool DETECTION_ARRAY[5]){
  //true IMPLIES WHITE LINE DETECTED
  for(int i = 0; i<5; i++){//similar to above func but has 5 values and they are boolean
  
    DETECTION_ARRAY[i] = digitalRead(IR_ARRAY[i]) == LOW;
  }
}

int MODE_SELECT(bool DETECTION_ARRAY[5], float DISTANCE_ARRAY[4]){
    int minimum = 1000;
    for(int i = 0; i<4; i++){
      if(DISTANCE_ARRAY[i]<minimum){
        minimum=DISTANCE_ARRAY[i];//If we dont detect anything near we go into patrol mode
      }
    }
     
      if(minimum>lock_on_distance){
        PATROL(DISTANCE_ARRAY);
        
  
      }
      else if(minimum<=lock_on_distance && minimum >= attack_distance){//If we see a robot in range, we will try to lock on using the lock on function
        LOCK_ON(DISTANCE_ARRAY);
      }
      else if(minimum<attack_distance){
        ATTACK(DISTANCE_ARRAY);//if the robot is really close we go for the attack function
      }
      for(int i = 0; i<5; i++){
        if(DETECTION_ARRAY[i]==true){//after we check the offensive options we will check of we are near the edge of the ring
           DEFEND(DETECTION_ARRAY);//if we are we have to invoke the defense function
        }
     }
     
 
     return 0;
  }


int ATTACK(float DISTANCE_ARRAY[4]){
  int target, minimum = 1000;
  float target_ARRAY[4];
  for(int i = 0; i<4; i++){
    if(DISTANCE_ARRAY[i]<attack_distance){ //Checks which side the opponent is withing attack distance of, and stores those values in the target array
      target_ARRAY[i]= DISTANCE_ARRAY[i]; //if the side is within attack range, we store this distance in the target_array
    }
    else{
      target_ARRAY[i] = 0;//if a side is not within range its target values is zero
    }
  }
  for(int i = 0; i<4;i++){
    if(target_ARRAY[i]<minimum && target_ARRAY[i]!=0){
      minimum = target_ARRAY[i];
      target = i; //finding which sensor is closest to the opponent
      
    }
  }

  switch(target){//we will move depending on which sensor is closest to the robot
    case 0://FRONT LEFT
    if(target_ARRAY[1]==0){//if only the left front sensor sees something then will move forward and to the left
      VELOCITY[0] = 0.75;
      VELOCITY[1] = 1;
    }
    else{//if both front sensors see something then it will move left proportional the the ratio of thetwo distances (will move slighly left in this case
      VELOCITY[0] = (target_ARRAY[0]/target_ARRAY[1]);
      VELOCITY[1] = 1;
    }
    break;
   case 1:
   if(target_ARRAY[0]==0){//similar except to the right
      VELOCITY[0] = 1;
      VELOCITY[1] = 0.75;
    }
   else{
      VELOCITY[0] = 1;
      VELOCITY[1] = (target_ARRAY[1]/target_ARRAY[0]);
    }
    break;
   case 2:
      VELOCITY[0] = 0.75;
      VELOCITY[1] = -0.75;
      break;
   case 3:
      VELOCITY[0] = -0.75;
      VELOCITY[1] = 0.75;
      break;
  }
  SET_VELOCITY(); //use our set velocity function to change the velocity based on what the switch case says
  IR_DEFENSE_OFFENSE = true;//also telling our robot that we are in attack mode and to use the middle two IR sensors
}



int LOCK_ON(float DISTANCE_ARRAY[4]){ 
 
  // This function is exactly the same as the above one except it will move slower towards the target
  int target, target_ARRAY[4], minimum = 10000;
  for(int i = 0; i<2; i++){
    if(DISTANCE_ARRAY[i]<lock_on_distance){ 
      target_ARRAY[i]= DISTANCE_ARRAY[i];
    }
    else{
      target_ARRAY[i] = 0;
    }
  }
    for(int i = 0; i<2;i++){
    if(target_ARRAY[i]<minimum && target_ARRAY[i]!=0){
      minimum = target_ARRAY[i];
      target = i;
    }
  }
  switch(target){
    case 0://FRONT LEFT
    if(target_ARRAY[1]==0){
      VELOCITY[0] = 0.5;
      VELOCITY[1] = 0.75;
      
    }
    else{
      VELOCITY[0] = (target_ARRAY[0]/target_ARRAY[1])*0.75;
      VELOCITY[1] = 0.75;
    }
    break;
   case 1:
     if(target_ARRAY[0]==0){
        VELOCITY[0] = 0.75;
        VELOCITY[1] = 0.5;
      }
     else{
        VELOCITY[0] = 0.75;
        VELOCITY[1] = (target_ARRAY[1]/target_ARRAY[0])*0.75;
      }
    break;
    case 2:
      VELOCITY[0] = 0.75;
      VELOCITY[1] = -0.75;
      break;
   case 3:
      VELOCITY[0] = -0.75;
      VELOCITY[1] = 0.75;
      break;
  }
  SET_VELOCITY();
}

int PATROL(float DISTANCE_ARRAY[4]){
  //similarly this moves even slower except it has an extra case where it turns around if it detects nothing
  int target = 5, target_ARRAY[4], minimum = 10000;
  for(int i = 0; i<2; i++){
    if(DISTANCE_ARRAY[i]<max_distance){ //finds the position of the target, and creates array wish is easier to use the velocity function with, it also tells our robot we are no longer in attack mode at the bottom
      target_ARRAY[i]= DISTANCE_ARRAY[i];
    }
    else{
      target_ARRAY[i] = 0;
    }
  }
  for(int i = 0; i<2;i++){
    if(target_ARRAY[i]<minimum && target_ARRAY[i]!=0){
      minimum = target_ARRAY[i];
      target = i;
    }
  }
  switch(target){
    case 0://FRONT LEFT
    if(target_ARRAY[1]==0){
      VELOCITY[0] = 0.3;
      VELOCITY[1] = 0.5;
    }
    else{
      VELOCITY[0] = (target_ARRAY[0]/target_ARRAY[1])*0.5;
      VELOCITY[1] = 0.5;
    }
    break;
   case 1:
   if(target_ARRAY[0]==0){
      VELOCITY[0] = 0.5;
      VELOCITY[1] = 0.3;
    }
   else{
      VELOCITY[0] = 0.5;
      VELOCITY[1] = (target_ARRAY[1]/target_ARRAY[0])*0.5;
    }
    break;
   case 2:
      VELOCITY[0] = 0.75;
      VELOCITY[1] = -0.75;
      break;
   case 3:
      VELOCITY[0] = -0.75;
      VELOCITY[1] = 0.75;
      break;
   case 4:
      VELOCITY[0] = -0.5;
      VELOCITY[1] = 0.5;
      break;
  }
  SET_VELOCITY();
}
int DEFEND(bool DETECTION_ARRAY[5]){//Only gets invoked if we are near a white line

  bool target_arr[3];

  if(IR_DEFENSE_OFFENSE == true){
    target_arr[0] = DETECTION_ARRAY[2];
    target_arr[1] = DETECTION_ARRAY[3];
    target_arr[2] = DETECTION_ARRAY[4];
  }
  else{
    target_arr[0] = DETECTION_ARRAY[0];
    target_arr[1] = DETECTION_ARRAY[1];
    target_arr[2] = DETECTION_ARRAY[4];
  }
  if(target_arr[0] == true && target_arr[2] == true){ //if the front left sensor and the back sensor see a line, we will move forward and to the right
      VELOCITY[0] = 1;
      VELOCITY[1] = 0.8;
  }
  else if(target_arr[1]== true &&target_arr[2] == true){//similar except to the right
      VELOCITY[0] = 0.8;
      VELOCITY[1] = 1;
  }
  else if(target_arr[0] == true && target_arr[1] == true){
      VELOCITY[0] = -1;
      VELOCITY[1] = -1;
  }
  else if(target_arr[0] == true){//if only the left sensor sees something, we will turn back and to the right so the side sensor can hopefully see something
      VELOCITY[0] = -1;
      VELOCITY[1] = -0.25;
  }
  else if(target_arr[1] == true){//similar expect to the left
      VELOCITY[0] = -0.25;
      VELOCITY[1] = -1;
  }
  else if(target_arr[2] == true){//if our back is to the line then we must drive forward
      VELOCITY[0] = 1;
      VELOCITY[1] = 1;
  }

  SET_VELOCITY();
  //delay a little to give our robot time to complete this motion
}
void SET_VELOCITY(){

   if(VELOCITY[0]<0){
    digitalWrite(MOTOR[0],HIGH);
    digitalWrite(MOTOR[1], LOW);
    analogWrite(MOTOR[4], int(abs(VELOCITY[0])*255));
  }
  else{
    digitalWrite(MOTOR[1],HIGH);
    digitalWrite(MOTOR[0], LOW);
    analogWrite(MOTOR[4],int(VELOCITY[0]*255));
  }

  if(VELOCITY[1]<0){
    digitalWrite(MOTOR[2],HIGH);
    digitalWrite(MOTOR[3], LOW);
    analogWrite(MOTOR[5], int(abs(VELOCITY[1])*255));
  }
  else{
    digitalWrite(MOTOR[3],HIGH);
    digitalWrite(MOTOR[2], LOW);
    analogWrite(MOTOR[5],int(VELOCITY[1]*255));
  }
}
