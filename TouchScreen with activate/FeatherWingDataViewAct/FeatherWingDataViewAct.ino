/*
 * This sketch will accept input from 
 * a touchscreen and use that input to 
 * retreive information from a sever
 * on values relating to Air Humidity,
 * Soil Humidity, Lux and Air Temperature.
 * 
 * This sketch is designed for use 
 * with an Adafruit Feather HUZZAH ESP8266
 * 
 * Created by Stephaine Istvan and Samuel Sheedy
 */
// tft and touchscreen libraries
#include <SPI.h>
#include <Adafruit_GFX.h>    
#include <Adafruit_HX8357.h>  
#include <Adafruit_STMPE610.h>
#include <SD.h>


//wifi library
#include <ESP8266WiFi.h>
//http client library
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
//Create a server object that will listen on port 80
WiFiServer server(80);
//Set a static IP address
IPAddress ip(192,168,0,60);
IPAddress subnet(255,255,255,0);
IPAddress gateway(192,168,0,1);

//these are the pins that are used by the touchscreen, tft and sd card
#define STMPE_CS 16
#define TFT_CS   0
#define TFT_DC   15
#define SD_CS    2
#define statusLED 16

#define TFT_RST -1

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3
int oldcolor, currentcolor;

#define BUFFPIXEL 20

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

int pageFlag, menuFlag;
int temp, airHumi, soilHumi, lux;
long mils, prevMils;

void setup(){
  //Define pins
  pinMode(statusLED,OUTPUT);
  
  Serial.begin(115200);

  //start the touchscreen and test if the driver is functioning
  if (!ts.begin()){
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");

  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
  }
  Serial.println("OK!");

  //tft.begin(HX8357D);
  tft.begin();
  tft.fillScreen(HX8357_BLACK);

  //Connect to wifi with the SSID and Password
  WiFi.config(ip,gateway, subnet);
  WiFi.begin("xxxx","xxxx");
  while ((!(WiFi.status() == WL_CONNECTED)))
  {
    delay(200);
    Serial.println("connecting");
  }

  bmpDraw("/logo.bmp", 0, 0);
  tft.setRotation(1);
  
}

void loop() 
{
  if(pageFlag == 9)
  {
    pageFlag = 0;
    blankScreen();
  }
  else if(ts.touched()){
    pageFlag=1;
  }
  else{
    pageFlag=0;    
  }
  if (pageFlag == 1){
      mainMenu();
  } 

  //Serial.println(WiFi.localIP()); //Debug - To find out ip of device
  //check if the device is still connected to wifi, connect again if it is not
  connectToWifi();
}

void mainMenu(){
  tft.fillScreen(0xA01F);
  tft.fillRect(0, 0, 480, 80, 0x5B0C);
  tft.fillRect(0, 240, 480, 80, 0x5B0C);
  tft.drawLine(240, 80, 240, 260, 0x5B0C);
  tft.drawLine(240, 240, 0, 480, 0x0000);
  tft.drawLine(240, 240, 480, 480, 0x0000);
  tft.fillTriangle(240,240,0,480,240,480,0xFFFF);
  tft.fillTriangle(240,240,240,480,480,480,0x9CD3);
  
  tft.setCursor(40, 30);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(4);
  tft.print("Select Menu Below");

  tft.setCursor(70, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("View Data");
  
  tft.setCursor(305, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Activate");

  delay(500);
  mils = millis();
  while(!ts.touched())
  {
    delay(100);
    if(millis() >= mils + 30000)
    {
      pageFlag = 9;
      break;
    }
  }

  while(pageFlag == 1)
  {
    if (ts.touched())
    {
      Serial.println("touching");
      TS_Point p; 
      while ( ! ts.bufferEmpty() )
      { 
        p = ts.getPoint();
      }
  
      p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(), 0);
      p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);
      Serial.print("X = "); Serial.print(p.x); Serial.print("\tY = "); 
      Serial.print(p.y);  Serial.print("\tPressure = "); Serial.println(p.z); 
    
      if(p.y >= 240 && p.y <= 480 && p.x >= 80 && p.x <= 240)
      {
        pageFlag = 6;
      }
      else if(p.y >= 0 && p.y <= 240 && p.x >= 80 && p.x <= 240)
      {
        pageFlag = 7;
      }
      else
      {
        pageFlag = 9;
        break;
      }
    }
    if(pageFlag == 6)
    {
      dataMenu();
    }
    else if(pageFlag == 7)
    {
      actMenu();
    }
  }
  
}

void dataMenu(){
  int line = 0x0000;
  tft.fillScreen(0xA01F);
  tft.fillRect(0, 0, 480, 80, 0x5B0C);
  tft.fillRect(0, 240, 480, 80, 0x5B0C);
  for(int i = 0; i <= 480; i = i + 120)
  {
    tft.drawLine(i, 80, i, 240, 0x0000);
    tft.drawLine(i, 240, 240, 480, 0x0000);
  }
  tft.fillTriangle(240,240,0,480,240,480,0xFFFF);
  tft.fillTriangle(240,240,240,480,480,480,0x9CD3);

  tft.setCursor(60, 30);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Select a piece of data to view");

  tft.setCursor(40, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Temp");

  tft.setCursor(165, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Air");

  tft.setCursor(275, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Soil");

  tft.setCursor(390, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Light");
  delay(500);
  mils = millis();
  while(!ts.touched())
  {
    delay(100);
    if(millis() >= mils + 30000)
    {
      pageFlag = 9;
      break;
    }
  }

  while(pageFlag == 6)
  {
    if (ts.touched())
    {
      Serial.println("touching");
      TS_Point p; 
      while ( ! ts.bufferEmpty() )
      { 
        p = ts.getPoint();
      }
  
      p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(), 0);
      p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);
      Serial.print("X = "); Serial.print(p.x); Serial.print("\tY = "); 
      Serial.print(p.y);  Serial.print("\tPressure = "); Serial.println(p.z); 
    
      if(p.y >= 360 && p.y <= 420 && p.x >= 80 && p.x <= 240)
      {
        pageFlag = 2;
      }
      else if(p.y >= 240 && p.y <= 360 && p.x >= 80 && p.x <= 240)
      {
        pageFlag = 3;  
      }
      else if(p.y >= 120 && p.y <= 240 && p.x >= 80 && p.x <= 240)
      {
        pageFlag = 4;
      }
      else if(p.y >= 0 && p.y <= 120 && p.x >= 80 && p.x <= 240)
      {
        pageFlag = 5;
      }
      else
      {
        break;
      }
    }
    if(pageFlag == 2)
    {
      tempMenu();
    }
    else if(pageFlag == 3)
    {
      airMenu();
    }
    else if(pageFlag == 4)
    {
      soilMenu();
    }
    else if(pageFlag == 5)
    {
      luxMenu(); 
    }
  }
}
void actMenu(){
  tft.fillScreen(0xA01F);
  tft.fillRect(0, 0, 480, 80, 0x5B0C);
  tft.fillRect(0, 240, 480, 80, 0x5B0C);
  tft.drawLine(160, 80, 160, 260, 0x5B0C);
  tft.drawLine(320, 80, 320, 260, 0x5B0C);

  tft.setCursor(70, 30);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(4);
  tft.print("Activate device");

  tft.setCursor(50, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Light");
  tft.setCursor(65, 170);
  tft.print("On");

  tft.setCursor(210, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Light");
  tft.setCursor(220, 170);
  tft.print("Off");

  tft.setCursor(370, 150);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(2);
  tft.print("Pump");
  tft.setCursor(380, 170);
  tft.print("On");

  delay(500);
  mils = millis();
  while(!ts.touched())
  {
    delay(100);
    if(millis() >= mils + 30000)
    {
      pageFlag = 9;
      break;
    }
  }

  while(pageFlag == 7)
  {
    if (ts.touched())
    {
      Serial.println("touching");
      TS_Point p; 
      while ( ! ts.bufferEmpty() )
      { 
        p = ts.getPoint();
      }
  
      p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(), 0);
      p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);
      Serial.print("X = "); Serial.print(p.x); Serial.print("\tY = "); 
      Serial.print(p.y);  Serial.print("\tPressure = "); Serial.println(p.z); 
    
      if(p.y >= 320 && p.y <= 480 && p.x >= 80 && p.x <= 240)
      {
        activateDevices("activateLights");
      }
      else if(p.y >= 160 && p.y <= 320 && p.x >= 80 && p.x <= 240)
      {
        activateDevices("deactivateLights");  
      }
      else if(p.y >= 0 && p.y <= 160 && p.x >= 80 && p.x <= 240)
      {
        activateDevices("activatePump");
      }
      else
      {
        break;
      }
    }
  }
}
void tempMenu(){
  mils = millis();
  accessServer();
  getData("temp.txt");
  tft.fillScreen(0x840F);
  tft.setCursor(150, 120);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(10);
  tft.print(temp); //Global variable
  tft.setTextSize(4);
  tft.print((char)247);
  tft.setTextSize(10);
  tft.print("C");
  pageFlag=0;
  while(!ts.touched())
  {
    delay(10);
    if(millis() >= mils + 20000)
    {
      pageFlag = 9;
      break;
    }
  }
}

void airMenu(){
  mils = millis();
  accessServer();
  getData("humi.txt");
  tft.fillScreen(0x840F);
  tft.setCursor(155, 120);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(10);
  tft.print(airHumi); //Global variable
  tft.print("%");
  pageFlag=0;
  while(!ts.touched())
  {
    delay(10);
    if(millis() >= mils + 20000)
    {
      pageFlag = 9;
      break;
    }
  }
}

void soilMenu(){
  mils = millis();
  accessServer();
  getData("soilHumi.txt");
  tft.fillScreen(0x840F);
  tft.setCursor(100, 120);
  tft.setTextColor(0XFFFF);
  tft.setTextSize(10);
  soilHumi = map(soilHumi, 778, 0, 0, 100);
  tft.printf("%3d%%", soilHumi); //Global variable
  pageFlag=0;
  while(!ts.touched())
  {
    delay(10);
    if(millis() >= mils + 20000)
    {
      pageFlag = 9;
      break;
    }
  }
}
 
void luxMenu(){
  mils = millis();
  accessServer();
  getData("lux.txt");
  tft.fillScreen(0x840F);
  //tft.setCursor(150, 120);
    if(lux < 10)
  {
    tft.setCursor(120, 120);
  }
  else if(lux > 10 && lux < 100)
  {
    tft.setCursor(110, 120);
  }
  else if(lux >= 100 && lux < 1000)
  {
    tft.setCursor(100, 120);
  }
  else if(lux >= 1000 && lux < 10000)
  {
    tft.setCursor(90, 120);
  }
  else if(lux >= 10000 && lux < 100000)
  {
    tft.setCursor(80, 120);
  }
  else
  {
    tft.setCursor(140, 120);
  }
  tft.setTextColor(0XFFFF);
  tft.setTextSize(10);
  tft.print(lux); //Global variable
  tft.setTextSize(4);
  tft.print("lx");//Global variable
  pageFlag=0;
  while(!ts.touched())
  {
    delay(10);
    if(millis() >= mils + 20000)
    {
      pageFlag = 9;
      break;
    }
  }
}

void blankScreen()
{
   tft.fillScreen(0X00000);
}

void connectToWifi()
{
  //This function will test if the board has 
  //a connection to an access point
  //and if it doesn't it will attempt to connect
  //to a specific access point
  if((!(WiFi.status() == WL_CONNECTED)))
  {
     WiFi.begin("VM5614E97","ua86ybhyZXhy");
  }
}

void getData(String url){
   HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        // configure traged server and url
        http.begin("http://192.168.0.221/garden/"+ url); //HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                if(pageFlag == 2){
                  temp = payload.toInt();
                }
                else if(pageFlag == 3){
                  airHumi = payload.toInt();
                }
                else if(pageFlag == 4){
                  soilHumi = payload.toInt();
                }
                else if(pageFlag == 5){
                  lux = payload.toInt();
                }
                Serial.println(payload);
            }
        } 
        http.end();
}

void accessServer(){
   HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        // configure traged server and url
 
        http.begin("http://192.168.0.221/garden/sendTFTVals.php"); //HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

          //if the server responds with a positive value, it means connection was successful
          //if it's negative there was a problem
          if( httpCode > 0)
          {
            Serial.println("Successfully connected to server");
          }
          else
          {
            Serial.println(httpCode);
          }

        http.end();
}

void activateDevices(String url){
   HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        // configure traged server and url
        http.begin("http://192.168.0.59/"); //HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.POST(url + "/");

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                tft.setCursor(20, 250);
                tft.setTextColor(0XFFFF);
                tft.setTextSize(4);
                tft.println(payload);
            }
        } 
        http.end();
        //delay(2000);
        pageFlag=0;
}
void bmpDraw(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}



