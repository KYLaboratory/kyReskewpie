#define OUTPUT_LED 12
#define INPUT_BUTTON 7
#define EFFECT_START_SIGNAL 1
#define EFFECT_STOP_SIGNAL 2
#define SPEED 9600
#define DELAY_TIME 50

int inputValue = LOW;
int inputState = 1; // 入力待ち状態
int output_val = LOW;

void setup() {
  pinMode(OUTPUT_LED, OUTPUT);
  pinMode(INPUT_BUTTON, INPUT);
  Serial.begin(SPEED);
}

void loop() {
  // ボタンからの入力を保持
  inputValue = digitalRead(INPUT_BUTTON);
  if ((inputValue == HIGH) && (inputState == 1)) {
    inputState = 0;
    // チャタリング防止
    delay(DELAY_TIME);
    // PCにエフェクト開始を通知
    Serial.write(EFFECT_START_SIGNAL);
  } else {
    inputState = 1;
  }
  
  if (Serial.available()) {
    if (Serial.read() == EFFECT_STOP_SIGNAL) {
      digitalWrite(OUTPUT_LED, HIGH);
    } else {
      digitalWrite(OUTPUT_LED, LOW);
    }
  }
}
