/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <epd.h>
#include<stdlib.h>
#include <math.h>
#include "RTClib.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

byte char2[10][5]={
  {
   0x7C,
   0x82,
   0x82,
   0x82,
   0x7C   
  },
  {
   0x00,
   0x00,
   0x00,
   0x02,
   0xFE   
  },
  {
   0xE4,
   0x92,
   0x92,
   0x92,
   0x4C  
  },
  {
   0x44,
   0x92,
   0x92,
   0x92,
   0x6C  
  }
,
  {
   0x1E,
   0x10,
   0x10,
   0x10,
   0xFE  
  }
,
  {
   0x5E,
   0x92,
   0x92,
   0x92,
   0x62  
  }
,
  {
   0x7C,
   0x92,
   0x92,
   0x92,
   0x64
  }
,
  {
   0x02,
   0xE2,
   0x12,
   0x0A,
   0x06
  }
,
  {
   0x6C,
   0x92,
   0x92,
   0x92,
   0x6C
  }
,
  {
   0x4C,
   0x92,
   0x92,
   0x92,
   0x7C
  }

};

unsigned long delayTime;
   int t_hour=0;
   int t_minute=0;
   

void draw_arrow(int x, int y, int dir){
  if (dir==1){
    plotLine(x,y+40,x+40,y,1);
    plotLine(x+40,y+40,x+40,y,1);
    plotLine(x,y,x+40,y,1);
  }
  else if (dir==-1){
    plotLine(x+40,y,x+40,y+40,1);
    plotLine(x,y+40,x+40,y+40,1);
    plotLine(x,y,x+40,y+40,1);    
  }
  else if (dir==0){
    plotLine(x,y+20,x+40,y+20,1);
    plotLine(x+20,y,x+40,y+20,1);
    plotLine(x+20,y+40,x+40,y+20,1);
  }  
}

void draw_sun(int x, int y){
//425,285
  int sz=10;
  int brush_size=6;
  epd_set_color(BLACK,WHITE);
  epd_fill_circle(x, y, 8*sz);
  epd_fill_rect(x-15*sz, y+1, x-9*sz, y+brush_size);//left horizontal
  epd_fill_rect(x+9*sz, y, x+sz*15, y+brush_size);//right horizontal
  epd_fill_rect(x, y-sz*15, x+brush_size, y-9*sz);//upper vertical 
  epd_fill_rect(x, y+9*sz+1, x+brush_size, y+sz*15+2);//lower vertical
  plotLine(x+11*sz-2, y-(+11*sz-2),x+6*sz+2, y-6*sz+1,3);
  plotLine(x+11*sz, y+11*sz, x+6*sz+3, y+6*sz+2,3);
  plotLine(x-11*sz-1, y+11*sz, x-6*sz-3, y+6*sz+4,3);
  plotLine(x-(11*sz-1), y-(11*sz), x-(6*sz+3), y-(6*sz+4),3);
  epd_set_color(WHITE,BLACK);
  epd_fill_circle(x, y, 8*sz-6);
  
}

void draw_cloud(int x, int y){
  //400,270
  int sz=10;
  epd_set_color(BLACK, WHITE);
  epd_fill_circle(x+sz, y+3*sz, 6*sz+6);
  epd_fill_circle(x+8*sz, y, 7*sz);
  epd_fill_circle(x+sz*12, y+4*sz, 7*sz);
  epd_fill_circle(x+20*sz, y+sz*2, sz*6);

  epd_set_color(WHITE,BLACK);
  epd_fill_circle(x+sz, y+3*sz, 6*sz);
  epd_fill_circle(x+8*sz, y, 7*sz-6);
  epd_fill_circle(x+sz*12, y+4*sz, 7*sz-6);
  epd_fill_circle(x+20*sz, y+sz*2, sz*6-6);

}

void draw_rain(int x, int y){
//400,330
  int sz=10;
  epd_set_color(BLACK, WHITE);
  epd_fill_rect(x, y, x+6, y+4*sz);
  epd_fill_rect(x+4*sz, y+sz*2, x+4*sz+6, y+6*sz);
  epd_fill_rect(x+8*sz, y, x+8*sz+6, y+4*sz);
  epd_fill_rect(x+12*sz, y+2*sz, x+12*sz+6, y+6*sz);

  epd_fill_rect(x+16*sz, y, x+16*sz+6, y+4*sz);
  epd_fill_rect(x+20*sz, y+sz, x+20*sz+6, y+6*sz);
  epd_fill_rect(x+24*sz, y, x+24*sz+6, y+4*sz);
  
}


void draw_char(unsigned char c[5], int xbegin, int ybegin, byte radius) 
{  
  epd_set_color(BLACK, WHITE);
  char ctemp;
  for (int x=0; x<5; x++){
     ctemp=c[x];
     for (int y=0; y<7; y++)
     {
        ctemp>>=1;
        if ( ctemp & 01)
          epd_fill_circle(x*radius*3+xbegin, y*radius*3+ybegin, radius);
//        else
//          epd_draw_circle(x*radius*3+xbegin, y*radius*3+ybegin, radius);
     }
   }
}


void draw_text(char text[], int x, int y, int font_size)
{ 
  if (font_size==64) 
    epd_set_en_font(ASCII64);
  else if (font_size==48)
    epd_set_en_font(ASCII48);
 else
    epd_set_en_font(ASCII32);

  epd_disp_string(text, x, y);
}

void plotLine(int x0,int y0, int x1,int y1, int brush_size)
  {
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  for(;;){
    epd_fill_circle(x0,y0,brush_size);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void circle_text(char txt[], int len, int x,int y, int radius){
  int left=x;
    for (int i=0; i<len; i++){
      if (txt[i]>=48 and txt[i]<=57){
        draw_char(char2[txt[i]-48], left, y,radius);
        if (txt[i+1]==49)
          left+=radius*10;
        else
          left+=radius*20;
        }
        else if (txt[i]==58){
         epd_fill_circle(left, y+radius*15, radius);
         epd_fill_circle(left, y+radius*4, radius);
         left+=radius*8;
        }
        else if (txt[i]==46){
         epd_fill_circle(left, y+radius*18, radius);
         left+=radius*8;
        }
        
        else{
          left+=radius*20;
          }  
    }
}

/*
 * station2sealevel verified, working correctly *
 */
int station2sealevel(int p, int height, int t){
  return (double) p*pow(1-0.0065*(double)height/(t+0.0065*(double)height+273.15),-5.275);
}

int myabs(int val){
  if (val<0)
    return -val;
  return val;
  }

int calc_zambretti(int curr_pressure, int prev_pressure, int mon){
        if (curr_pressure<prev_pressure){
          //FALLING
          if (mon>=4 and mon<=9)
          //summer
          {
            if (curr_pressure>=1030)
              return 2;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 8;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 18;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 21;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 24;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 24;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 26;
            else if(curr_pressure<970)
              return 26;
          }
          else{
          //winter
            if (curr_pressure>=1030)
              return 2;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 8;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 15;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 21;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 22;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 24;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 26;
            else if(curr_pressure<970)
              return 26;
          }
        }
        else if (curr_pressure>prev_pressure){
          //RAISING
          if (mon>=4 and mon<=9){
            //summer
            if (curr_pressure>=1030)
              return 1;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 2;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 3;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 7;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 9;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 12;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 17;
            else if(curr_pressure<970)
              return 17;
          }
          else
            //winter
           {
            if (curr_pressure>=1030)
              return 1;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 2;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 6;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 7;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 10;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 13;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 17;
            else if(curr_pressure<970)
              return 17;
           }
        }
        else{
            if (curr_pressure>=1030)
              return 1;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 2;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 11;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 14;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 19;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 23;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 24;
            else if(curr_pressure<970)
              return 26;

        }

}

void sunny(int x, int y){
  draw_sun(x,y);
}

void sunny_cloudy(int x, int y){
  draw_sun(x,y);
  draw_cloud(x,y);
}

void cloudy(int x, int y){
  draw_cloud(x,y);  
}

void worsening(int x, int y){
  draw_sun(x,y);
  draw_cloud(x,y);
  draw_rain(400,y+100);
}

void rainy(int x, int y){
  draw_cloud(x,y);
  draw_rain(x,y+100);
}

int pressureArray[10]={0};
byte counter=0;
byte delta_time=0;

RTC_DS3231 rtc;
Adafruit_BME280 bme; // I2C

void setup() {
    bool status;
    // default settings
    status = bme.begin();
    if (!status) {
       while (1);
    } 
    delayTime = 20000;
    epd_init();
    epd_wakeup();
    epd_set_memory(MEM_NAND);
 //   epd_clear();
//    epd_set_color(WHITE, BLACK);
    delay(100); // let sensor boot up
}


void loop() {    
  char tStr[21];
  char pStr[22];
  char hStr[20];
  char pseaStr[26];
  char timeStr[6];
  char dateStr[12];
  char zambretti[10]="N/A";
  char pressureHistory[57];
  int temperature=(int)bme.readTemperature();
  int humidity=(int)bme.readHumidity();
  int pressure=(int)(bme.readPressure()/100.0F);
  int altitude=560;//(int)bme.readAltitude(SEALEVELPRESSURE_HPA);
  int seapressure = station2sealevel(pressure,altitude,temperature);
  //int seapressure = station2sealevel(944,595,27);

  DateTime now = rtc.now();
  int t_hour2=now.hour();
  int t_minute2=now.minute();
  int Z=0;

  if (t_hour2!=t_hour or t_minute2!=t_minute){
    delta_time++;
    if (delta_time>10){
      delta_time=0;
      
      if (counter==10)
      {
        for (int i=0; i<9;i++){
          pressureArray[i]=pressureArray[i+1];
        }
        pressureArray[counter-1]=seapressure; 
      }
      else{
        pressureArray[counter]=seapressure;  
        counter++;
      }
    }
  Z=calc_zambretti((pressureArray[9]+pressureArray[8]+pressureArray[7])/3,(pressureArray[0]+pressureArray[1]+pressureArray[2])/3, now.month());
  sprintf(zambretti, "Z=%d", Z);
     epd_wakeup();
     epd_clear();
     sprintf(tStr, "%d C", temperature);
     sprintf(hStr, "%d %%", humidity);
     sprintf(pStr, "%d hPa", pressure);
     sprintf(pseaStr, "%d hPa", seapressure);
     sprintf(dateStr, "%02d.%02d.%d", now.day(), now.month(),now.year());
     sprintf(timeStr, "%02d:%02d", now.hour(), now.minute());
     
     //sprintf(pressureHistory, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",pressureArray[0],pressureArray[1],pressureArray[2],
     //pressureArray[3],pressureArray[4],pressureArray[5],pressureArray[6],pressureArray[7],pressureArray[8],pressureArray[9]);
     //sprintf(pressureHistory, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",calc_zambretti(1030,1040,1),calc_zambretti(1020,1030,1),calc_zambretti(1010,1020,1),calc_zambretti(1000,1010,1),
     //calc_zambretti(990,1000,1),calc_zambretti(980,990,1),calc_zambretti(970,980,1),calc_zambretti(960,970,1),calc_zambretti(950,960,1),calc_zambretti(940,950,1));

    // sprintf(pressureHistory, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",station2sealevel(930, 600,25),station2sealevel(940, 600,25),station2sealevel(950, 600,25),station2sealevel(960, 600,25),station2sealevel(970, 600,25)
    // ,station2sealevel(980, 600,25),station2sealevel(990, 600,25),station2sealevel(1000, 600,25),station2sealevel(1010, 600,25),station2sealevel(1020, 600,25));


     circle_text(tStr, strlen(tStr),30,550, 2);
     draw_text("temp", 30,510, 32);

     circle_text(pStr, strlen(pStr), 230,550,2);
     draw_text("press", 230,510,32);
     
     circle_text(hStr, strlen(hStr), 430,550, 2);
     draw_text("humid", 430,510, 32);
     
     circle_text(pseaStr, strlen(pseaStr), 630,550, 2);
     draw_text("rel. press.", 630,510, 32);

    // draw_text(zambretti, 600,240, 32);
     //draw_text(pressureHistory, 30,570,32);
     
   //  DEBUG, REMOVED
//     pressureArray[9]=1020;
//     pressureArray[8]=1020;
//     pressureArray[7]=1020;
//     pressureArray[6]=1020;
//     pressureArray[5]=1020;
//     pressureArray[4]=1020;
//     pressureArray[3]=1020;
//     pressureArray[2]=1020;
//     pressureArray[1]=1020;
//     pressureArray[0]= 1021;
//     Z=6;

     if (pressureArray[9]>0 and pressureArray[0]>0){
        if (pressureArray[9]+pressureArray[8]+pressureArray[7]-pressureArray[0]-pressureArray[1]-pressureArray[2]>=3){
      //RAISING
       draw_arrow(670,450,1);
        if (Z<3){
          sunny(500,350);
        }
        else if (Z>=3 and Z<=9){
          sunny_cloudy(400,350);
        }
        else if (Z>9 and Z<=17)
          cloudy(300,300);
        else if (Z>17){
          rainy(300,300);
          }
      }

        else if (pressureArray[0]+pressureArray[1]+pressureArray[2]-pressureArray[9]-pressureArray[8]-pressureArray[7]>=3){
      //FALLING
        draw_arrow(670,450,-1);
        if (Z<4)
          sunny(400,350);
        else if (Z>=4 and Z<14){
          sunny_cloudy(400,350);
        }
        else if (Z>=14 and Z<19){
         worsening(400,350);
        }
        else if (Z>=19 and Z<21)
          cloudy(300,300);
        else if (Z>=21){
          rainy(300,300);
          }
      }
      else{
       //STEADY
        draw_arrow(670,450,0);
       if (Z<5)
          sunny(400,350);
        else if (Z>=5 and Z<=11){
          sunny_cloudy(400,350);
        }
        else if (Z>11 and Z<14)
          cloudy(300,300);
        else if (Z>=14 and Z<19){
          worsening(400,350);
        }
        else if (Z>19){
          rainy(300,300);
        }
      }
     } 
     else{
      if (seapressure<1005)
        rainy(300,300);
      else if (seapressure>=1005 and seapressure<=1015)
        cloudy(300,300);
      else if (seapressure>1015 and seapressure<1025)
         sunny_cloudy(400,350);
      else
        rainy(300,300);
     }

//     draw_sun(400,350);//DEBUG
//     draw_cloud(300,300);//DEBUG
//     draw_rain(300,400); //DEBUG
     
    circle_text(dateStr,strlen(dateStr),150,140,3);
    circle_text(timeStr,strlen(timeStr),180,10,5);
    
    //find min and max pressure values for scaling graph
//    int pmin=pressureArray[0];
//    int pmax=pressureArray[0];
//    for (int i=1; i<=9; i++){
//      if (pmin>pressureArray[i])
//        pmin=pressureArray[i];
//      else if (pmax<pressureArray[i])
//        pmax=pressureArray[i];
//    }
    //draw pressure history graph    
//    for (int i=0; i<=9; i++)
//      epd_fill_rect(i*80, 600, (i+1)*80-5, 590-((float)(pressureArray[i]-pmin)/max((pmax-pmin),1)*100.0F));
    t_hour=t_hour2;
    t_minute=t_minute2;
  
    epd_udpate();
    epd_enter_stopmode();

  }
    delay(delayTime);
    
}
