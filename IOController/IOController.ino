unsigned long init_x = 0;
unsigned long init_y = 0;
unsigned long init_z = 0;
unsigned long init_norm = 0;
#define OUTPUT_LED 12
#define SPEED 9600
#define ANALOG_PIN_X 3
#define ANALOG_PIN_Y 4
#define ANALOG_PIN_Z 5
#define EFFECT_START_SIGNAL 1
#define EFFECT_STOP_SIGNAL 2

void setup() {
  // put your setup code here, to run once:
  initializeSensor();
  initializeActuator();  
  Serial.begin(SPEED);  
}

void initializeSensor()
{
  init_x = analogRead(ANALOG_PIN_X);
  init_y = analogRead(ANALOG_PIN_Y);
  init_z = analogRead(ANALOG_PIN_Z);
  init_norm = sqrt(init_x * init_x + init_y * init_y + init_z * init_z);
/*
  Serial.print("X\t");
  Serial.println(init_x);
  Serial.print("Y\t");
  Serial.println(init_y);
  Serial.print("Z\t");
  Serial.println(init_z);  
  Serial.print("NORM\t");
  Serial.println(init_norm);
*/
}

void initializeActuator()
{
  
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorLoop();
  actuatorLoop();
  delay(1000);
}

void sensorLoop()
{
  unsigned long x = 0;
  unsigned long y = 0;
  unsigned long z = 0;
  unsigned long norm = 0;
  unsigned long diff = 0;
  const unsigned long thresh = 20;
  
  x = analogRead(3);
  y = analogRead(4);
  z = analogRead(5);
  norm = sqrt(x*x + y*y + z*z);
  diff = getDifference(norm, init_norm);
  
  if(diff > thresh){
    //Serial.print("HA\t");
    //Serial.print(diff);
    unsigned int effect_parameter = diff/10;
    Serial.print(effect_parameter);
    Serial.print('\n');
    //Serial.println(norm);
    //Serial.println(init_norm);
  }
/*
  Serial.print("X\t");
  Serial.println(x);
  Serial.print("Y\t");
  Serial.println(y);
  Serial.print("Z\t");
  Serial.println(z);  
  Serial.print("NORM\t");
  Serial.println(norm);
*/
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

void actuatorLoop()
{
  if (Serial.available()) {
    if (Serial.read() == EFFECT_STOP_SIGNAL) {
      digitalWrite(OUTPUT_LED, HIGH);
    } else {
      digitalWrite(OUTPUT_LED, LOW);
    }
  }
}




