#include <Servo.h>
#define BIT_RATE 9600
#define MEDIUM_GLASS_TIME 2500
#define FULL_GLASS_TIME 5000
#define SPEED_PUMP 180


const int button_list[] = {A0,A1,A2,A3,A4};
const int pump_list [] = {9,10};
const int b_play = A5;
const int b_right = 1;
const int b_left=2;
const int servo = 11;
const int motor_stepper[][2]={{3,4},{5,6}};
const int fdc_stepper1[] ={7,8};/* {left_end_course , right_end_course}*/
const int fdc_stepper2[] = {11,12}; /*{up_end_course , down_end_course}*/
const int piece_here = 13;
Servo myservo;  // create servo object to control a servo

int pos_servo = 0;    // variable to store the servo position
long time_pump;
boolean pump_act = false;
boolean need_active_pump = false;
int last_pump ;
int last_type_pump;

void setup() {
  init_pin();
  Serial.begin(BIT_RATE);
  myservo.attach(servo);  // attaches the servo on pin 9 to the servo object
  reset();
}

void loop() {
  button_move();
  playing();
  active_pump(last_pump, last_type_pump);
  
}

void init_pin()
{
  
  pinMode(b_left,INPUT);
  pinMode(b_right,INPUT);
  pinMode(servo,OUTPUT);
  pinMode(piece_here,INPUT);
  for(int i = 0 ; i < sizeof(pump_list); i++) pinMode(pump_list[i],OUTPUT);
  for(int i = 0 ; i < sizeof(motor_stepper); i++)
  {
    pinMode(motor_stepper[i][0],OUTPUT); pinMode(motor_stepper[i][1],OUTPUT);  
  }
  for(int i = 0 ; i < sizeof(fdc_stepper1); i++)
  {
    pinMode(fdc_stepper1[i],INPUT); pinMode(fdc_stepper2[i],INPUT); 
  }
  
  
}
void button_move()
{
  if(button_active(b_right,false))
  {
    if(!button_active(fdc_stepper1[1],false))
      make_step(20,0,true);
  }
  if(button_active(b_left,false))
  {
    if(!button_active(fdc_stepper1[0],false))
      make_step(20,0,true);
  }
  
}
void reset()
{
  
  open_plier(myservo);
  while(!button_active(fdc_stepper1[1],false) or !button_active(fdc_stepper2[0],false))
  { 
    if(!button_active(fdc_stepper1[1],false))
        make_step(5,0,true);
    if(!button_active(fdc_stepper2[0],false) and piece_here )
        make_step(5,1,true);
    active_pump(last_pump, last_type_pump);
  }
  delay(500);
  close_plier(myservo); 
  while(!button_active(fdc_stepper2[1],false))
  {
    active_pump(last_pump, last_type_pump);
    make_step(5,1,true); 
  }
  while(pump_act)
    active_pump(last_pump, last_type_pump);
  send_message(4,"");
}
boolean button_active( int button , boolean analog)
{
  boolean is_active = false;
  if (analog)
  {
    if(analogRead(button)>=900) is_active =true;
  }
  else
  {
    if(digitalRead(button)) is_active = true;
  }
  return is_active;
}
void playing()
{
  if(button_active(b_play,false))
  {
    open_plier(myservo);
    send_message(1,"");
    int value = -1;
    while(value == -1)
      {
        value = detect_button_drink();
        if(!button_active(fdc_stepper1[1],false)) 
          make_step(3,0,true);
      }
    send_message(2,(String)(""+(String)(value+1)));
    setup_pump(value);
    active_pump(last_pump, last_type_pump);
    reset();
  }
}
void setup_pump(int value)
{
  switch(value)
  {
    case 0: last_pump = 1; last_type_pump = 1; break;
    case 1: last_pump = 1; last_type_pump = 1; break;
    case 2: last_pump = 1; last_type_pump = 1; break;
    case 3: last_pump = 1; last_type_pump = 1; break;
    case 4: last_pump = 1; last_type_pump = 1; break;
  }
  need_active_pump = true;
}
void init_button_drink()
{
  for (int i = 0 ; i <sizeof(button_list) ; i++)
    pinMode(button_list[i],INPUT);
}
int detect_button_drink ()
{
  int val_return = -1;  
  for (int i  =0 ; i <sizeof(button_list) ; i++)
  {
      if(button_active(button_list[i],true))
        val_return = i;
  }
  return val_return;
}
void make_step(int number_of_step,int stepper_number,boolean way)
{
  /*
    clockwise way stage = 
    anticlockwise way tage = 
  */
  digitalWrite(motor_stepper[stepper_number][1],way);
  for(int i =0 ; i < number_of_step ; i++)
  {
    digitalWrite(motor_stepper[stepper_number][0],HIGH);
    delay(3);
    digitalWrite(motor_stepper[stepper_number][0],LOW);
    delay(3); 
  }
}
void open_plier(Servo servo)
{
  for (pos_servo  = 86; pos_servo  <= 110; pos_servo  += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos_servo );              // tell servo to go to position in variable 'pos'
    delay(50);                       // waits 15ms for the servo to reach the position
  }
}
void close_plier(Servo servo)
{
    for (pos_servo  = 110; pos_servo  >= 86; pos_servo  -= 1) { // goes from 180 degrees to 0 degrees
    servo.write(pos_servo);              // tell servo to go to position in variable 'pos'
    delay(50);                       // waits 15ms for the servo to reach the position
  }
}


void active_pump(int number_pump, int type)
{
  int time_max;
  switch (type)
  {
    case 1:  time_max = FULL_GLASS_TIME;break;
    case 2 : time_max = MEDIUM_GLASS_TIME;break;
    default : break;
  }
   if (!pump_act and need_active_pump)
   {
      time_pump = millis();
      need_active_pump = false;
      pump_act = true;
      analogWrite(pump_list[number_pump],SPEED_PUMP);
   }
   else
   {
      if(millis()-time_pump >=time_max)
      {
        analogWrite(number_pump,0);
        pump_act = false;
        send_message(3,"");
      }
   }
}
void send_message( int id_message, String complement)
{
  switch (id_message) 
  {
    case 1:Serial.println("!P?");break;/*Boutton "Play"*/
    case 2:Serial.println("!"+complement+"?");break;/*La pièce tombe dans le trou complement : */
    case 3:Serial.println("!D?");break;/*Le verre a fini d'être rempli :*/
    case 4:Serial.println("!R?");break;/*Le système est reset*/
  }
}


