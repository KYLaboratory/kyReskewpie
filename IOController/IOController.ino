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
#define DELAY_TIME 50

void setup() {
  // put your setup code here, to run once:
  Serial.begin(SPEED);
  initializeSensor();
  initializeActuator();
}

void initializeSensor()
{
  init_x = analogRead(ANALOG_PIN_X);
  init_y = analogRead(ANALOG_PIN_Y);
  init_z = analogRead(ANALOG_PIN_Z);
  init_norm = sqrt(init_x * init_x + init_y * init_y + init_z * init_z);
  Serial.print("delta_x\t");
  Serial.print("delta_y\t");
  Serial.print("delta_z\t");
  Serial.print("delta_norm");
}

void initializeActuator()
{
  
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorLoop();
  actuatorLoop();
  delay(DELAY_TIME);
}

void sensorLoop()
{
  unsigned long x = 0;
  unsigned long y = 0;
  unsigned long z = 0;
  unsigned long norm = 0;
  unsigned long delta_norm = 0;
  unsigned long delta_x = 0;
  unsigned long delta_y = 0;
  unsigned long delta_z = 0;
  const unsigned long thresh = 20;
  
  x = analogRead(ANALOG_PIN_X);
  y = analogRead(ANALOG_PIN_Y);
  z = analogRead(ANALOG_PIN_Z);
  norm = sqrt(x*x + y*y + z*z);
  delta_norm = getDifference(norm, init_norm);
  delta_x = getDifference(x, init_x);
  delta_y = getDifference(y, init_y);
  delta_z = getDifference(z, init_z);
  
  if(delta_norm > thresh){
    notifyEffect();
  }

  Serial.print(delta_x);
  Serial.print("\t");
  Serial.print(delta_y);
  Serial.print("\t");
  Serial.print(delta_z);  
  Serial.print("\t");
  Serial.print(delta_norm);
  Serial.print("\n");
}

void notifyEffect()
{
  unsigned int effect_parameter = delta_norm/10;
  Serial.print(effect_parameter);
  Serial.print('\n');
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




