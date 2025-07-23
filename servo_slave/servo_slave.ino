//ESP32,wiiリモコン間の通信（wiiリモコンからセンサの情報を取得し、サーボモーターを制御）

#include "ESP32Wiimote.h"
#include <ESP32Servo.h>

ESP32Wiimote wiimote;
Servo myservo;
double angle;
int angle2;

int servoPin = 13;

static bool logging = true;
static long last_ms = 0;
static int num_run = 0, num_updates = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("ESP32Wiimote");
    
    wiimote.init();
    if (! logging)
        wiimote.addFilter(ACTION_IGNORE, FILTER_ACCEL); // optional
    
    Serial.println("Started");
    last_ms = millis();

      myservo.attach(servoPin);  // サーボモーターを指定したピンに接続
}

void loop()
{
    wiimote.task();
    num_run++;

    if (wiimote.available() > 0) 
    {
        AccelState   accel   = wiimote.getAccelState();

        if (logging)
        {
      
            Serial.printf(", wiimote.axis: %3d/%3d/%3d", accel.xAxis, accel.yAxis, accel.zAxis);


            angle = (double)accel.yAxis / 1.41;
            if(accel.yAxis <= 113){
              angle2 = 80;
            }else if(accel.yAxis >= 142){
              angle2 = 100;
            }else{
              angle2 = (int)angle;
            }

            Serial.printf("Accel x: %3d, y: %3d, z: %3d, Angle: %d\n", accel.xAxis, accel.yAxis, accel.zAxis, angle2);
            myservo.write(angle2); 
            delay(10);  
        }
    }
}