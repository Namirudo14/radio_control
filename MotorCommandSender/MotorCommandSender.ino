#include <esp_now.h>
#include <WiFi.h>

int AIN1 = 16, AIN2 = 17;
int BIN1 = 18, BIN2 = 19;

typedef struct {
  uint8_t forward;
  uint8_t backward;
} MotorCommand;

uint8_t receiverMac[] = {0xB8, 0xD6, 0x1A, 0xBC, 0xEC, 0xD4}; // 受信側MACに変更

/// ESP-NOWの初期化とペア設定 ///
void SetupEspNow() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

/// モーターコマンドを送信する関数 ///
/// 固定値を送る（今はテスト用） ///
void SendMotorCommand() {
  MotorCommand command = {200, 0};
  esp_now_send(receiverMac, (uint8_t*)&command, sizeof(command));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  SetupEspNow();
}

void loop() {
  SendMotorCommand();
  delay(1000);
}
