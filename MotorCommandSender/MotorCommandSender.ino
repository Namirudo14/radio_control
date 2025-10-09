#include <esp_now.h>
#include <WiFi.h>

#define BUTTON_PIN 23    // タクトスイッチ（押すとLOW）
#define SWITCH_PIN 22    // 方向スイッチ（INPUT_PULLUP）
#define POT_PIN    34    // 可変抵抗（ADC1ピン：Wi-Fi併用可）

// 送るデータ：左右のスピードと進行方向
// dir: 0=停止, 1=前進, 2=後退
typedef struct {
  uint8_t left;   // 0-200
  uint8_t right;  // 0-200
  uint8_t dir; 
} MotorCommand;

uint8_t receiverMac[] = {0xB8, 0xD6, 0x1A, 0xBC, 0xEC, 0xD4}; // 受信側MACに置換

// 送信間引き用（状態変化時のみ送る）
MotorCommand lastSent = {255, 255, 255}; // ありえない初期値

// 設定
const uint8_t BASE_SPEED = 200;      // ボタン押下時の基準速度
const int STEER_RANGE = 100;         // 左右に加減する量の最大値
const int DEAD_BAND = 5;             // 可変抵抗の中央デッドバンド（%相当）

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

void SendMotorCommand(const MotorCommand &cmd) {
  // 状態が変わった時だけ送信
  if (cmd.left != lastSent.left || cmd.right != lastSent.right || cmd.dir != lastSent.dir) {
    esp_now_send(receiverMac, (const uint8_t*)&cmd, sizeof(cmd));
    lastSent = cmd;
    Serial.printf("TX L:%d R:%d DIR:%d\n", cmd.left, cmd.right, cmd.dir);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  // POT_PIN は analogRead なので pinMode不要

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  SetupEspNow();

  analogReadResolution(12); // ESP32標準 0-4095
  Serial.println("Transmitter ready");
}

int readSteerSigned() {
  // 0..4095 → -100..+100 にマップ
  int raw = analogRead(POT_PIN);
  long s = map(raw, 0, 4095, -STEER_RANGE, STEER_RANGE);

  // デッドバンド（中央付近を0に吸着）
  if (abs(s) < DEAD_BAND) s = 0;
  return (int)s; // -100..+100
}

void loop() {
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);
  bool switchOn = (digitalRead(SWITCH_PIN) == LOW); // INPUT_PULLUPなのでON=LOW

  MotorCommand cmd;

  if (!buttonPressed) {
    // 押していない間は停止を維持
    cmd = {0, 0, 0};
    SendMotorCommand(cmd);
    delay(40);
    return;
  }

  // 押している間は方向スイッチで前進/後退を切り替え
  uint8_t dir = switchOn ? 1 : 2; // ONで前進, OFFで後退（逆がよければここを反転）

  // ステア値で左右差をつける
  int steer = readSteerSigned(); // -100..+100
  int left  = BASE_SPEED - steer;
  int right = BASE_SPEED + steer;

  // 範囲制限
  left  = constrain(left,  0, 200);
  right = constrain(right, 0, 200);

  // 後退時のステア方向が直感と逆に感じる場合はここで左右を入れ替える
  // if (dir == 2) { int t = left; left = right; right = t; }

  cmd.left = (uint8_t)left;
  cmd.right = (uint8_t)right;
  cmd.dir = dir;

  SendMotorCommand(cmd);
  delay(40); // チャタリング＆送信間隔
}
