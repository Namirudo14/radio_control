#include <TFT_eSPI.h>

#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

#define BUTTON_PIN 23
#define SWITCH_PIN 22
#define POT_PIN    34

typedef struct {
  uint8_t left;
  uint8_t right;
  uint8_t dir;
} MotorCommand;

uint8_t receiverMac[] = {0xB8, 0xD6, 0x1A, 0xBC, 0xEC, 0xD4};
MotorCommand lastSent = {255, 255, 255};

const uint8_t BASE_SPEED = 200;
const int STEER_RANGE = 100;
const int DEAD_BAND = 5;

TFT_eSPI tft = TFT_eSPI();
int currentSteer = 0; // スコア用

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

void ShowStatus(const MotorCommand &cmd, bool buttonPressed, int steerValue) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);

  tft.setTextColor(TFT_WHITE);
  tft.printf("Button: %s\n", buttonPressed ? "ON" : "OFF");

  tft.setTextColor(TFT_YELLOW);
  tft.printf("Score : %+4d\n", steerValue); // -100～+100の表示

  tft.setTextColor(TFT_CYAN);
  if (cmd.dir == 1)
    tft.println("DIR   : \x18"); // ↑
  else if (cmd.dir == 2)
    tft.println("DIR   : \x19"); // ↓
  else
    tft.println("DIR   : -");
}

void SendMotorCommand(const MotorCommand &cmd, bool buttonPressed, int steerValue) {
  if (cmd.left != lastSent.left || cmd.right != lastSent.right || cmd.dir != lastSent.dir) {
    esp_now_send(receiverMac, (const uint8_t*)&cmd, sizeof(cmd));
    lastSent = cmd;
    Serial.printf("TX L:%d R:%d DIR:%d STEER:%d\n", cmd.left, cmd.right, cmd.dir, steerValue);

    ShowStatus(cmd, buttonPressed, steerValue);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  SetupEspNow();

  analogReadResolution(12);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Transmitter Ready");
  delay(1000);
}

int readSteerSigned() {
  int raw = analogRead(POT_PIN);
  long s = map(raw, 0, 4095, -STEER_RANGE, STEER_RANGE);
  if (abs(s) < DEAD_BAND) s = 0;
  return (int)s;
}

void loop() {
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);
  bool switchOn = (digitalRead(SWITCH_PIN) == LOW);

  MotorCommand cmd;
  currentSteer = readSteerSigned();

  if (!buttonPressed) {
    cmd = {0, 0, 0};
    SendMotorCommand(cmd, buttonPressed, currentSteer);
    delay(40);
    return;
  }

  uint8_t dir = switchOn ? 1 : 2;

  int left  = BASE_SPEED - currentSteer;
  int right = BASE_SPEED + currentSteer;
  left  = constrain(left,  0, 200);
  right = constrain(right, 0, 200);

  cmd.left = (uint8_t)left;
  cmd.right = (uint8_t)right;
  cmd.dir = dir;

  SendMotorCommand(cmd, buttonPressed, currentSteer);
  delay(40);
}
