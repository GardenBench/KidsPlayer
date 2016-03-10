/*
  KidsPlayer.ino

  Copyright (c) 2016 Jeroen Hilgers
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "cjson.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <AudioZero.h>

/////////////////////////////////////////////////////////////////////////////
// Globals
File myFile;
uint8_t fileBuf[1025];

/////////////////////////////////////////////////////////////////////////////
// LCD interface

enum BMP_MODE
{
  BMP_WRITE = 1,
  BMP_OR = 2,
  BMP_AND = 3,
  BMP_XOR = 4
};

void LcdImage(BMP_MODE mode, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *ptr)
{
  uint16_t cnt;
  Wire.beginTransmission(0x07);
  Wire.write(0xA4);              
  Wire.write(mode);              
  Wire.write(x);              
  Wire.write(y);              
  Wire.write(w);              
  Wire.write(h);              
  cnt = (w+7)/8;
  cnt *= h;
  for(; cnt; cnt--)
    Wire.write(*ptr++);
  Wire.endTransmission();
}

void LcdPrint(const char *str, const char *file)
{
  while(*str)
  {
    Wire.beginTransmission(0x07);
    Wire.write(0xA3);              
    Wire.write(*str++);              
    Wire.endTransmission();
  }
  while(*file)
  {
    Wire.beginTransmission(0x07);
    Wire.write(0xA3);              
    Wire.write(*file++);              
    Wire.endTransmission();
  }
}

/////////////////////////////////////////////////////////////////////////////
// BMP file reader

void ShowBmp(const char *fileName)
{
   myFile = SD.open(fileName);
   if (!myFile) 
   {
    LcdPrint("ShowBmp file open failed!", fileName);
    return;
   }
   myFile.read(fileBuf, 54); // Bmp header
   uint16_t offset = (uint16_t)fileBuf[10] + ((uint16_t)fileBuf[11])*256;
   myFile.seek(offset);
   myFile.read(fileBuf, 1024); // Bmp data.
   myFile.close();   
   // Draw.
   uint16_t y;
   for(y=0; y<1024; y++)
    fileBuf[y] = fileBuf[y]^0xFF;
   for(y=0; y<64; y++)
   {
        LcdImage(BMP_WRITE, 0, y , 128, 1, &fileBuf[(63-y)*16]);
   }
}


const char *nextScreen = 0;

// Process the fields.
void ProcessItem (cJSON *ptr)
{
  static uint8_t audioInitialized = 0;
  if(!ptr)
    return;
  cJSON *field;
  // 'image' to display?
  field = cJSON_GetObjectItem(ptr, "image");
  if(field)
  {
    if(field->valuestring)
      ShowBmp(field->valuestring);
  }
  // 'color' for RGB led?
  field = cJSON_GetObjectItem(ptr, "color");
  if(field)
  {
    if(field->valuestring)
    {
      // Dirty:
      if(field->valuestring[2] > '0')
         digitalWrite(1, HIGH);
      else
         digitalWrite(1, LOW);
      if(field->valuestring[4] > '0')
         digitalWrite(2, HIGH);
      else
         digitalWrite(2, LOW);
      if(field->valuestring[6] > '0')
         digitalWrite(3, HIGH);
      else
         digitalWrite(3, LOW);
    }
  }
  // 'load' field for next screen?
  field = cJSON_GetObjectItem(ptr, "screen");
  if(field)
  {
    nextScreen = field->valuestring;
  }
  // 'sound' field for raw (44.khz, 8bit unsinged) audio playback.
  field = cJSON_GetObjectItem(ptr, "sound");
  if(field)
  {
    if(field->valuestring)
    {  
      myFile = SD.open(field->valuestring);
      if(myFile)
      {
        if(!audioInitialized)
        {
          audioInitialized = 1;
          AudioZero.begin(44100);
        }
        AudioZero.play(myFile);   
      }
    }
  }
}

cJSON *currentScreen = 0;

void LoadScreen (const char *name)
{
  // Load file into memory.
  myFile = SD.open(name);
  if (!myFile) 
  {
    LcdPrint("LoadScreen file open failed!", name);
    return;
  }
  uint16_t len = myFile.read(fileBuf, 1024);
  fileBuf[len] = 0; // Terminate string.
  myFile.close();

  // Discard previous screen if any.
  if(currentScreen)
    cJSON_Delete(currentScreen);

  // Parse JSON from newly loaded screen.
  currentScreen = cJSON_Parse((char *)fileBuf);
  if(!currentScreen)
  {
    LcdPrint("LoadScreen JSON decode failed!", name);
    return;
  }

  // Process 'init' field of loaded screen.
  ProcessItem(cJSON_GetObjectItem(currentScreen,"init"));
}

void setup()
{
  // RGB LCD backlight LED
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  // Push buttons.
  pinMode(A1, INPUT_PULLUP);  
  pinMode(A2, INPUT_PULLUP);  
  pinMode(A3, INPUT_PULLUP);  
  pinMode(A4, INPUT_PULLUP);  
  
  // SD card.
  if (!SD.begin(4)) 
  {
    LcdPrint("Initialize SD card failed!", "");
    return;
  }
  
  // I2C display.
  Wire.begin(); 
  
  // Let the mainloop load the initial screen.
  nextScreen = "main.scr";
}

void loop() 
{
  // See if a new screen should be loaded.
  if(nextScreen)
  {
    const char *tmp = nextScreen;
    nextScreen = 0; // Clear 'nextScreen' as 'LoadScreen' might immediately refer to 
                    // a new screen.
    LoadScreen(tmp);
  }
  
  // See if the user is pressing a button. 
  if(digitalRead(A1)==LOW)
  {
    ProcessItem(cJSON_GetObjectItem(currentScreen,"B1"));
  }
  if(digitalRead(A2)==LOW)
  {
    ProcessItem(cJSON_GetObjectItem(currentScreen,"B2"));
  }
  if(digitalRead(A3)==LOW)
  {
    ProcessItem(cJSON_GetObjectItem(currentScreen,"B3"));
  }
  if(digitalRead(A4)==LOW)
  {
    ProcessItem(cJSON_GetObjectItem(currentScreen,"B4"));
  }
}
