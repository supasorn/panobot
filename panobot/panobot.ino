#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <pins_arduino.h>
#include <ESP8266WiFi.h> 
#include <Servo.h> 
#include <ArduinoOTA.h>
#include <Time.h>
#include <EEPROM.h>

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

unsigned long v_state[2];

int menuId = 0;
int menuOrder[] = {
  0, // Home
  //1, // Move
  2, // HDR
  3, // Row
  4, // Col
  //5, // LOW
  //6, // Hi
  7, // UD-S
  8, // LR-S
  9, // Sweep
//  10, 11, // debug lr servo
//  12, 13, // debug ud servo
  14, // sweep delay
  };
int menuNum = sizeof(menuOrder) / sizeof(menuOrder[0]);

int set_hdr = 0;

int hdr[] = {1, 2, 3, 5, 7, 9};
int hdrNum = sizeof(hdr) / sizeof(hdr[0]);

// Sigma
// int set_low = 20;
// int set_hi = 40;
// int set_row = 3;
// int set_col = 6;


// fisheye
int set_low = 50;
int set_hi = 75;
int set_row = 2;
int set_col = 6;
int set_sweep_delay = 1000;

float set_uds = 0;
float set_lrs = 0;
int set_move = 0;
Servo servoUD, servoLR; 

int debug_lr_servo = 800;
int debug_ud_servo = 800;

int pwmUD = -1, pwmLR = -1;

const char* ssid = "Chirashi";
const char* password = "bangkokthailand";

int holdCount = 0;
int wifiConnected = 0;

int devmode = 1; // connect wifi first
int useota = 0;
void ota() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("OTA\nConnecting");
  display.display();
    
  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  int trial = 0;
  /*
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    char tmp[8];
    display.clearDisplay();
    display.setCursor(0,0);
    sprintf(tmp, "%d\n", trial++);
    display.println(tmp);
    display.display();
    WiFi.begin(ssid, password);
    //ESP.restart();
  }*/
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(1000);
    ESP.restart(); 
  }
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("OTA\nConnected\n");
  display.println(WiFi.localIP());
  display.display();
  //while (1) {
  // ArduinoOTA.handle();
  // delay(10);
 // }
  /*if (devMode == 0) {
    
  }*/
}




void niceMessage(char *str) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.println(str);
  display.display();
}

void niceMessage(int a) {
  char tmp[16];
  sprintf(tmp, "%d", a);
  niceMessage(tmp);
}
void drawHeader(char *str) {
  int xoff = (64 - menuNum * 4) / 2;
  display.drawRect(0, 18, xoff-1, 2, WHITE);
  display.drawRect(menuNum * 4 + 1 + xoff, 18, 64, 2, WHITE);
  for (int i = 0; i < menuNum; i++) {
    if (i == menuId) {
      //display.drawRect(xoff + 1 + i * 4 - 1, 15, 4, 4, WHITE);
      display.drawRect(xoff + 1 + i * 4, 18, 2, 4, WHITE);
    } else {
      display.drawRect(xoff + 1 + i * 4, 18, 2, 2, WHITE);
    }
  }
  display.drawRect(0, 17, 64, 1, WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(str);

  if (useota && WiFi.status() == WL_CONNECTED)
    display.drawRect(62, 0, 2, 2, WHITE);
}

void drawNumber(int v) {
  int dig = 0;
  if (v <= -10)
    dig = 3;
  else if (v < 0)
    dig = 2;
  else if (v < 10)
    dig = 1;
  else if (v < 100)
    dig = 2;
  else if (v < 1000)
    dig = 3;
  else if (v < 10000)
    dig = 4;

  display.setTextSize(3);
  if (dig == 1) {
    display.setCursor(24, 27);
  } else if (dig == 2) {
    display.setCursor(16, 27);
  } else if (dig == 3) {
    display.setCursor(8, 27);
  } else if (dig == 4) {
    display.setTextSize(2);
    display.setCursor(8, 27);
  }
  
  char tmp[8];
  sprintf(tmp, "%d", v);
  display.println(tmp);
}
void drawMenu(int debug=-1) {
  char tmp[16];
  display.setTextColor(WHITE);
  display.clearDisplay();

  int menu = menuOrder[menuId];
  if (menu == 0) {
    drawHeader("Home");
    display.setTextSize(2);
    display.setCursor(0, 34);
    display.println("Start");
    display.fillTriangle(28, 33, 34, 33, 31, 30, WHITE); 
  } else if (menu == 1) {
    drawHeader("Move");
    drawNumber(set_move);
  } else if (menu == 2) {
    drawHeader("HDR");
    drawNumber(hdr[set_hdr]);
  } else if (menu == 3) {
    drawHeader("Row");
    drawNumber(set_row);
  } else if (menu == 4) {
    drawHeader("Col");
    drawNumber(set_col);
  } else if (menu == 5) {
    drawHeader("Low");
    drawNumber(set_low);
  } else if (menu == 6) {
    drawHeader("Hi");
    drawNumber(set_hi);
  } else if (menu == 7) {
    drawHeader("UD-S");
    drawNumber(set_uds);
  } else if (menu == 8) {
    drawHeader("LR-S");
    drawNumber(set_lrs);
  } else if (menu == 9) {
    drawHeader("Sweep");
    display.setCursor(0, 34);
    display.println("Start");
    display.fillTriangle(28, 33, 34, 33, 31, 30, WHITE); 
  } else if (menu == 10 || menu == 11) {
    drawHeader("LR-D");
    drawNumber(debug_lr_servo);
  } else if (menu == 12 || menu == 13) {
    drawHeader("UD-D");
    drawNumber(debug_ud_servo);
  } else if (menu == 14) {
    drawHeader("Delay");
    drawNumber(set_sweep_delay);
  }

  if (debug != -1) {
    display.setTextSize(1);
    display.setCursor(0, 27);
     char tmps[16];
    sprintf(tmps, "%d", debug);
    display.println(tmps);
  }
  display.display();
}

void waitTillNeutral() {
  int v = analogRead(A0);
  while (v < 400 || v > 600) {
    delay(10);
    v = analogRead(A0);
  }
}

int waiting(int mil) {
  unsigned long wait = millis();
  while (millis() - wait < mil) {
    int v1 = analogRead(A0);
  
    if (v1 < 300 || v1 > 700) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(10, 0);
      display.println("Stop");
      display.setCursor(10, 30);
      display.println("Resm");
      display.fillTriangle(28, 23, 34, 23, 31, 20, WHITE); 
//        display.fillRect(28, 23, 7, 1, WHITE);
      display.fillTriangle(28, 25, 34, 25, 31, 28, WHITE); 
      display.display();
        
      while (v1 < 400 || v1 > 600) {
        v1 = analogRead(A0);
        delay(10);
      }
      while (true) {
        delay(10);
        v1 = analogRead(A0);
        if (v1 < 300) { // Stop
          while (v1 < 400) {
            v1 = analogRead(A0);
            delay(10);
          }
          return -1;
        } else if (v1 > 700) { // Resume
          while (v1 > 600) {
            v1 = analogRead(A0);
            delay(10);
          }
          return 1;
        }
      }
    }
    delay(10);
  }
  return 0;
}

void releaseShutter(int t) {
  //digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  delay(t);
  //digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
}

int getPWMLR(int lr) {
  return map(lr, 0, 360, 880, 2050);
}

int getPWMUD(int ud) {
  //return map(ud + 90, 0, 180, 870, 2135);
  return map(ud + 90, 2, 180, 870, 2100);
}

void setServosInstant() {
  pwmUD = getPWMUD(set_uds);
  pwmLR = getPWMLR(set_lrs);
  servoUD.writeMicroseconds(pwmUD);
  servoLR.writeMicroseconds(pwmLR);
}

// base delay for UD, LR
void setServos(int baseUD = 0, int baseLR = 0, int enableUD = 1, int enableLR = 1) {
  char tmp[8];
  int newPwmUD = getPWMUD(set_uds);
  int newPwmLR = getPWMLR(set_lrs);
  
  if (pwmUD == -1) pwmUD = 870;
  if (pwmLR == -1) pwmLR = 870;
  int n;
  n = fabs(newPwmUD - pwmUD) + 1;
  if (enableUD) {
    for (int i = 0; i < n; i++) {
      float t;
      if (n == 1) 
        t = 1;
      else
        t = 1.0 * i / (n - 1);
      float sc = (cos(t * 2 * 3.141592653) + 1)/2 * 3 + 1;
      servoUD.writeMicroseconds(round(t * newPwmUD + (1-t) * pwmUD));
     
      delay(3 + baseUD); // so that watchdog is fed. https://github.com/esp8266/Arduino/issues/2240
      delayMicroseconds(1000 * sc);
    }
    pwmUD = newPwmUD;
  }
    
  n = fabs(newPwmLR - pwmLR) + 1;
  if (enableLR) {
    for (int i = 0; i < n; i++) {
      float t = 1.0 * i / (n - 1);
      if (n == 1) 
        t = 1;
      else
        t = 1.0 * i / (n - 1);
      float sc = (cos(t * 2 * 3.141592653) + 1)/2 * 3 + 1;
      servoLR.writeMicroseconds(round(t * newPwmLR + (1-t) * pwmLR));
      //delayMicroseconds(round(1000) * sc);
      delay(9 + baseLR);
      delayMicroseconds(1000 * sc);
    }
    pwmLR = newPwmLR;
  }
  
  
  //servoUD.writeMicroseconds(pwmUD);
  //servoLR.writeMicroseconds(pwmLR);
}
void capturing() {
  
  char tmp[8];
  int n = set_row * set_col * hdr[set_hdr];
  int down = -90 + set_low;
  int up = 90 - set_hi;
  int diff = up - down;
  int delta1 = diff / set_row;

  int delta2 = 360 / set_col;

  int count = 0;
  for (int i = 0; i < set_col; i++) {
    for (int j = 0; j < set_row; j++) {
      int y;
      if (i % 2) {
        y = set_row - 1 - j;
      } else {
        y = j;
      }
      set_lrs = i * 360.0 / set_col;
      set_uds = down + (up - down) * 1.0 * y / (set_row - 1);

      setServos();
      if ((hdr[set_hdr] == 1 && waiting(500) == -1)
       || (hdr[set_hdr] > 1 && waiting(2000) == -1))
          return;
      //for (int k = 0; k < hdr[set_hdr]; k++) {
      count += hdr[set_hdr];
      if (hdr[set_hdr] == 1)
        releaseShutter(10);
      else
        releaseShutter(2000);
        
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(2, 20);
      sprintf(tmp, "LR: %+3d", round(set_lrs));
      display.println(tmp);
      display.setCursor(2, 28);
      sprintf(tmp, "UD: %+3d", round(set_uds));
      display.println(tmp);

      display.setCursor(2, 36);
      sprintf(tmp, "N : %d/%d", count, n);
      display.println(tmp);

      display.setCursor(16, 0);
      display.setTextSize(2);
      sprintf(tmp, "%2d%%", count * 100 / n);
      display.println(tmp);
      display.display();

     // }
      if (waiting(200) == -1)
          return;
    }
  }
  set_lrs = 0;
  set_uds = -90;
  setServosInstant();
}

void sweeping() {
  char tmp[8];

  set_lrs = 0;
  set_uds = 0;
  setServos(0, 0, 0);
  delay(2000);

  int p0 = getPWMLR(0);
  int p1 = getPWMLR(360);
  int n = p1 - p0;

  int deg = 5;

  for (int t = 0; t < 360; t+=deg) {
    set_lrs = t;
    setServos(30, 30, 0);
    //setServos();
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(2, 20);
    sprintf(tmp, "LR: %+3d", round(set_lrs));
    display.println(tmp);
    display.setCursor(2, 28);
    sprintf(tmp, "UD: %+3d", round(set_uds));
    display.println(tmp);

    //display.setCursor(2, 36);
    //sprintf(tmp, "N : %d/%d", count, n);
    //display.println(tmp);

    display.setCursor(16, 0);
    display.setTextSize(2);
    sprintf(tmp, "%2d%%", t * 100 / 360);
    display.println(tmp);
    display.display();

    if (waiting(set_sweep_delay) == -1)
      return;
      
    releaseShutter(100);
      
  }
}

void loop() {
  if (useota)
    ArduinoOTA.handle();
  
  char tmp[16];

  digitalWrite(D5, HIGH);
  delay(10);
  int v[2];
  
  v[0] = analogRead(A0);
  digitalWrite(D5, LOW);
  delay(10);
  v[1] = analogRead(A0);

  if (abs(v[0] - 500) > abs(v[1] - 500))
    v[1] = 500;
  else 
    v[0] = 500;
  char but[2][2] = {0};
  
  for (int i = 0; i < 2; i++) {
    if (v_state[i] == 0) {
      if (v[i] < 100) { 
        but[i][0] = 1;
        v_state[i] = millis();
      } else if (v[i] > 900) {
        but[i][1] = 1;
        v_state[i] = millis();
      }
    } else if (v[i] > 200 && v[i] < 800) {
      v_state[i] = 0;
      holdCount = 0;
    } else if (i == 1 && millis() - v_state[i] > 800) {
      if (v[i] < 100) { 
        but[i][0] = 1;
        v_state[i] = millis() - 790;
      } else if (v[i] > 900) {
        but[i][1] = 1;
        v_state[i] = millis() - 790;
      }
      holdCount++;
    }
  }
  char up = but[1][0], down = but[1][1], left = but[0][1], right = but[0][0];  

  if (right) {
    menuId = (menuId + 1) % menuNum;
  } else if (left) {
    menuId = (menuNum + menuId - 1) % menuNum;
  }

  int menu = menuOrder[menuId];
  if (menu == 0) { // Capture
    if (up) {
      waitTillNeutral();
      capturing();
    }
  } else if (menu == 1) { // Move
    int change = 0;
     if (down && set_move > 0) {
      set_move--;
      change = 1;
    } else if (up && set_move < 3) {
      set_move++;
      change = 1;
    }
    if (change) {
      if (set_move == 0) 
        set_uds = -90;
      else if (set_move == 1) 
        set_uds = -90 + set_low;
      else if (set_move == 2) 
        set_uds = 0;
      else if (set_move == 3)
        set_uds = 90 - set_hi;
      setServos();
    }
  } else if (menu == 2) { // HDR
    if (down && set_hdr > 0) {
      set_hdr--;
    } else if (up && set_hdr < hdrNum-1) {
      set_hdr++;
    }
  } else if (menu == 3) { // Row
    if (down && set_row > 1)
      set_row--;
    else if (up)
      set_row++;
  } else if (menu == 4) { // Col
    if (down && set_col > 1)
      set_col--;
    else if (up)
      set_col++;
  } else if (menu == 5) { // Low
    if (down && set_low > 0)
      set_low-= 5;
    else if (up)
      set_low+= 5;
  } else if (menu == 6) { // Hi
    if (down && set_hi > 0)
      set_hi-= 5;
    else if (up)
      set_hi+= 5;
  } else if (menu == 7) { // uds
    int change = 0;
    int cstep = 1;

    if (holdCount > 18)
      cstep = 10;
    else if (holdCount > 12)
      cstep = 5;
    else if (holdCount > 6)
      cstep = 2;
    
    if (down && set_uds > -100) {
      set_uds -= cstep;
      change = 1;
    } else if (up && set_uds < 100) {
      set_uds += cstep;
      change = 1;
    }
    if (change)
     setServos();
  } else if (menu == 8) { // lrs
    int change = 0;
    int cstep = 5;
    
    if (holdCount > 16)
      cstep = 20;
    else if (holdCount > 8)
      cstep = 10;
      
    if (down && set_lrs > -10) {
      set_lrs-= cstep;
      change = 1;
    } else if (up && set_lrs < 400) {
      set_lrs+= cstep;
      change = 1;
    }
    if (change)
     setServos();
  } else if (menu == 9) { // Sweep
    if (up) {
      waitTillNeutral();
      sweeping();
    }
  } else if (menu == 10) { // debug lr servo
    int change = 0;
    if (down) {
      debug_lr_servo -=10;
      change = 1;
    } else if (up) {
      debug_lr_servo +=10;
      change = 1;
    }
    if (change)
      servoLR.writeMicroseconds(debug_lr_servo);
  } else if (menu == 11) { // debug lr servo
    int change = 0;
    if (down) {
      debug_lr_servo --;
      change = 1;
    } else if (up) {
      debug_lr_servo ++;
      change = 1;
    }
    if (change)
      servoLR.writeMicroseconds(debug_lr_servo);
  } else if (menu == 12) { // debug ud servo
    int change = 0;
    if (down) {
      debug_ud_servo -=10;
      change = 1;
    } else if (up) {
      debug_ud_servo +=10;
      change = 1;
    }
    if (change)
      servoUD.writeMicroseconds(debug_ud_servo);
  } else if (menu == 13) { // debug ud servo
    int change = 0;
    if (down) {
      debug_ud_servo --;
      change = 1;
    } else if (up) {
      debug_ud_servo ++;
      change = 1;
    }
    if (change)
      servoUD.writeMicroseconds(debug_ud_servo);
  } else if (menu == 14) { // set sweep delay
    if (down) {
      set_sweep_delay -= 100;
    } else if (up) {
      set_sweep_delay += 100;
    }
  }
  //sprintf(tmp, "%d %d", v1, v2);
  //niceMessage(tmp);
  //delay(1);

      
  //drawMenu(v[0]);
  drawMenu();
}


void setup() {
  Serial.begin(9600);  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.clearDisplay();
  niceMessage("Starting");
  //delay(1000);
  pinMode(D4, INPUT_PULLUP);
  if (analogRead(A0) < 300) {
    useota = 1;
    ota();
  }
  
  servoUD.attach(D7);
  servoLR.attach(D8);

  pwmUD = getPWMUD(set_uds);
  pwmLR = getPWMLR(set_lrs);
  setServos();

  
  //setServosInstant();

  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
//  uint8_t a = 123;
 // EEPROM.begin(512);
  //EEPROM.write(0, a);
 // niceMessage(EEPROM.read(0));
 // while(1) ;
  
}

