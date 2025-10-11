#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

const int AIN1 = 16; // 左モータ用 前進側
const int AIN2 = 17; // 左モータ用 後退側
const int BIN1 = 18; // 右モータ用 前進側
const int BIN2 = 19; // 右モータ用 後退側

typedef struct {
  uint8_t left;   // 0-200
  uint8_t right;  // 0-200
  uint8_t dir;    // 0=停止, 1=前進, 2=後退
} MotorCommand;

// PWM設定
const uint32_t PWM_FREQ = 10000;   // 10kHz
const uint8_t  PWM_RES  = 8;       // 8bit (0..255)
const int      SCALE_MAX_INPUT = 200;

inline uint8_t scaleToPwm(uint8_t v) {
  return (uint8_t)map(v, 0, SCALE_MAX_INPUT, 0, (1 << PWM_RES) - 1);
}

void SetupMotors() {
  // v3 API: ピンごとにアタッチ（チャネルは自動割当）
  ledcAttach(AIN1, PWM_FREQ, PWM_RES);
  ledcAttach(AIN2, PWM_FREQ, PWM_RES);
  ledcAttach(BIN1, PWM_FREQ, PWM_RES);
  ledcAttach(BIN2, PWM_FREQ, PWM_RES);

  // 初期は全停止
  ledcWrite(AIN1, 0);
  ledcWrite(AIN2, 0);
  ledcWrite(BIN1, 0);
  ledcWrite(BIN2, 0);
}

void DriveStop() {
  ledcWrite(AIN1, 0);
  ledcWrite(AIN2, 0);
  ledcWrite(BIN1, 0);
  ledcWrite(BIN2, 0);
}

void DriveForward(uint8_t left, uint8_t right) {
  uint8_t lp = scaleToPwm(left);
  uint8_t rp = scaleToPwm(right);
  // 前進側だけPWM、反対側はLOW
  ledcWrite(AIN2, 0);
  ledcWrite(BIN2, 0);
  ledcWrite(AIN1, lp);
  ledcWrite(BIN1, rp);
}

void DriveBackward(uint8_t left, uint8_t right) {
  uint8_t lp = scaleToPwm(left);
  uint8_t rp = scaleToPwm(right);
  // 後退側だけPWM、反対側はLOW
  ledcWrite(AIN1, 0);
  ledcWrite(BIN1, 0);
  ledcWrite(AIN2, lp);
  ledcWrite(BIN2, rp);
}

void ApplyMotorCommand(const MotorCommand &c) {
  switch (c.dir) {
    case 1: DriveForward(c.left, c.right); break;
    case 2: DriveBackward(c.left, c.right); break;
    default: DriveStop(); break;
  }
}

// 受信コールバック
void OnReceive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  if (len < (int)sizeof(MotorCommand)) return;
  MotorCommand c;
  memcpy(&c, data, sizeof(c));
  ApplyMotorCommand(c);

  const uint8_t* mac = info->src_addr;
  Serial.printf("RX %02X:%02X:%02X:%02X:%02X:%02X | L:%d R:%d DIR:%d\n",
    mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], c.left, c.right, c.dir);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  SetupMotors();

  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(OnReceive);
    Serial.println("ESP-NOW Receiver Ready");
  } else {
    Serial.println("ESP-NOW init failed");
  }
}

void loop() {
  // 受信で駆動
}
