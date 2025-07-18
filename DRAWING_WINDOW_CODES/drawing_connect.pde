import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import processing.net.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class drawing_connect extends PApplet {

enum MovementMode
{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT, 
  SPEED
};

class Movement
{
  public int distance;
  public MovementMode mode;
  
  public String toString()
  {
    String result = "X";
    switch(mode)
    {
      case FORWARD:
        result = "F";
        break;
      case BACKWARD:
        result = "B";
        break;
      case LEFT:
        result = "L";
        break;
      case RIGHT:
        result = "R";
        break;
      case SPEED:
        result = "S";
        break;
    }
    result = result + str(distance);
    return result;
  }
};

static int SIZE = 100;

int nr = -1;
int nrstart = -1;
int x[], y[];
String dir[];
float ang[], len[];
float distanceMultiplier = 6;
float angleMultiplier = 11.1f;

PImage logo;

int appel = color(68,192,158);
int appellight = color(68,192,158,200);
int gray = color(46,46,46);
int graylight = color(130,130,130);

boolean connected = false;
boolean done = false;
boolean start = false;
boolean rotating = false;
boolean sliding = false;

int prevMillis = 0;
int currentPos = 0;
int currentMovement = 0;
int destinationMovement = 0;

int sliderPos = 0;

 

Client c;

public void sendMovement(Movement mov)
{
  c.write("1:"+mov.toString()+"\r\n");
};

public void setup() 
{  
  background(gray);
  
  
  
  textAlign(CENTER, CENTER);
  
  x = new int[SIZE];
  y = new int[SIZE];
  ang = new float[SIZE];
  len = new float[SIZE];
  dir = new String[SIZE];
  
  logo = loadImage("D:/FINAL YEAR PROJECT/remote-controlling program/drawing_connect/data/LOGO.png");
  
  c = null;
}

public void draw() 
{  
  // Create blank screen
  fill(gray);
  noStroke();
  rect(0,0,width,height);
  // Logo
  image(logo, 0, 0, 410, 156);
  if (start == false) Draw_drawing();
  else Draw_started();
}

public void Draw_drawing()
{
  // Draw circles
  textSize(35);
  fill(255);
  noStroke();
  for (int i = 0; i <= nr; ++i)
    ellipse(x[i], y[i], 10, 10);
  // Draw lines
  stroke(255);
  for (int i = 1; i <= nr; ++i)
    line(x[i-1], y[i-1], x[i], y[i]);
  // Draw current line + angle + length
  if (nr > 0 && nr < SIZE-1 && done == false)
  {
    String a = "L";
    line(x[nr], y[nr], mouseX, mouseY);
    PVector prev = new PVector(x[nr-1]-x[nr], y[nr]-y[nr-1]);
    PVector current = new PVector(x[nr]-mouseX, mouseY-y[nr]);
    current.set(x[nr]-mouseX, mouseY-y[nr], 0);
    float angle = angle(prev, current);
    angle = degrees(angle);
    if(angle > 180) 
    {
      a = "R";
      angle = 181 - (angle - 180);
    }
    fill(255);
    text(PApplet.parseInt(angle)+"\u00b0"+" "+a, width/2, 10);
    ang[nr] = angle;
    dir[nr] = a;
    float d = sqrt(pow(x[nr]-mouseX,2)+pow(mouseY-y[nr],2));
    text(d+" m", width/2, 40);
    len[nr] = d;
  }
  else if (nr == 0 && nr < SIZE-1 && done == false)
  {
    String a = "L";
    line(x[nr], y[nr], mouseX, mouseY);
    PVector prev = new PVector(x[nr]-x[nr], y[nr]-(y[nr]+1));
    PVector current = new PVector(x[nr]-mouseX, mouseY-y[nr]);
    current.set(x[nr]-mouseX, mouseY-y[nr], 0);
    float angle = angle(prev, current);
    angle = degrees(angle);
    if(angle > 180) 
    {
      a = "R";
      angle = 181 - (angle - 180);
    }
    fill(255);
    text(PApplet.parseInt(angle)+"\u00b0"+" "+a, width/2, 10);
    ang[nr] = angle;
    dir[nr] = a;
    float d = sqrt(pow(x[nr]-mouseX,2)+pow(mouseY-y[nr],2));
    text(d+" m", width/2, 40);
    len[nr] = d;
  }
  // Draw start
  if (nr >= 0)
  {
    fill(255);
    text("start", x[0], y[0]-25);
  }
  // Button connect
  boolean connected = false;
  if (c != null)
  {
    if (c.active())
    {
      connected = true;
    }
  }
  if (!connected)
  {
    if ( overRect(width-400, height-40*6, 400, 40) ) {
      fill(appel);
    }
    else {
      fill(graylight);
    }
    stroke(gray);
    rect(width-400, height-40*6, 400, 40);
    fill(255);
    text("connect", width-200, height-40*5-23);
  }
  // Draw slider
  fill(graylight);
  stroke(graylight);
  strokeWeight(5);
  line(width-380, height-40*4-23, width-20, height-40*4-23);
  noStroke();
  strokeWeight(4);
  ellipse(width-380 + sliderPos, height-40*4-23, 20,20);
  // Button speed
  if ( overRect(width-400, height-40*4, 400, 40) ) {
    fill(appel);
  }
  else {
    fill(graylight);
  }
  stroke(gray);
  rect(width-400, height-40*4, 400, 40);
  fill(255);
  text("Speed " + (int)(sliderPos/3.6f) + "%", width-200, height-40*3-23);
  // Button clear
  if ( overRect(width-400, height-40*3, 400, 40) ) {
    fill(appel);
  }
  else {
    fill(graylight);
  }
  stroke(gray);
  rect(width-400, height-40*3, 400, 40);
  fill(255);
  text("clear", width-200, height-40*2-23);
  // Button done
  if ( overRect(width-400, height-40*2, 400, 40) ) {
    fill(appel);
  }
  else {
    fill(graylight);
  }
  stroke(gray);
  rect(width-400, height-40*2, 400, 40);
  fill(255);
  text("done", width-200, height-40*1-23);
  // Button start
  if ( overRect(width-400, height-40, 400, 40) ) {
    fill(appel);
  }
  else {
    fill(graylight);
  }
  stroke(gray);
  rect(width-400, height-40, 400, 40);
  fill(255);
  text("start", width-200, height-23);
  // Create list of movements
  fill(graylight);
  for (int i = 0; i <= nr-1; ++i, ++nrstart)
    text(ang[i]+"\u00b0"+" "+dir[i]+" "+len[i]+"mm", width-200, 75+i*37);
}

public void Draw_started()
{
  int ms = millis();
  int dTime = ms - prevMillis;
  prevMillis = ms;
  currentPos += dTime;
  // Rotate robot
  if (rotating)
  {
    if (currentPos > destinationMovement)
    {
      Movement mov = new Movement();
      mov.mode = MovementMode.FORWARD;
      mov.distance = PApplet.parseInt(len[currentMovement] * distanceMultiplier);
      sendMovement(mov);
      prevMillis = millis();
      destinationMovement = mov.distance;
      currentPos = 0;
      rotating = false;
    }
  }
  // Move robot
  else
  {
    if (currentPos > destinationMovement)
    {
      if (currentMovement == nr-1)
      {
        start = false;
        Draw_drawing();
        return;
      }
      else
      {
        ++currentMovement;
        Movement mov = new Movement();
        if (dir[currentMovement] == "R")
          mov.mode = MovementMode.RIGHT;
        else
          mov.mode = MovementMode.LEFT;
        mov.distance = PApplet.parseInt(ang[currentMovement] * angleMultiplier);
        sendMovement(mov);
        prevMillis = millis();
        destinationMovement = mov.distance;
        currentPos = 0;
        rotating = true;
      }
    }
  }
  // Draw lines
  stroke(255);
  for (int i = currentMovement+1; i <= nr; ++i)
    line(x[i-1], y[i-1], x[i], y[i]);
  stroke(appel);
  for (int i = 1; i <= currentMovement; ++i)
    line(x[i-1], y[i-1], x[i], y[i]);
  if (currentMovement < nr && !rotating)
  {
    int endOffsetX = (int)((((float)(x[currentMovement] - x[currentMovement+1]))/len[currentMovement])*(((float)(currentPos))/distanceMultiplier));
    int endOffsetY = (int)((((float)(y[currentMovement] - y[currentMovement+1]))/len[currentMovement])*(((float)(currentPos))/distanceMultiplier));
    line(x[currentMovement], y[currentMovement], x[currentMovement] - endOffsetX, y[currentMovement] - endOffsetY);  
  }
  // Draw circles
  fill(appel);
  noStroke();
  for (int i = 0; i <= currentMovement; ++i)
    ellipse(x[i], y[i], 10, 10);
  fill(255);
  for (int i = currentMovement+1; i <= nr; ++i)
    ellipse(x[i], y[i], 10, 10);
  // Text
  fill(appel);
  for (int i = 0; i <= nr-1; ++i, ++nrstart)
    text(ang[i]+"\u00b0"+" "+dir[i]+" "+len[i]+" m", width-100, 75+i*20);
}

public float angle(PVector v1, PVector v2) {
  float a = atan2(v2.y, v2.x) - atan2(v1.y, v1.x);
  if (a < 0) a += TWO_PI;
  return a;
}

public void mouseClicked() 
{
  if (start)
    return;
  if (overRect(width-400, height-240, 400, 240) == false && done == false) {
    ++nr;
    if(nr >= SIZE)
    {
      nr = -1;
      return;
    }
    x[nr] = mouseX;
    y[nr] = mouseY;
  }
  
  boolean connected = false;
  if (c != null)
  {
    if (c.active())
    {
      connected = true;
    }
  }
  if (!connected)
  {
    if (overRect(width-400, height-40*6, 400, 40)) {
      c = new Client(this, "192.168.4.1", 1234); 
    }
  }
  
  if (connected)
  {
    if (overRect(width-400, height-40*4, 400, 40)) {
      Movement mov = new Movement();
      mov.mode = MovementMode.SPEED;
      mov.distance = PApplet.parseInt(sliderPos/3.6f);
      sendMovement(mov);
    }
  }
  
  if (overRect(width-400, height-40*3, 400, 40)) {
    nr = -1;
    done = false;
  }
  
  if (overRect(width-400, height-40*2, 400, 40)) {
    done = true;
  }
  
  if (overRect(width-400, height-40*1, 400, 40)) {
    if (nr < 0)
      return;
    if (!connected)
      return;
    start = true;
    currentPos = 0;
    currentMovement = 0;
    Movement mov = new Movement();
    if (dir[currentMovement] == "R")
      mov.mode = MovementMode.RIGHT;
    else
      mov.mode = MovementMode.LEFT;
    mov.distance = PApplet.parseInt(ang[currentMovement] * angleMultiplier);
    sendMovement(mov);
    prevMillis = millis();
    destinationMovement = mov.distance;
    rotating = true;
  }
  
}

public void mouseDragged() {
  if (overCircle(width-380 + sliderPos, height-40*4-23, 70)) {
    sliderPos = mouseX - (width-380);
    if (sliderPos < 0)
      sliderPos = 0;
    else if (sliderPos > 360)
      sliderPos = 360; 
  }
  for (int i = 0; i <= nr; ++i)
    if (overCircle(x[i], y[i], 30)) {
      x[i] = mouseX;
      y[i] = mouseY;
      if (i > 1)
      {
        String a = "L";
        PVector prev = new PVector(x[i-2]-x[i-1], y[i-1]-y[i-2]);
        PVector current = new PVector(x[i-1]-x[i], y[i]-y[i-1]);
        float angle = angle(prev, current);
        angle = degrees(angle);
        if(angle > 180) 
        {
          a = "R";
          angle = 181 - (angle - 180);
        }
        ang[i-1] = angle;
        dir[i-1] = a;
        a = "L";
        prev = new PVector(x[i-1]-x[i], y[i]-y[i-1]);
        current = new PVector(x[i]-x[i+1], y[i+1]-y[i]);
        angle = angle(prev, current);
        angle = degrees(angle);
        if(angle > 180) 
        {
          a = "R";
          angle = 181 - (angle - 180);
        }
        ang[i] = angle;
        dir[i] = a;
        float b = sqrt(pow(x[i]-x[i-1],2)+pow(y[i-1]-y[i],2));
        len[i-1] = b;
        float c = sqrt(pow(x[i]-x[i+1],2)+pow(y[i+1]-y[i],2));
        len[i] = c;
      }
      if (i == 1)
      {
        String a = "L";
        PVector prev = new PVector(0, -1);
        PVector current = new PVector(x[i-1]-x[i], y[i]-y[i-1]);
        float angle = angle(prev, current);
        angle = degrees(angle);
        if(angle > 180) 
        {
          a = "R";
          angle = 181 - (angle - 180);
        }
        ang[i-1] = angle;
        dir[i-1] = a;
        a = "L";
        prev = new PVector(x[i-1]-x[i], y[i]-y[i-1]);
        current = new PVector(x[i]-x[i+1], y[i+1]-y[i]);
        angle = angle(prev, current);
        angle = degrees(angle);
        if(angle > 180) 
        {
          a = "R";
          angle = 181 - (angle - 180);
        }
        ang[i] = angle;
        dir[i] = a;
        float b = sqrt(pow(x[i]-x[i-1],2)+pow(y[i-1]-y[i],2));
        len[i-1] = b;
        float c = sqrt(pow(x[i]-x[i+1],2)+pow(y[i+1]-y[i],2));
        len[i] = c;
        
      }
      else if (i == 0)
      {
        String a = "L";
        PVector prev = new PVector(0, 1);
        PVector current = new PVector(x[i+1]-x[i], y[i]-y[i+1]);
        float angle = angle(prev, current);
        angle = degrees(angle);
        if(angle > 180) 
        {
          a = "R";
          angle = 181 - (angle - 180);
        }
        ang[i] = angle;
        dir[i] = a;
        float c = sqrt(pow(x[i]-x[i+1],2)+pow(y[i+1]-y[i],2));
        len[i] = c;
      }
    }
}                                    

public boolean overRect(int x, int y, int width, int height)  {
  if (mouseX >= x && mouseX <= x+width && 
      mouseY >= y && mouseY <= y+height) {
    return true;
  } else {
    return false;
  }
}

public boolean overCircle(int x, int y, int diameter) {
  float disX = x - mouseX;
  float disY = y - mouseY;
  if (sqrt(sq(disX) + sq(disY)) < diameter/2 ) {
    return true;
  } else {
    return false;
  }
}
  public void settings() {  fullScreen();  smooth(); }
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "drawing_connect" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
      }
  }
}
