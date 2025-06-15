#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <TFT_22_ILI9225.h>
#include <math.h>

// TFT Pins
#define TFT_CLK  18
#define TFT_MOSI 23
#define TFT_RST  33
#define TFT_RS   32
#define TFT_CS   27
#define TFT_LED  26

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED);

#define SCREEN_WIDTH 220
#define SCREEN_HEIGHT 176
#define CENTER_X 110
#define CENTER_Y 88
#define RADIUS 60
#define NEEDLE_LENGTH 50
#define MIN_ANGLE -135
#define MAX_ANGLE 135
#define MAX_SPEED 5000

uint16_t speed = 0;
uint8_t voltage = 0;
uint8_t current = 0;
uint8_t motorTemp = 0;
uint8_t controllerTemp = 0;

int last_speed = -1, last_voltage = -1, last_current = -1, last_motorTemp = -1, last_controllerTemp = -1;

float mapSpeedToAngle(int speed) {
  return MIN_ANGLE + ((float)speed / MAX_SPEED) * (MAX_ANGLE - MIN_ANGLE);
}

typedef struct struct_message {
  int rpm;
  uint8_t voltage;
  uint8_t current;
  uint8_t motorTemp;
  uint8_t controllerTemp;
} struct_message;

struct_message incomingData;


void OnDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  speed = incomingData.rpm;
  voltage = incomingData.voltage;
  current = incomingData.current;
  motorTemp = incomingData.motorTemp;
  controllerTemp = incomingData.controllerTemp;

  update_display();
  updateSpeedDial();
  Serial.printf("RPM: %d, Voltage: %u, Current: %u, MotorTemp: %u, ControllerTemp: %u\n",
  incomingData.rpm, incomingData.voltage, incomingData.current, incomingData.motorTemp, incomingData.controllerTemp);
}


void drawSpeedDial() {
  tft.drawCircle(CENTER_X, CENTER_Y, RADIUS, COLOR_WHITE);
  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i += 45) {
    float angle = i * PI / 180;
    int x1 = CENTER_X + (RADIUS - 8) * sin(angle - PI);
    int y1 = CENTER_Y + (RADIUS - 8) * cos(angle - PI);
    int x2 = CENTER_X + RADIUS * sin(angle - PI);
    int y2 = CENTER_Y + RADIUS * cos(angle - PI);
    tft.drawLine(x1, y1, x2, y2, COLOR_WHITE);
  }
  tft.setFont(Terminal6x8);
  tft.drawText(CENTER_X - 50, CENTER_Y + RADIUS - 15, "0", COLOR_WHITE);
  tft.drawText(CENTER_X - 15, CENTER_Y - RADIUS - 10, "2500", COLOR_WHITE);
  tft.drawText(CENTER_X + 45, CENTER_Y + RADIUS - 15, "5000", COLOR_WHITE);
}

void valuelabel() {
  tft.setFont(Terminal11x16);
  tft.drawText(10, 10, "V: ", COLOR_WHITE);
  tft.drawText(SCREEN_WIDTH - 60, 10, "I: ", COLOR_WHITE);
  tft.drawText(10, SCREEN_HEIGHT - 26, "Motor: ", COLOR_WHITE);
  tft.drawText(SCREEN_WIDTH - 95, SCREEN_HEIGHT - 26, "Ctrl: ", COLOR_WHITE);
}

void drawNeedle(int angle, uint16_t color) {
  float rad = angle * PI / 180;
  int x = CENTER_X + NEEDLE_LENGTH * sin(rad);
  int y = CENTER_Y + NEEDLE_LENGTH * cos(rad + PI);
  tft.drawLine(CENTER_X, CENTER_Y, x, y, color);
  tft.fillCircle(CENTER_X, CENTER_Y, 4, COLOR_WHITE);
}

void clearNeedle(int angle) {
  float rad = angle * PI / 180;
  int x = CENTER_X + NEEDLE_LENGTH * sin(rad);
  int y = CENTER_Y + NEEDLE_LENGTH * cos(rad + PI);
  tft.drawLine(CENTER_X, CENTER_Y, x, y, COLOR_BLACK);
}

void updateSpeedDial() {
  static int last_angle = MIN_ANGLE;
  int new_angle = mapSpeedToAngle(speed);
  clearNeedle(last_angle);
  drawNeedle(new_angle, COLOR_RED);
  last_angle = new_angle;
}

void update_display() {
  char buffer[10];

  if (speed != last_speed) {
    tft.fillRectangle(CENTER_X - 25, CENTER_Y + RADIUS - 22, CENTER_X + 25, CENTER_Y + RADIUS, COLOR_BLACK);
    sprintf(buffer, "%d", speed);
    tft.setFont(Terminal12x16);
    tft.drawText(CENTER_X - 25, CENTER_Y + RADIUS - 22, buffer, COLOR_WHITE);
    last_speed = speed;
  }

  if (voltage != last_voltage) {
    tft.fillRectangle(30, 10, 70, 26, COLOR_BLACK);
    sprintf(buffer, "%dV", voltage);
    tft.setFont(Terminal11x16);
    tft.drawText(30, 10, buffer, COLOR_YELLOW);
    last_voltage = voltage;
  }

  if (current != last_current) {
    tft.fillRectangle(SCREEN_WIDTH - 40, 10, SCREEN_WIDTH, 26, COLOR_BLACK);
    sprintf(buffer, "%dA", current);
    tft.setFont(Terminal11x16);
    tft.drawText(SCREEN_WIDTH - 40, 10, buffer, COLOR_CYAN);
    last_current = current;
  }

  if (motorTemp != last_motorTemp) {
    tft.fillRectangle(70, SCREEN_HEIGHT - 26, 120, SCREEN_HEIGHT, COLOR_BLACK);
    sprintf(buffer, "%dC", motorTemp);
    tft.setFont(Terminal11x16);
    tft.drawText(73, SCREEN_HEIGHT - 26, buffer, COLOR_RED);
    last_motorTemp = motorTemp;
  }

  if (controllerTemp != last_controllerTemp) {
    tft.fillRectangle(SCREEN_WIDTH - 42, SCREEN_HEIGHT - 26, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BLACK);
    sprintf(buffer, "%dC", controllerTemp);
    tft.setFont(Terminal11x16);
    tft.drawText(SCREEN_WIDTH - 42, SCREEN_HEIGHT - 26, buffer, COLOR_ORANGE);
    last_controllerTemp = controllerTemp;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  tft.begin();
  tft.setOrientation(1);
  tft.clear();
  drawSpeedDial();
  valuelabel();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Ready, waiting for data...");
}

void loop() {
  // Nothing to do here, updates happen in callback
}
