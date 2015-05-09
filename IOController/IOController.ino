#include <Grove_LED_Bar.h>

#define OUTPUT_SOLENOID 4
#define INPUT_BUTTON 6
#define OUTPUT_VIBRATOR 2

#define OUTPUT_LED 12
#define SPEED 9600

#define ANALOG_PIN_X1 3
#define ANALOG_PIN_Y1 4
#define ANALOG_PIN_Z1 5

#define EFFECT_START_SIGNAL 1
#define EFFECT_STOP_SIGNAL 1

#define DELAY_TIME 50

//actuation
int LEDbarValue = 3;//LED bar value
int solenoidState = HIGH;//solenoid
int HP; //hit point 

Grove_LED_Bar bar(9, 8, 0);  // Clock pin, Data pin, Orientation

struct vector3D
{
  unsigned long x;
  unsigned long y;
  unsigned long z;
  unsigned long norm;
};

struct vector3D initParam1 = {0, 0, 0, 0};

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
  const unsigned long thresh = 300;
  
  const struct vector3D currentParam = createVector3D(analogRead(ANALOG_PIN_X1), analogRead(ANALOG_PIN_Y1), analogRead(ANALOG_PIN_Z1));
  const struct vector3D deltaParam = calcDiffVector3D(currentParam, initParam1);
  
  if(deltaParam.x > thresh){
    notifyEffect(deltaParam);
  }
}

struct vector3D calcDiffVector3D(const struct vector3D& currentParam, const struct vector3D& initParam)
{
  struct vector3D deltaAccParam = createVector3D(
    getDifference(currentParam.x, initParam.x), 
    getDifference(currentParam.y, initParam.y),
    getDifference(currentParam.z, initParam.z));
  return deltaAccParam;
}

void notifyEffect(const struct vector3D& param)
{
  //unsigned int parameter = rv_parameter/10;
  //Serial.print(parameter);
  //Serial.print('\n');
  Serial.print(param.x);
  Serial.print("\t");
  Serial.print(param.y);
  Serial.print("\t");
  Serial.print(param.z);  
  Serial.print("\t");
  Serial.print(param.norm);
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
