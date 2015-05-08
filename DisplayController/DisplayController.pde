import processing.serial.*;

Serial port;
static final String SERIAL_PORT_NAME = Serial.list()[2];
static final int EFFECT_START_SIGNAL = 1;
static final int EFFECT_STOP_SIGNAL = 1;
static final int SPEED = 9600;
int readValue = 0;

static final int WINDOW_WIDTH = 1350;
static final int WINDOW_HEIGHT = 700;
static final int BACKGROUND_COLOR = 0;

final int MAX_PARTICLE = 30;
Particle[] p = new Particle[MAX_PARTICLE];
 
final int LIGHT_FORCE_RATIO = 10;
final int LIGHT_DISTANCE= 75 * 75;
final int BORDER = 75;
 
float baseRed, baseGreen, baseBlue;
float baseRedAdd, baseGreenAdd, baseBlueAdd;
final float RED_ADD = 100.2;
final float GREEN_ADD = 150.7;
final float BLUE_ADD = 202.3;

float xpos, ypos;
int inEffect = 0;
 
void setup() {
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  background(BACKGROUND_COLOR);
  noFill();
  
  xpos = 80;
  ypos = height / 2;
  
  baseRed = RED_ADD;
  baseGreen = GREEN_ADD;
  baseBlue = BLUE_ADD;
  
  port = new Serial(this, SERIAL_PORT_NAME, SPEED);
}
 
void draw() {
  if (inEffect == 1) {
    fill(0, 0, 0, 20);
    rect(0, 0, width, height);
    
    xpos += 30;
    
    colorOutCheck();
    windowOutCheck();
    
    for (int pid = 0; pid < MAX_PARTICLE; pid++) {
      p[pid].move(xpos, ypos);
    }
    
    int tRed = (int)baseRed;
    int tGreen = (int)baseGreen;
    int tBlue = (int)baseBlue;
    
    tRed *= tRed;
    tGreen *= tGreen;
    tBlue *= tBlue;
    
    loadPixels();
    for (int pid = 0; pid < MAX_PARTICLE; pid++) {
      
      int left = max(0, p[pid].x - BORDER);
      int right = min(width, p[pid].x + BORDER);
      int top = max(0, p[pid].y - BORDER);
      int bottom = min(height, p[pid].y + BORDER);
      
      for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
          int pixelIndex = x + y * width;
          
          int r = pixels[pixelIndex] >> 16 & 0xFF;
          int g = pixels[pixelIndex] >> 8 & 0xFF;
          int b = pixels[pixelIndex] & 0xFF;
          
          int dx = x - p[pid].x;
          int dy = y - p[pid].y;
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
}

void mousePressed() {
  for (int i = 0; i < MAX_PARTICLE; i++) {
    p[i] = new Particle();
  }
  inEffect = 1;
  xpos = 80;
  ypos = height / 2;
  background(BACKGROUND_COLOR);
  noFill();
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

void windowOutCheck() {
  for (int i = 0; i < MAX_PARTICLE; i++) {
    if (p[i].isOffScreen()) {
      port.write(EFFECT_STOP_SIGNAL);
    }
  }
}

// todo: change me
void serialEvent(Serial p){
  readValue = p.read();
  println("read:" + readValue);
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
