// Arduino摩爾斯解釋器（訓練用）
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define KEY 7      // 電鍵從引腳 7 連接到 GND
#define BUZZER 8   // 蜂鳴器從引腳 8 連接到 GND
#define LED_PIN 4 // LED燈從引腳 4 連接到 GND（使用220歐姆電阻）

LiquidCrystal_I2C lcd(0x27, 16, 2);  // 設定 LCD 地址為 0x27，用於 16 字符 2 行的顯示器

void setup() {
  pinMode(KEY, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  lcd.init();                      // 初始化 LCD
  lcd.backlight();                 // 開啟背光
}

float DashDuration = 200.0;        // 劃的持續時間（ms）
boolean PrevState = false;
long tStartChar, tStartPause;
boolean State;
String currentSymbol = "";

void loop() {
  State = !digitalRead(KEY);       // 讀取按鍵狀態（由於 INPUT_PULLUP 而反轉）

  if (State) {                     // 按鍵按下
    if (State != PrevState) {      // 狀態改變
      tStartChar = millis();
      DecodePause(tStartPause);
    }
    digitalWrite(BUZZER, HIGH);    // 打開蜂鳴器
    digitalWrite(LED_PIN, HIGH);   // 點亮 LED 燈
  }
  else {                           // 按鍵釋放
    if (State != PrevState) {      // 狀態改變
      tStartPause = millis();
      DecodeChar(tStartChar);
    }
    digitalWrite(BUZZER, LOW);     // 關閉蜂鳴器
    digitalWrite(LED_PIN, LOW);    // 關閉 LED 燈
  }

  // 檢查長暫停（單詞結束）
  if (abs(millis() - tStartPause) > DashDuration * 10) {
    DecodePause(tStartPause);
  }

  PrevState = State;
}

// 解碼按下的字符（點或劃）
void DecodeChar(long startTime) {
  char symbol = '?';
  long duration = abs(millis() - startTime); // 發送字符的持續時間
  float DotDuration = DashDuration / 3.0;

  if (duration <= 2) return; // 防抖動

  if (duration <= DotDuration) symbol = '.';
  else if (duration > DashDuration) symbol = '-';
  else if ((duration > (DashDuration + DotDuration) / 1.9) && duration <= DashDuration) symbol = '-';
  else symbol = '.';
  
  // 自動校準時間
  if (symbol == '-') {
    if (duration > DashDuration) DashDuration++;
    if (duration < DashDuration) DashDuration--;
  }
  else if (symbol == '.') {
    if (duration > DashDuration / 3.0) DashDuration++;
    if (duration < DashDuration / 3.0) DashDuration--;
  }
  
  currentSymbol += symbol;
}

// 解碼字符之間的暫停
void DecodePause(long startTime) {
  if (currentSymbol == "") return;

  long duration = abs(millis() - startTime);
  
  // 字符結束（暫停 > 1 單位）
  if (duration > DashDuration - DashDuration / 40) {
    DecodeSymbol();
  }
  
  // 單詞結束（暫停 > 2 單位）
  if (duration > DashDuration * 2) {
    DecodeSymbol();
    PrintLCD(" ");
    Serial.print(" ");
  }
}

// 將莫爾斯電碼符號解碼為字母
void DecodeSymbol() {
  static String morseLetters[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "E"
  };
  int i = 0;
  while (morseLetters[i] != "E") {
    if (morseLetters[i] == currentSymbol) {
      char letter = (char)('A' + i);
      PrintLCD(String(letter));
      Serial.print(letter);
      break;
    }
    i++;
  }
  if (morseLetters[i] == "E") {
    PrintLCD(currentSymbol);
    Serial.print(currentSymbol);
  }
  currentSymbol = "";
}

// LCD 光標位置變量
int col = 0, row = 0;

// 將字符串打印到 LCD 並管理光標位置
void PrintLCD(String s) {
  for (int i = 0; i < s.length(); i++) {
    updateCursor();
    lcd.print(s[i]);
  }
}

// 為 16x2 LCD 更新光標位置
void updateCursor() {
  lcd.setCursor(col, row);
  col++;
  if (col >= 16) {  // 為 16x2 LCD 從 20 改為 16
    col = 0;
    row++;
  }
  if (row >= 2) {   // 為 16x2 LCD 從 4 改為 2
    lcd.clear();
    col = 0;
    row = 0;
  }
}
