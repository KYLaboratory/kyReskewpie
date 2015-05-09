import processing.serial.*;
import ddf.minim.*;

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
String readCommand;
int readValue;
static final int MAX_READ_VALUE = 100;
int effectNumber;
boolean sentEffectStopSignal;
boolean canPlaySE;

float halfWidth, halfHeight;
final int MAX_LINE = 40;
NeonLine[] neonLine = new NeonLine[MAX_LINE];
final int DRAW_TIMES = 15;
final int BORDER_DEAD_COUNT = 8;
boolean neonEffectStarted = false;
int neonStrength;
static final int NEON_WEAK = 2;
static final int NEON_STRONG = 4;

static final int ERROR_READ_VALUE = -1;
static final int ERROR_READ_COMMAND = -2;

Minim minim;
AudioSample fx1;
AudioSample fx2;
AudioSample fx_damage;
AudioSample fx_exp;
 
void setup() {
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  initializeDisplay();
  
  readCommand = "";
  readValue = 0;
  effectNumber = 0;
  halfWidth = width / 2;
  halfHeight = height / 2;
  neonStrength = NEON_WEAK;
  canPlaySE = false;
  
  for(int i = 0; i < MAX_LINE; i++){
    neonLine[i] = new NeonLine();
  }
  
  if (connectsArduino()) {
    port = new Serial(this, SERIAL_PORT_NAME, SPEED);
  }
  
  // audio setup
  minim = new Minim(this);
  fx1 = minim.loadSample("fx_single.wav");
  fx2 = minim.loadSample("fx_double.wav");
  fx_damage = minim.loadSample("fx_damage.wav");
  fx_exp = minim.loadSample("fx_exp.wav");
}

void initializeDisplay() {
  background(BACKGROUND_COLOR);
  for (int i = 0; i < MAX_PARTICLE; i++) {
    particles[i] = new Particle();
  }
  baseRed = RED_ADD;
  baseGreen = GREEN_ADD;
  baseBlue = BLUE_ADD;
  baseRedAdd = RED_ADD;
  baseGreenAdd = GREEN_ADD;
  baseBlueAdd = BLUE_ADD;
  sentEffectStopSignal = false;
  xpos = 80;
  ypos = height / 2;
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
  if (effectNumber == 0) {
    return;
  }
  
  if (effectNumber == 1) {
    if (canPlaySE) {
      playSE();
      canPlaySE = false;
    }
    fill(0, 0, 0, 20);
    rect(0, 0, width, height);
    
    xpos += 30;
    colorOutCheck();
    
    for (int pid = 0; pid < MAX_PARTICLE; pid++) {
      particles[pid].move(xpos, ypos);
    }
    applyAdditiveSynthesis();
  } else if (effectNumber == 2) {
    if (canPlaySE) {
      playSE();
      canPlaySE = false;
    }
    fill(0, 0, 0, 15);
    rect(0, 0, width, height);
    
    xpos += 55;
    baseRed += baseRedAdd;
    baseGreen += baseGreenAdd;
    baseBlue += baseBlueAdd;
    colorOutCheck();
    
    for (int pid = 0; pid < MAX_PARTICLE; pid++) {
      particles[pid].move(xpos, ypos);
      particles[pid].explode();
    }
    applyAdditiveSynthesis();
  } else if (effectNumber == 3) {
    if (canPlaySE) {
      playSE();
      canPlaySE = false;
    }
    if (sentEffectStopSignal) {
      fill(0, 0, 0, 20);
    } else {
      fill(0, 0, 0, 10);
    }
    rect(0, 0, width, height);
   
    int deadCounter = 0;
    loadPixels();
    for(int j = 0; j < MAX_LINE;j++){
      if(neonLine[j].live) {
        for(int i = 0; i < DRAW_TIMES; i++){
          neonLine[j].draw(pixels);
        }
      } else {
        deadCounter++;
      }
    }
    updatePixels();
    
    if(deadCounter >= BORDER_DEAD_COUNT && !neonEffectStarted){
      neonEffectStarted = true;
      for(int i = 0; i < MAX_LINE; i++){
        if(!neonLine[i].live){
          float x = 0;
          float y = (height / 2) + random(20);
          neonLine[i].revival(x, y);
        }
      }
    }
  } else if (effectNumber == 4) {
    if (canPlaySE) {
      playSE();
      canPlaySE = false;
    }
    if (sentEffectStopSignal) {
      fill(0, 0, 0, 20);
    } else {
      fill(0, 0, 0, 10);
    }
    rect(0, 0, width, height);
   
    int deadCounter = 0;
    loadPixels();
    for(int j = 0; j < MAX_LINE;j++){
      if(neonLine[j].live) {
        for(int i = 0; i < DRAW_TIMES; i++){
          neonLine[j].draw(pixels);
        }
      } else {
        deadCounter++;
      }
    }
    updatePixels();
    
    if(deadCounter >= BORDER_DEAD_COUNT && !neonEffectStarted){
      neonEffectStarted = true;
      for(int i = 0; i < MAX_LINE; i++){
        if(!neonLine[i].live){
          float x = 0;
          float y = (height / 2) + random(20);
          neonLine[i].revival(x, y);
        }
      }
    }
  }
  
  if (!sentEffectStopSignal && (particleWindowOutCheck() || neonWindowOutCheck())) {
    if (connectsArduino()) {
      port.write(EFFECT_STOP_SIGNAL);
    }
    sentEffectStopSignal = true;
    fx_damage.trigger();
  }
}

void playSE() {
  switch(effectNumber)
  {
    case 1:
      fx1.trigger();
      break;
    case 2:
      fx2.trigger();
      break;
    case 3:
      fx1.trigger();
      break;
    case 4:
      fx1.trigger();
      break;
    default:
      break;
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

void applyAdditiveSynthesis() {
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

boolean particleWindowOutCheck() {
  for (int i = 0; i < MAX_PARTICLE; i++) {
    if (particles[i].isOffScreen()) {
      return true;
    }
  }
  return false;
}

boolean neonWindowOutCheck() {
  if (!neonEffectStarted || effectNumber == 1 || effectNumber == 2) {
    return false;
  }
  for (int i = 0; i < MAX_LINE; i++) {
    if (!neonLine[i].live) {
      return true;
    }
  }
  return false;
}

void serialEvent(Serial serialPort) {
  String inBuffer = serialPort.readStringUntil('\n');
  if (inBuffer != null) {
    String[] inBuffers = inBuffer.split("[,\\n]", 0);
    readCommand = inBuffers[0];
    readValue = Integer.parseInt(inBuffers[1]);
    println("command:" + readCommand);
    println("read:" + readValue);
  }
  
  initializeDisplay();
  effectNumber = decideEffectNumberFrom(readCommand, readValue);
}

int decideEffectNumberFrom(String readCommand, int readValue) {
  if (readValue < 0 || readValue > MAX_READ_VALUE) {
    return ERROR_READ_VALUE;
  }
  
  if (readCommand.equals("s")) {
    canPlaySE = true;
    if (readValue <= (MAX_READ_VALUE / 2)) {
      return 1;
    } else {
      return 2;
    }
  } else if (readCommand.equals("d")) {
    canPlaySE = true;
    if (readValue <= (MAX_READ_VALUE / 2)) {
      neonEffectStarted = false;
      neonStrength = NEON_WEAK;
      return 3;
    } else {
      neonEffectStarted = false;
      neonStrength = NEON_STRONG;
      return 4;
    }
  } else {
    return ERROR_READ_COMMAND;
  }
}

void mousePressed() {
  initializeDisplay();
  canPlaySE = true;
  effectNumber = 1;
}

void keyPressed() {
  if (key == '1') {
    initializeDisplay();
    canPlaySE = true;
    effectNumber = 1;
  } else if (key == '2') {
    initializeDisplay();
    canPlaySE = true;
    effectNumber = 2;
  }  else if (key == '3') {
    initializeDisplay();
    canPlaySE = true;
    neonEffectStarted = false;
    neonStrength = NEON_WEAK;
    effectNumber = 3;
  } else if (key == '4') {
    initializeDisplay();
    canPlaySE = true;
    neonEffectStarted = false;
    neonStrength = NEON_STRONG;
    effectNumber = 4;
  }
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

class NeonLine {
  float x;
  float y;
  float speed;
  float direction;
  float addDirection;
  float accelDirection;
  float addlDirectionRange;
  float stateCounter;
  color col;
  boolean live = false;
 
  final int BORDER_STATE_COUNTER = 150;
 
  NeonLine(){
    switch((int)random(6)){
    case 0:
      col = color(255, 255, 128, 25);
      break;
    case 1:
      col = color(255, 128, 255, 25);
      break;
    case 2:
      col = color(128, 255, 255, 25);
      break;
    case 3:
      col = color(255, 128, 128, 25);
      break;
    case 4:
      col = color(128, 255, 128, 25);
      break;
    case 5:
      col = color(128, 128, 255, 25);
      break;
    }
  }
 
  void revival(float _x, float _y){
    x = _x;
    y = _y;
    speed = 2;
    direction = degrees(atan2(halfHeight - y, halfWidth - x));
    direction = random(50) - 25.0;
    if (direction >= 0) {
      addDirection = -0.03;
    } else {
      addDirection = 0.03;
    }
    
    accelDirection = 0;
    addlDirectionRange = random(6);
    stateCounter = -random(200);
    live = true;
  }
 
  void draw(int[] pixels){
    if(live){
      x += cos(radians(direction)) * speed;
      y += sin(radians(direction)) * speed;
      direction += addDirection;
      //addDirection += accelDirection;
      
      stateCounter++;
      if(false) {
        stateCounter = 0;
        addDirection = random(addlDirectionRange) - addlDirectionRange / 2;
      }
      
      boolean deadFlg = false;    
      if(abs(x - width) >= width) {
        live = false;
        return;
      }
      if(abs(y - height) >= height) {
        live = false;
        return ;
      }
      
      for(int i = 1; i < neonStrength; i++){
        neonDia(pixels, i);
      }
    }
  }
  
  private void neonDia(int[] px, int size){
    float size2 = 8 * size * size / 2;
 
    for(int j = max(0, (int)(y - size2)); j < min(height - 1, (int)(y + size2)); j++){
      float wide = size2 - abs(j - y);
      for(int i = max(0, (int)(x - wide)); i < min(width - 1, (int)(x + wide)); i++){
        int id = j * width + i;
        px[id] = col;
      }
    }
  }
}
