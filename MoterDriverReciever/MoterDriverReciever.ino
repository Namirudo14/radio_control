#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>


const int AIN1 = 16;
const int AIN2 = 17;
const int BIN1 = 18;
const int BIN2 = 19;

typedef struct {
  uint8_t forward;
  uint8_t backward;
} MotorCommand;

/// モーターの初期設定（出力ピン設定） ///
void SetupMotors() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

/// モーターを制御する関数 ///
/// forward: 前進, backward: 後退 ///
void DriveMotors(uint8_t forward, uint8_t backward) {
  if (forward > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else if (backward > 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  }
}

/// ESP-NOWでデータを受信したときのコールバック関数 ///
void OnReceive(const esp_now_recv_info_t* recvInfo, const uint8_t* data, int len) {
  MotorCommand command;
  memcpy(&command, data, sizeof(command));
  DriveMotors(command.forward, command.backward);

  const uint8_t* mac = recvInfo->src_addr;
  Serial.printf("Received from %02X:%02X:%02X:%02X:%02X:%02X\n",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  SetupMotors();

  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(OnReceive);
  }
}

void loop() {
  // メインループでは何もしない（受信時に処理）
}
