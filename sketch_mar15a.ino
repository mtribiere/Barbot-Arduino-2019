#include <Servo.h>
#include "BasicStepperDriver.h"

#define BIT_RATE 9600


#define MOTOR_STEPS 400
#define MICROSTEPS 1
#define RPM 120


#define MEDIUM_GLASS_TIME 2500
#define FULL_GLASS_TIME 5000
#define SPEED_PUMP 255


#define SENS1 true
#define SENS2 false 

/*
 * TO DO: FAIRE DES TESTS POUR LES SENS DES STEPPER ET MODIFIER ENSUITE LE CODE EN CONSEQUENCE ( A CHAQUE MAKE STEP)
 * METTRE A JOUR LES DIFFERENTS MODE POUR LES POMPES
 * 
 */


/*déclaration entrée sortie*/
const int button_list[] = {A0,A1,A2,A3};
const int pump_list [] = {9,10};
const int b_play = 1;
const int b_right = A5;
const int b_left=A4;
const int servo = 11;/* Besoin d'être une Pin PWM*/
const int motor_stepper[][2]={{3,4},{6,5}};/* format pin de pas ,  sens du pas*/
const int fdc_stepper1[] ={2,8};/* {left_end_course , right_end_course}*/
const int fdc_stepper2[] = {13,12}; /*{up_end_course , down_end_course}*/

BasicStepperDriver stepper_VE(MOTOR_STEPS, motor_stepper[0][1], motor_stepper[0][0]);/* module coté analogie*/
BasicStepperDriver stepper_HO(MOTOR_STEPS, motor_stepper[1][1], motor_stepper[1][0]);/*module coté digital*/

Servo myservo;  // create servo object to control a servo
/* variable global du code */
int pos_servo = 0;    // variable to store the servo position
long time_pump;
boolean pump_act = false;
boolean need_active_pump = false;
int pump ;
int type_pump;
/*fonction d'initialisation*/
void setup() {
  init_pin();
  Serial.begin(BIT_RATE);
  myservo.attach(servo);  // attaches the servo on pin 9 to the servo object
  setup_pump(0);
  reset();
}

void loop() {
  button_move();
  playing();
  active_pump(pump, type_pump);
  
  
  
}
/* fonction de declaration si les pins sont des entrées sortie*/
void init_pin()
{
  
 // pinMode(b_left,INPUT);
  //pinMode(b_right,INPUT);
  pinMode(b_play,INPUT);
  pinMode(servo,OUTPUT);

  for(int pump = 0 ; pump < sizeof(pump_list); pump++) pinMode(pump_list[pump],OUTPUT);
  stepper_VE.begin(RPM, MICROSTEPS);
  stepper_HO.begin(RPM, MICROSTEPS);
  for(int fdc_step = 0 ; fdc_step < sizeof(fdc_stepper1); fdc_step++)
  {
    pinMode(fdc_stepper1[fdc_step],INPUT); pinMode(fdc_stepper2[fdc_step],INPUT); 
  }
}
/* detection if  button right or left is press and make few step on the first stepper*/
void button_move()
{
  if(button_active(b_right,true))
  {
    if(!button_active(fdc_stepper1[1],false))
      make_step(10,stepper_VE,SENS1);
  }
  if(button_active(b_left,true))
  {
    if(!button_active(fdc_stepper1[0],false))
      make_step(10,stepper_VE,SENS2);
  }
  
}
/* this fonction place the system in his main position*/
/*
 * pour la mise en route il faudra mettre la pièce sur sur le remonte pièce
 */
void reset()
{
  
  open_plier(myservo);
  while(!button_active(fdc_stepper1[1],false) or !button_active(fdc_stepper2[0],false))
  { 
    if(!button_active(fdc_stepper1[1],false))
        make_step(5,stepper_VE,SENS1);
    if(!button_active(fdc_stepper2[0],false) )
        make_step(5,stepper_HO,SENS1);
    active_pump(pump, type_pump);
  }
  delay(500);
  close_plier(myservo); 
  while(!button_active(fdc_stepper2[1],false))
  {
    active_pump(pump, type_pump);
    make_step(5,stepper_HO,SENS2); 
  }
  while(pump_act)
    active_pump(pump, type_pump);
  send_message(4,"");
}
/* detect if a button is activ, no matter if it's an analog input*/
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
/* start the sequence of playing (let fall the piece and wait until the piece pass in front of one of the button*/
void playing()
{
  if(button_active(b_play,false))
  {
    open_plier(myservo);
    send_message(1,"");
    int value;
    do
    {
        value = detect_button_drink();
        if(!button_active(fdc_stepper1[1],false)) 
          make_step(3,stepper_VE,SENS1);
      
    } while(value == -1);
      
    send_message(2,(String)(""+(String)(value+1)));
    setup_pump(value);
    active_pump(pump, type_pump);
    reset();
  }
}
/* this fonction setup the pump in fonction of the value 
TO DO: modify the number of the pump and the type*/
void setup_pump(int value)
{
  switch(value)
  {
    case 0: pump = 0; type_pump = 1; break;
    case 1: pump = 0; type_pump = 2; break;
    case 2: pump = 1; type_pump = 1; break;
    case 3: pump = 1; type_pump = 2; break;
  }
  need_active_pump = true;
}
/* usless because we are are using analog input*/
void init_button_drink()
{
  for (int i = 0 ; i <sizeof(button_list) ; i++)
    pinMode(button_list[i],INPUT);
}

/*detect wich button is press and return his number*/
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
/* this fonction do a number of step give for the drv8825 we could try to change the value of the delay between step*/
void make_step(int number_of_step,BasicStepperDriver stepper,boolean way)
{
  int sens;
  int number_step = int(float(36/40)*number_of_step);
  
  if(way) sens = 1; else sens=-1;
  stepper.move(sens*MOTOR_STEPS*MICROSTEPS);
  stepper.rotate(number_step);
}
void open_plier(Servo servo)
{
  for (pos_servo  = 86; pos_servo  <= 110; pos_servo  += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos_servo );              // tell servo to go to position in variable 'pos'
    delay(50);
    Serial.println(pos_servo);// waits 15ms for the servo to reach the position
  }
}
void close_plier(Servo servo)
{
    for (pos_servo  = 110; pos_servo  >= 86; pos_servo  -= 1) { // goes from 180 degrees to 0 degrees
    servo.write(pos_servo);              // tell servo to go to position in variable 'pos'
    delay(50);
    Serial.println(pos_servo);// waits 15ms for the servo to reach the position
  }
  
}


/* activ the pump and give the liquid (no blocking) need to call it in each loop*/
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
      if(millis()-time_pump >=time_max and pump_act)
      {
        analogWrite(pump_list[number_pump],0);
        pump_act = false;
        send_message(3,"");
      }
   }
   
}
/* comunication with the computer*/
void send_message( int id_message, String complement)
{
  switch (id_message) 
  {
    case 1:Serial.println("!P?");break;/*Boutton "Play"*/
    case 2:Serial.println("!R"+complement+"?");break;/*La pièce tombe dans le trou complement : */
    case 3:Serial.println("!D?");break;/*Le verre a fini d'être rempli :*/
    case 4:Serial.println("!E?");break;/*Le système est reset*/
  }
}
