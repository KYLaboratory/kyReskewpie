import processing.serial.*;

Serial port;
static final String SERIAL_PORT_NAME = Serial.list()[2];
static final int EFFECT_STOP_SIGNAL = 1;
static final int SPEED = 9600;
static final int WINDOW_WIDTH = 1350;
static final int WINDOW_HEIGHT = 700;
static final int BACKGROUND_COLOR = 0;

final int MAX_PARTICLE = 30;
Particle[] particles = new Particle[MAX_PARTICLE];
final int LIGHT_FORCE_RATIO = 10;
final int LIGHT_DISTANCE= 75 * 75;
final int BORDER = 75;
float baseRed, baseGreen, baseBlue;
float baseRedAdd, baseGreenAdd, baseBlueAdd;
final float RED_ADD = 100.2;
final float GREEN_ADD = 150.7;
final float BLUE_ADD = 202.3;

float xpos, ypos;
int readValue;
static final int MAX_READ_VALUE = 100;
int effectNumber;
boolean sentEffectStopSignal;

static final int ERROR_READ_VALUE = -1;
 
void setup() {
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  background(BACKGROUND_COLOR);
  noFill();
  
  for (int i = 0; i < MAX_PARTICLE; i++) {
    particles[i] = new Particle();
  }
  readValue = 0;
  effectNumber = 0;
  sentEffectStopSignal = false;
  xpos = 80;
  ypos = height / 2;
  
  baseRed = RED_ADD;
  baseGreen = GREEN_ADD;
  baseBlue = BLUE_ADD;
  
  if (connectsArduino()) {
    port = new Serial(this, SERIAL_PORT_NAME, SPEED);
  }
}

boolean connectsArduino() {
  for (String portName : Serial.list()) {
    if (portName.indexOf("usbmode") != -1) {
      return true;
    }
  }
  
  return false;
}
 
void draw() {
  if (effectNumber == 1) {
    println(sentEffectStopSignal);
    fill(0, 0, 0, 20);
    rect(0, 0, width, height);
    
    xpos += 30;
    
    colorOutCheck();
    
    for (int pid = 0; pid < MAX_PARTICLE; pid++) {
      particles[pid].move(xpos, ypos);
    }
    
    int tRed = (int)baseRed;
    int tGreen = (int)baseGreen;
    int tBlue = (int)baseBlue;
    
    tRed *= tRed;
    tGreen *= tGreen;
    tBlue *= tBlue;
    
    loadPixels();
    for (int pid = 0; pid < MAX_PARTICLE; pid++) {
      
      int left = max(0, particles[pid].x - BORDER);
      int right = min(width, particles[pid].x + BORDER);
      int top = max(0, particles[pid].y - BORDER);
      int bottom = min(height, particles[pid].y + BORDER);
      
      for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
          int pixelIndex = x + y * width;
          
          int r = pixels[pixelIndex] >> 16 & 0xFF;
          int g = pixels[pixelIndex] >> 8 & 0xFF;
          int b = pixels[pixelIndex] & 0xFF;
          
          int dx = x - particles[pid].x;
          int dy = y - particles[pid].y;
          int distance = (dx * dx) + (dy * dy);
          
          if (distance < LIGHT_DISTANCE) {
            int fixFistance = distance * LIGHT_FORCE_RATIO;
            if (fixFistance == 0) {
              fixFistance = 1;
            }   
            r = r + tRed / fixFistance;
            g = g + tGreen / fixFistance;
            b = b + tBlue / fixFistance;
          }
          
          pixels[x + y * width] = color(r, g, b);
        }
      }
    }
    updatePixels();
  }
  
  if (!sentEffectStopSignal && particleWindowOutCheck()) {
    if (connectsArduino()) {
      port.write(EFFECT_STOP_SIGNAL);
    }
    sentEffectStopSignal = true;
  }
}

void colorOutCheck() {
  if (baseRed < 10) {
    baseRed = 10;
    baseRedAdd *= -1;
  }
  else if (baseRed > 255) {
    baseRed = 255;
    baseRedAdd *= -1;
  }
 
  if (baseGreen < 10) {
    baseGreen = 10;
    baseGreenAdd *= -1;
  }
  else if (baseGreen > 255) {
    baseGreen = 255;
    baseGreenAdd *= -1;
  }
 
  if (baseBlue < 10) {
    baseBlue = 10;
    baseBlueAdd *= -1;
  }
  else if (baseBlue > 255) {
    baseBlue = 255;
    baseBlueAdd *= -1;
  }
}

boolean particleWindowOutCheck() {
  for (int i = 0; i < MAX_PARTICLE; i++) {
    if (particles[i].isOffScreen()) {
      return true;
    }
  }
  return false;
}

void serialEvent(Serial port) {
  readValue = port.read();
  println("read:" + readValue);
  
  for (int i = 0; i < MAX_PARTICLE; i++) {
    particles[i] = new Particle();
  }
  sentEffectStopSignal = false;
  effectNumber = decideEffectNumberFrom(readValue);
  xpos = 80;
  ypos = height / 2;
  background(BACKGROUND_COLOR);
  noFill();
}

int decideEffectNumberFrom(int readValue) {
  if (readValue < 0 || readValue > MAX_READ_VALUE) {
    return ERROR_READ_VALUE;
  }
  
  if (readValue < (MAX_READ_VALUE / 2)) {
    return 1;
  } else {
    return 2;
  }
}

void mousePressed() {
  for (int i = 0; i < MAX_PARTICLE; i++) {
    particles[i] = new Particle();
  }
  sentEffectStopSignal = false;
  effectNumber = 1;
  xpos = 80;
  ypos = height / 2;
  background(BACKGROUND_COLOR);
  noFill();
}

class Particle {
  int x, y;
  float vx, vy;
  float slowLevel;
  final float DECEL_RATIO = 1;
 
  Particle() {
    x = (int)random(2);
    y = (height / 2) + (int)random(100);
    slowLevel = 500 + random(1000);
  }
  
  void move(float targetX, float targetY) {
    
    vx = vx * DECEL_RATIO + (targetX - x) / slowLevel;
    vy = vy * DECEL_RATIO + (targetY - y) / slowLevel;
    
    x = (int)(x + vx);
    y = (int)(y + vy);
  }
  
  void explode() {
    vx = random(100) - 50;
    vy = random(100) - 50;
    slowLevel = random(100) + 5;
  }
  
  boolean isOffScreen() {
    return (x > width);
  }
}
