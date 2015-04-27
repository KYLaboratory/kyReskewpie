import processing.serial.*; 

Serial port;
static final String SERIAL_PORT_NAME = "/dev/cu.usbmodemfd111";
static final int EFFECT_START_SIGNAL = 1;
static final int EFFECT_STOP_SIGNAL = 2;
static final int SPEED = 9600;
int readValue = 0;

static final int WINDOW_WIDTH = 640;
static final int WINDOW_HEIGHT = 360;
static final int BACKGROUND_COLOR = 102;
static final int FPS = 30;

static final int RAD = 60;        // Width of the shape
float xpos, ypos;    // Starting position of shape    
static final float X_STARTING_POSITION = 80;
float xspeed = 0;  // Speed of the shape
float yspeed = 0;  // Speed of the shape
static final float DEFAULT_X_SPEED = 8;
int xdirection = 1;  // Left or Right
int ydirection = 1;  // Top to Bottom

void setup() 
{
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  noStroke();
  frameRate(FPS);
  ellipseMode(RADIUS);
  // Set the starting position of the shape
  xpos = X_STARTING_POSITION;
  ypos = height / 2;
  
  port = new Serial(this, SERIAL_PORT_NAME, SPEED);
}

void draw() 
{
  background(BACKGROUND_COLOR);
  
  // Update the position of the shape
  xpos += ( xspeed * xdirection );
  ypos += ( yspeed * ydirection );
  
  // Test to see if the shape exceeds the boundaries of the screen
  // If it does, reverse its direction by multiplying by -1
  if (xpos > width - RAD || xpos < RAD) {
    //xdirection *= -1;
    readValue = 0;
    xspeed = 0;
    port.write(EFFECT_STOP_SIGNAL);
    xpos = X_STARTING_POSITION;
  }
  if (ypos > height - RAD || ypos < RAD) {
    ydirection *= -1;
  }

  // Draw the shape
  ellipse(xpos, ypos, RAD, RAD);
  
  if (readValue == EFFECT_START_SIGNAL) {
    xspeed = DEFAULT_X_SPEED;
  }
}

void serialEvent(Serial p){
  readValue = p.read();
  println("read:" + readValue);
}
