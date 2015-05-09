#include <Grove_LED_Bar.h>
#include <ChainableLED.h>

#define OUTPUT_SOLENOID 4
//#define INPUT_BUTTON 6
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
void notifyEffect(const struct vector3D& param1, const struct vector3D& param2, const ARMS_STATUS arms);

struct vector3D initParam1 = {0, 0, 0, 0};
struct vector3D initParam2 = {0, 0, 0, 0};

bool is_notify = true;  //notifyEffect() and actuatorLoop()


//actuation
int LEDbarValue = 3;//LED bar value
int solenoidState = HIGH;//solenoid
int HP; //hit point 

Grove_LED_Bar bar(9, 8, 0);  // Clock pin, Data pin, Orientation
ChainableLED leds(6, 7, NUM_LEDS);

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
  pinMode(OUTPUT_VIBRATOR, OUTPUT);
  pinMode(INPUT_BUTTON, INPUT);
  leds.init();
  
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

void notifyEffect(const struct vector3D& param1, const struct vector3D& param2, const ARMS_STATUS arms)
{  

  
  if(arms == ARMS_DOUBLE)
  {
    if(is_notify){
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
      Serial.println("DOUBLE");
      is_notify = false;
    }
  }
  else if(arms == ARMS_SINGLE)
  {
    if(is_notify){
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
      Serial.println("SINGLE");
      is_notify = false;
    }
  }
  else
  {
    const unsigned int wait_thresh = 100;
    static unsigned int wait_count = 0;
    if(wait_count == wait_thresh){
      Serial.println("READY");
      is_notify = true;
      wait_count = 0;
    }
    else{
      if(!is_notify)wait_count++;
    }
    
  }
  
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
    
    // if(is_notify)でシングルLED光らせる　else シングルLED消灯
    // hogehoge
    if(is_notify){
      leds.setColorRGB(0, 0, 0, 250);
    }else{
      leds.setColorRGB(0, 250, 0, 0);
    }
    
}


void serialEvent() {  
  const int DELAY_TIME_LEDbar=300;  //micro second
  const int DELAY_TIME_VIBRATOR=300;  //micro second
  if (Serial.read() == EFFECT_STOP_SIGNAL) {
    if(HP<=0){
      digitalWrite(OUTPUT_SOLENOID, HIGH);
      LEDbarValue = 0;
      bar.setLevel(LEDbarValue);
      delay(DELAY_TIME_LEDbar);
      digitalWrite(OUTPUT_SOLENOID, LOW);
      effectLED();
    }else if(HP>0){
      digitalWrite(OUTPUT_VIBRATOR, HIGH);
      delay(DELAY_TIME_VIBRATOR);
      digitalWrite(OUTPUT_VIBRATOR, LOW);      
    }
  }
}

void effectLED(){
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
