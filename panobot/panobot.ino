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

unsigned long v2_state;
unsigned long v1_state;
int menuNum = 10;
int menuId = 0;

int set_hdr = 0;

int hdr[] = {1, 2, 3, 5, 7, 9};
int hdrNum = sizeof(hdr) / sizeof(hdr[0]);
int set_row = 3;
int set_col = 6;
int set_low = 20;
int set_hi = 10;
float set_uds = 0;
float set_lrs = 0;
int set_move = 0;
Servo servoUD, servoLR; 


const char* ssid = "sushi";
const char* password = "12345687";

int wifiConnected = 0;

int devmode = 1; // connect wifi first

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
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    char tmp[8];
    display.clearDisplay();
    display.setCursor(0,0);
    sprintf(tmp, "%d\n", trial++);
    display.println(tmp);
    if (WiFi.status() == WL_NO_SHIELD)
      display.println("WL_NO_SHIELD");
    else if (WiFi.status() == WL_IDLE_STATUS)
      display.println("WL_IDLE_STATUS");
      else if (WiFi.status() == WL_NO_SSID_AVAIL)
      display.println("WL_NO_SSID_AVAIL");
      else if (WiFi.status() == WL_SCAN_COMPLETED)
      display.println("WL_SCAN_COMPLETED");
      else if (WiFi.status() == WL_CONNECT_FAILED)
      display.println("WL_CONNECT_FAILED");
      else if (WiFi.status() == WL_CONNECTION_LOST)
      display.println("WL_CONNECTION_LOST");
      else if (WiFi.status() == WL_DISCONNECTED)
      display.println("WL_DISCONNECTED");
    //display.write('.');
    display.display();
    //WiFi.begin(ssid, password);
    //ESP.restart();
  }
  
  //while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //  Serial.println("Connection Failed! Rebooting...");
  //  delay(1000);
  //  ESP.restart();
   
  //}
  
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
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("OTA\nConnected\n");
  display.println(WiFi.localIP());
  display.display();
  if (devMode == 0) {
    while (1) {
      ArduinoOTA.handle();
      delay(10);
    }
  }
}


void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  pinMode(D4, INPUT_PULLUP);
  display.clearDisplay();
  servoUD.attach(D6);
  servoLR.attach(D5);
  niceMessage("test");

//  uint8_t a = 123;
 // EEPROM.begin(512);
  //EEPROM.write(0, a);
 // niceMessage(EEPROM.read(0));
 // while(1) ;
  //ota();
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
  else 
    dig = 3;
    
  if (dig == 1)
    display.setCursor(24, 27);
  else if (dig == 2)
    display.setCursor(16, 27);
  else if (dig == 3)
    display.setCursor(8, 27);
  display.setTextSize(3);
  char tmp[8];
  sprintf(tmp, "%d", v);
  display.println(tmp);
}
void drawMenu() {
  char tmp[16];
  display.setTextColor(WHITE);
  display.clearDisplay();

  if (menuId == 0) {
    drawHeader("Home");
    display.setTextSize(2);
    display.setCursor(0, 34);
    display.println("Start");
    display.fillTriangle(28, 33, 34, 33, 31, 30, WHITE); 
  } else if (menuId == 1) {
    drawHeader("Move");
    drawNumber(set_move);
  } else if (menuId == 2) {
    drawHeader("HDR");
    drawNumber(hdr[set_hdr]);
  } else if (menuId == 3) {
    drawHeader("Row");
    drawNumber(set_row);
  } else if (menuId == 4) {
    drawHeader("Col");
    drawNumber(set_col);
  } else if (menuId == 5) {
    drawHeader("Low");
    drawNumber(set_low);
  } else if (menuId == 6) {
    drawHeader("Hi");
    drawNumber(set_hi);
  } else if (menuId == 7) {
    drawHeader("UD-S");
    drawNumber(set_uds);
  } else if (menuId == 8) {
    drawHeader("LR-S");
    drawNumber(set_lrs);
  } else if (menuId == 9) {
    drawHeader("OTA");
    display.setTextSize(2);
    display.setCursor(9, 34);
    display.println("Link");
    display.fillTriangle(28, 31, 34, 31, 31, 28, WHITE); 
  }
  display.display();
}

int treatValue(int data) {
  return (data * 9 / 1024) + 48;
}



int waiting(int mil) {
  unsigned long wait = millis();
  while (millis() - wait < mil) {
    int v1 = analogRead(A0);
    int v2 = digitalRead(D4);
    if (v1 < 300 || v1 > 700 || v2 == 0) {
      while (v1 < 300 || v1 > 700 || v2 == 0) {
        v1 = analogRead(A0);
        v2 = digitalRead(D4);
        delay(10);
      }
      while (true) {
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

void releaseShutter() {
  delay(10);
}

void setServos() {
  char tmp[8];
  // set_uds = -86 is fully down
  int value = map(set_uds + 90 + 4, 0, 180, 870, 2130);
  servoUD.writeMicroseconds(value);

  value = map(set_lrs, 0, 390, 870, 2130);
  servoLR.writeMicroseconds(value);
  
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
      waiting(500);
      for (int k = 0; k < hdr[set_hdr]; k++) {
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
        
        releaseShutter();
        count ++;
        
        int ret = waiting(1000);
        if (ret == -1)
          return;
      
      }
    }
  }
}
void loop() {
  char tmp[16];
  
  int v1 = analogRead(A0);
  int v2 = digitalRead(D4);
  int up = 0, down = 0;

  if (v1_state == 0) {
    if (v1 < 300) { 
      up = 1;
      v1_state = millis();
    } else if (v1 > 700) {
      down = 1;
      v1_state = millis();
    }
  } else if (v1 > 400 && v1 < 600) {
    v1_state = 0;
  } else if (millis() - v1_state > 800) {
    if (v1 < 300) { 
      up = 1;
      v1_state = millis() - 780;
    } else if (v1 > 700) {
      down = 1;
      v1_state = millis() - 780;
    }
  }

  if (v2 == 0) { // move right
    if (v2_state == 0) {
      menuId = (menuId + 1) % menuNum;
      v2_state = millis();
    }
    if (millis() - v2_state > 400) {
      menuId = 0;
    }
  } else {
    v2_state = 0;
  }

  
  if (menuId == 0) { // Capture
    if (up) {
      while (v1 < 400) {
        v1 = analogRead(A0);
        delay(10);
      }
      capturing();
    }
  } else if (menuId == 1) { // Move
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
  } else if (menuId == 2) { // HDR
    if (down && set_hdr > 0) {
      set_hdr--;
    } else if (up && set_hdr < hdrNum-1) {
      set_hdr++;
    }
  } else if (menuId == 3) { // Row
    if (down && set_row > 1)
      set_row--;
    else if (up)
      set_row++;
  } else if (menuId == 4) { // Col
    if (down && set_col > 1)
      set_col--;
    else if (up)
      set_col++;
  } else if (menuId == 5) { // Low
    if (down && set_low > 0)
      set_low-= 5;
    else if (up)
      set_low+= 5;
  } else if (menuId == 6) { // Hi
    if (down && set_hi > 0)
      set_hi-= 5;
    else if (up)
      set_hi+= 5;
  } else if (menuId == 7) { // uds
    if (down && set_uds > -90)
      set_uds--;
    else if (up && set_uds < 90)
      set_uds++;
     setServos();
  } else if (menuId == 8) { // lrs
    if (down && set_lrs > 0)
      set_lrs-= 5;
    else if (up && set_lrs < 360)
      set_lrs+= 5;
     setServos();
  } else if (menuId == 9) { // OTA Update
    if (up) {
      while (v1 < 400) {
        v1 = analogRead(A0);
        delay(10);
      }
      ota();
    }
  }
  //sprintf(tmp, "%d %d", v1, v2);
  //niceMessage(tmp);
  //delay(1);
  drawMenu();
}
/*
void loop() {
  // reads the value of the variable resistor 
  value1 = analogRead(joyPin1);   
  // this small pause is needed between reading
  // analog pins, otherwise we get the same value twice
  delay(100);             
  // reads the value of the variable resistor 
  value2 = analogRead(joyPin2);   
  
  digitalWrite(ledPin, HIGH);           
  delay(value1);
  digitalWrite(ledPin, LOW);
  delay(value2);
  Serial.print('J');
  Serial.print(treatValue(value1));
  Serial.println(treatValue(value2));
}*/
