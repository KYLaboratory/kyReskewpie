#include <Grove_LED_Bar.h>
#include <ChainableLED.h>
#include <Servo.h>

#define OUTPUT_SOLENOID 4
#define OUTPUT_VIBRATOR 2
#define OUTPUT_SERVO 3

#define OUTPUT_LED 13
#define SPEED 9600

#define ANALOG_PIN_X1 3
#define ANALOG_PIN_Y1 4
#define ANALOG_PIN_Z1 5

#define ANALOG_PIN_X2 6
#define ANALOG_PIN_Y2 7
#define ANALOG_PIN_Z2 8

#define EFFECT_STOP_SIGNAL 1

#define DELAY_TIME 50
#define NUM_LEDS 1

//sensor
struct vector3D
{
  unsigned long x;
  unsigned long y;
  unsigned long z;
  unsigned long norm;
};

enum ARMS_STATUS
{
  ARMS_NONE,
  ARMS_SINGLE,
  ARMS_DOUBLE
};

ARMS_STATUS judgeArmsStatus(const struct vector3D& param1, const struct vector3D& param2);
void notifyEffect(const struct vector3D& param1, const struct vector3D& param2, const ARMS_STATUS arms);

struct vector3D initParam1 = {0, 0, 0, 0};
struct vector3D initParam2 = {0, 0, 0, 0};

bool is_notify = true;  //notifyEffect() and actuatorLoop()


//actuation
int LEDbarValue = 3;//LED bar value
int solenoidState = HIGH;//solenoid
int HP = 10; //hit point 

Grove_LED_Bar bar(9, 8, 0);  // Clock pin, Data pin, Orientation
ChainableLED leds(6, 7, NUM_LEDS);

Servo myservo;  // create servo object to control a servo

void setup() {
  // put your setup code here, to run once:
  Serial.begin(SPEED);
  initializeSensor();
  initializeActuator();
}

struct vector3D createVector3D(unsigned long x, unsigned long y, unsigned long z)
{
  struct vector3D newAccParam = {x, y, z, sqrt(x * x + y * y + z * z)};
  return newAccParam;
}

void initializeSensor()
{  
  initParam1 = createVector3D(analogRead(ANALOG_PIN_X1), analogRead(ANALOG_PIN_Y1), analogRead(ANALOG_PIN_Z1));
  initParam2 = createVector3D(analogRead(ANALOG_PIN_X2), analogRead(ANALOG_PIN_Y2), analogRead(ANALOG_PIN_Z2));
}

void initializeActuator()
{
  pinMode(OUTPUT_SOLENOID, OUTPUT);
  pinMode(OUTPUT_VIBRATOR, OUTPUT);
  leds.init();
  myservo.attach(OUTPUT_SERVO);  // attaches the servo on pin 9 to the servo object
  pinMode(OUTPUT_LED, OUTPUT);
  digitalWrite(OUTPUT_LED, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorLoop();
  actuatorLoop();
  delay(DELAY_TIME);
}

void sensorLoop()
{
  const struct vector3D currentParam1 = createVector3D(analogRead(ANALOG_PIN_X1), analogRead(ANALOG_PIN_Y1), analogRead(ANALOG_PIN_Z1));
  const struct vector3D deltaParam1 = calcDiffVector3D(currentParam1, initParam1);
  
  const struct vector3D currentParam2 = createVector3D(analogRead(ANALOG_PIN_X2), analogRead(ANALOG_PIN_Y2), analogRead(ANALOG_PIN_Z2));
  const struct vector3D deltaParam2 = calcDiffVector3D(currentParam2, initParam2);
  
  notifyEffect(deltaParam1, deltaParam2, judgeArmsStatus(deltaParam1, deltaParam2));
}

ARMS_STATUS judgeArmsStatus(const struct vector3D& param1, const struct vector3D& param2)
{
  const unsigned long thresh = 300;
  const unsigned long subthresh = 200;
  
  const bool isArm1Moving = param1.x > thresh;
  const bool isArm2Moving = param2.x > thresh;
  const bool isArm1MovingGentle = param1.x > subthresh;
  const bool isArm2MovingGentle = param2.x > subthresh;

  if((isArm1Moving && isArm2Moving)
  || (isArm2Moving && isArm1MovingGentle)
  || (isArm1Moving && isArm2MovingGentle)){
    return ARMS_DOUBLE;
  }
  else if(isArm1Moving || isArm2Moving){
    return ARMS_SINGLE;
  }
  else{
  }
  return ARMS_NONE;
}

struct vector3D calcDiffVector3D(const struct vector3D& currentParam, const struct vector3D& initParam)
{
  struct vector3D deltaAccParam = createVector3D(
    getDifference(currentParam.x, initParam.x), 
    getDifference(currentParam.y, initParam.y),
    getDifference(currentParam.z, initParam.z));
  return deltaAccParam;
}

void notifyEffect(const struct vector3D& param1, const struct vector3D& param2, const ARMS_STATUS arms)
{  
  const unsigned int data_length = 20;
  const unsigned long parameter_max = 468;
  char data[data_length] = {0};
  
  if(arms == ARMS_DOUBLE)
  {
    if(is_notify){
      sprintf(data, "d,%d", ((param1.x + param2.x) / 2) * 100 / parameter_max );
      Serial.println(data);
      decrementN(HP, 5);
      is_notify = false;
      vibration();
    }
  }
  else if(arms == ARMS_SINGLE)
  {
    if(is_notify){
      decrementN(HP, 1);
      if(param1.x < param2.x){ 
        sprintf(data, "s,%d", param2.x * 100 / parameter_max);
      }
      else{
        sprintf(data, "s,%d",param1.x * 100 / parameter_max);
      }
      Serial.println(data);
      is_notify = false;
      vibration();
    }
  }
  else{
  }  
}

void vibration(){
  const int DELAY_TIME_VIBRATOR=300;  //ms

  digitalWrite(OUTPUT_VIBRATOR, HIGH);
  delay(DELAY_TIME_VIBRATOR);
  digitalWrite(OUTPUT_VIBRATOR, LOW); 
}

void decrementN(int& rv_value, int n)
{
  for(int i = 0; i < n; i++)if(rv_value > 0)rv_value--;
}


unsigned long getDifference(unsigned long rv_a, unsigned long rv_b)
{
  if(rv_a > rv_b){
    return(rv_a - rv_b);
  }
  else{
    return(rv_b - rv_a);
  }
}

void actuatorLoop()// LEDbarのみの記述
{
    if(HP>0){
      LEDbarValue = HP;
    }else if(HP<=0){
      LEDbarValue = 0;
    }
    bar.setLevel(LEDbarValue);

    if(is_notify){
      leds.setColorRGB(0, 0, 0, 250);
    }else{
      leds.setColorRGB(0, 250, 0, 0);
    }
    
}


void serialEvent() {  
  const int DELAY_TIME_LEDbar=300;  //ms
  const int DELAY_TIME_SERVO=2000;  //ms
  if (Serial.read() == EFFECT_STOP_SIGNAL) {
    is_notify = true;
    if(HP<=0){  // finish game
      digitalWrite(OUTPUT_SOLENOID, HIGH);
      LEDbarValue = 0;
      bar.setLevel(LEDbarValue);
      delay(DELAY_TIME_LEDbar);
      digitalWrite(OUTPUT_SOLENOID, LOW);
      myservo.write(120); //サーボを動かす(120度)
      leds.setColorRGB(0, 0, 0, 0); //RGB LED OFF
      digitalWrite(13, LOW);
      //effectLED();
      delay(DELAY_TIME_SERVO);
      myservo.write(80); //サーボを動かす(80度)
      
    }else if(HP>0){  
      //no action
    }
  }
}

void effectLED(){ // effect LEDbar in finish time
  const int DELAY_TIME_EFFECTLED = 700;
  int k;
  for(k=0;k<3;k++){
    // Turn on LEDs 1, 2, 3, 4, 5
    // 0b000000000011111 == 0x1F
    bar.setBits(0b000001010101010);
    delay(DELAY_TIME_EFFECTLED);
  
    // Turn on LEDs 6, 7, 8, 9, 10
    // 0b000001111100000 == 0x3E0
    bar.setBits(0b000000101010101);
    delay(DELAY_TIME_EFFECTLED);
  }
  for(k=0;k<3;k++){
    bar.setBits(0b000001111111111);
    delay(DELAY_TIME_EFFECTLED);
    bar.setBits(0b000000000000000);
    delay(DELAY_TIME_EFFECTLED);
  }
}

