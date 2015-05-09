#include <Grove_LED_Bar.h>

#define OUTPUT_SOLENOID 4
#define INPUT_BUTTON 6
#define OUTPUT_VIBRATOR 2

#define OUTPUT_LED 12
#define SPEED 9600

#define ANALOG_PIN_X1 3
#define ANALOG_PIN_Y1 4
#define ANALOG_PIN_Z1 5

#define ANALOG_PIN_X2 6
#define ANALOG_PIN_Y2 7
#define ANALOG_PIN_Z2 8

#define EFFECT_START_SIGNAL 1
#define EFFECT_STOP_SIGNAL 1

#define DELAY_TIME 50

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

struct vector3D initParam1 = {0, 0, 0, 0};
struct vector3D initParam2 = {0, 0, 0, 0};

//actuation
int LEDbarValue = 3;//LED bar value
int solenoidState = HIGH;//solenoid
int HP; //hit point 

Grove_LED_Bar bar(9, 8, 0);  // Clock pin, Data pin, Orientation


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
  
  Serial.print("delta_x\t");
  Serial.print("delta_y\t");
  Serial.print("delta_z\t");
  Serial.print("delta_norm\n");
}

void initializeActuator()
{
  pinMode(OUTPUT_SOLENOID, OUTPUT);
  pinMode(INPUT_BUTTON, INPUT);
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
  
  switch(judgeArmsStatus(deltaParam1, deltaParam2))
  {
    case ARMS_DOUBLE:
      notifyEffect(deltaParam1, deltaParam2, 2);
      break;
    case ARMS_SINGLE:
      notifyEffect(deltaParam1, deltaParam2, 1);
      break;
    default:
      break;
  }
  
}

ARMS_STATUS judgeArmsStatus(const struct vector3D& param1, const struct vector3D& param2)
{
  const unsigned long thresh = 300;
  
  const bool isArm1Moving = param1.x > thresh;
  const bool isArm2Moving = param2.x > thresh;
  
  if(isArm1Moving && isArm2Moving)
  {
    return ARMS_DOUBLE;
  }
  else if(isArm1Moving || isArm2Moving)
  {
    return ARMS_SINGLE;
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

void notifyEffect(const struct vector3D& param1, const struct vector3D& param2, const int arms)
{
  //unsigned int parameter = rv_parameter/10;
  //Serial.print(parameter);
  //Serial.print('\n');
  Serial.print(param1.x);
  Serial.print("\t");
  Serial.print(param1.y);
  Serial.print("\t");
  Serial.print(param1.z);  
  Serial.print("\t");
  Serial.print(param1.norm);
  Serial.print("\t");
  Serial.print(param2.x);
  Serial.print("\t");
  Serial.print(param2.y);
  Serial.print("\t");
  Serial.print(param2.z);  
  Serial.print("\t");
  Serial.print(param2.norm);
  Serial.print("\t");
  if(arms == 2)
  {
    Serial.print("DOUBLE");
  }
  else if(arms == 1)
  {
    Serial.print("SINGLE");
  }
  else
  {
    Serial.print("NONE");
  }
  Serial.print("\n");
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
    //　ここから
    if(HP>0){
      LEDbarValue = HP;
    }else if(HP<=0){
      LEDbarValue = 0;
    }
    bar.setLevel(LEDbarValue);
    // ここまで　をIOControllerにマージする。
}


void serialEvent() {  
  if (Serial.read() == EFFECT_STOP_SIGNAL) {
    if(HP<=0){
      digitalWrite(OUTPUT_SOLENOID, HIGH);
      LEDbarValue = 0;
      bar.setLevel(LEDbarValue);
      delay(300);
      digitalWrite(OUTPUT_SOLENOID, LOW);
    }else if(HP>0){
      digitalWrite(OUTPUT_VIBRATOR, HIGH);
      delay(300);
      digitalWrite(OUTPUT_VIBRATOR, LOW);      
    }
  }
}
