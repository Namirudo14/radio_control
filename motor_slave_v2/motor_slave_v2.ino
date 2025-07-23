(フットコントローラ上のesp32,ラジコン上のesp32間のbluetooth通信)
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

//valuable
int AIN1 = 18;
int AIN2 = 19;
int f=0;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test2"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  pinMode(26,OUTPUT);
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);


}

void loop() {
  if (SerialBT.available()) {
    //データ受信  
    uint8_t a[2];
    SerialBT.readBytes(a,2);
    Serial.println(a[0]);
    Serial.println(a[1]);


　　//モーターの制御
    if(a[0]>0){
    //前進
    analogWrite(AIN1,a[0]);
    analogWrite(AIN2,0);
    delay(50);
    }else if(a[1]>0){
    //後退
    analogWrite(AIN1,0);
    analogWrite(AIN2,a[1]);
    delay(50);
    }else{
      analogWrite(AIN1,0);
      analogWrite(AIN2,0);
      delay(50);
    }

    
  }
  
}
