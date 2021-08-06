#include <TM1637Display.h>
#include <SimpleTimer.h>

#define debounceTime 200      // debounce시간 설정(200ms)

// 와이파이 설정
String ssid = "iot401-1";
String PASSWORD = "iot123456";
String host = "192.168.0.9";
String port = "11540";

// 핀번호 설정
const uint8_t ledPowerPin = 4;
const uint8_t ledRockPin = 5;
const uint8_t fndPin[7] = {37, 38, 39, 40, 41, 42, 43};
const uint8_t ledThermoPin[8] = {46, 47, 48, 49, 50, 51, 52, 53};

const uint8_t swPowerPin = 2;
const uint8_t swRockPin = 3;
const uint8_t swThermoUpPin = 20;
const uint8_t swThermoDownPin = 21;
const uint8_t swTimerUpPin = 18;
const uint8_t swTimerDownPin = 19;

const uint8_t CLK = 31;
const uint8_t DIO = 30;

const uint8_t PressSensor = A1;

// 인터럽트에 쓰이는 변수
volatile int setTime = 0; // 타이머 시간 변수
volatile int thermo = 0;  // 온도 조절 변수
volatile bool powerFlag = false;  // 전원 변화를 체크하는 플래그 변수
volatile bool powerState = LOW;   // 전원led 상태 변수
volatile bool rockFlag = false;  // 잠금 변화를 체크하는 플래그 변수
volatile bool rockState = LOW;   // 잠금led 상태 변수
volatile bool timerFlag = false;   // 타이머 완료 확인 변수

// fnd 데이터 배열
const uint16_t fndData[9] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F};
uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };    // 시계형 7세그먼트 4칸모듈에 출력해주기 위해 저장되는 배열

// 타이머에 사용되는 변수
int num1, num2, num3, num4;             // 흘러가는 시간 출력 변수들(첫번째 칸수 ~ 네번째 칸 번호)
int current_sec = 0;                    // start를 누른후 흘러가는 초를 세주기 위한 변수
int pre_sec;
int period = 0;

// led 상태변수
bool ledState = LOW;

// 무게감지 플래그 변수
bool weightFlag = false;

// 시간 통신 변수
uint32_t onTime = 0;
uint32_t offTime = 0;
uint32_t sendTime = 0;
bool sendFlag = false;

int i;

SimpleTimer timer;                      // timer 를 쓰겠다고 선언함
TM1637Display display(CLK, DIO);        // clk, dio핀을 연결하고 시계형 7세그먼트 4칸 모듈에 출력함

void setup() {
  //set up led
  pinMode(ledPowerPin, OUTPUT);
  pinMode(ledRockPin, OUTPUT);
  for (i = 0; i < 8; i++) {
    pinMode(ledThermoPin[i], OUTPUT);
  }
  
  for (i = 0; i < 7; i++) {
    pinMode(fndPin[i], OUTPUT);
  }
  
  // set up sw
  pinMode(swPowerPin, INPUT_PULLUP);
  pinMode(swRockPin, INPUT_PULLUP);
  pinMode(swThermoUpPin, INPUT_PULLUP);
  pinMode(swThermoDownPin, INPUT_PULLUP);
  pinMode(swTimerUpPin, INPUT_PULLUP);
  pinMode(swTimerDownPin, INPUT_PULLUP);  

  // use interrupt
  attachInterrupt(digitalPinToInterrupt(swPowerPin), powerInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(swRockPin), RockInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(swThermoUpPin), thermoUpInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(swThermoDownPin), thermoDownInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(swTimerUpPin), timerUpInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(swTimerDownPin), timerDownInterrupt, FALLING);

  timer.setInterval(1000, setTimer);     // 타이머 설정, 파라미터는 '시간(밀리초)', 'function이름'
  timer.run();                           // timer 시작

  display.setBrightness(7);              // 밝기 설정 범위는 0 ~ 7 
 
  Serial.begin(115200);                  // 시리얼 통신 설정
  Serial3.begin(115200);

  connectWifi();
  delay(500);
}

void loop() {
  if (powerFlag == false) {     // 전원이 off일 때 타이머, 온도조절 불가
    thermo = 0;
    setTime = 0;
  }
  digitalWrite(ledPowerPin, powerState);
  digitalWrite(ledRockPin, rockState);
    
  setTimer();
  weight();
  if (setTime == 0 && timerFlag == true) {    // 타이머가 작동중일 때 0으로 되면 온도 조절 off, 타이머 작동 off
    thermo = 0;
    timerFlag = false;
  }
  if (weightFlag == false) {    // 무게 감지가 안되면 온도조절 off
    thermo = 0;
    setTime = 0;
  }
  fndDataOut(fndData[thermo]);
  ledDataOut(thermo);
  
  if (powerFlag == false && sendFlag == true) {
    String str = String(sendTime);
    httpclient(str);   
  }
}

void fndDataOut(uint8_t data){    // fnd 출력
  for (i = 0; i < 7; i++) {
    digitalWrite(fndPin[i], (data >> i) & 0x01);
  }
}

void ledDataOut(uint8_t data) {   // led 출력
  for (i = 0; i < 8; i++) {
    if (i < data) {
      ledState = HIGH;
    }
    else if (i >= data) {
      ledState = LOW;
    }
    digitalWrite(ledThermoPin[i], ledState);
  }
}

void powerInterrupt() {   // power interrupt
  static uint32_t lastTime = 0;
  uint32_t now = millis();
  if ((now - lastTime) > debounceTime && rockFlag == false) {
    powerFlag = !powerFlag;
    powerState = !powerState;
    if (powerFlag == true) {
      onTime = millis();
      Serial.println(onTime);
      sendFlag = !sendFlag;
    }
    else {
      offTime = millis();
      sendTime = offTime - onTime;
      Serial.println(sendTime);
      onTime = 0;
      offTime = 0;
    }
  }
  lastTime = now;
}

void RockInterrupt() {    // 잠금 장치 함수
  static uint32_t lastTime = 0;
  uint32_t now = millis();
  
  if ((now - lastTime) > debounceTime && powerFlag == false) {
    rockFlag = !rockFlag;
    rockState = !rockState;
  }
  lastTime = now;  
}

void setTimer() {         // 타이머 설정 함수
  current_sec = setTime - ((millis()/1000) - pre_sec);
  if (current_sec <= period) {
    current_sec = 0;
    setTime = 0;
  }

  num1 =((current_sec / 60) / 10);   // 분. 십의 자리 출력
  num2 =((current_sec / 60) % 10);   // 분. 일의 자리 출력
  num3 =((current_sec % 60) / 10);   // 초. 십의 자리 출력
  num4 =(current_sec % 10);          // 초. 일의 자리 출력

  data[0] = display.encodeDigit(num1);    // 시간 출력
  data[1] = display.encodeDigit(num2);
  data[2] = display.encodeDigit(num3);
  data[3] = display.encodeDigit(num4);
    
  display.setSegments(data);      // 배열 출력
}

void weight() {     // 감압센서 무게 출력 함수
  int adcValue = analogRead(PressSensor);
  double voltageOut = (5.0 * (double)adcValue) / ((double)adcValue + 1000.0);
  //if (voltageOut >= 0) {   // 감압센서가 정상일 때
  if (voltageOut >= 0) {     // 감압센서가 망가져서 0으로 설정
    weightFlag = true;
  }
  else {
    weightFlag = false;
  }
}

void thermoUpInterrupt() {      // 온도 증가 함수
  static uint32_t lastTime = 0;
  uint32_t now = millis();
  
  if ((now - lastTime) > debounceTime && powerFlag == true && weightFlag == true) {
    thermo += 1;
    if (thermo > 8) {
      thermo = 8;
    }
  }
  lastTime = now;
}

void thermoDownInterrupt() {      // 온도 감소 함수
  static uint32_t lastTime = 0;
  uint32_t now = millis();
  
  if ((now - lastTime) > debounceTime && powerFlag == true && weightFlag == true) {
    thermo -= 1;
    if (thermo < 0) {
      thermo = 0;
    }
  }
  lastTime = now;
}

void timerUpInterrupt() {       // 타이머 시간 설정 증가 함수
  static uint32_t lastTime = 0;
  uint32_t now = millis();
  timerFlag = true;
  
  if ((now - lastTime) > debounceTime) {
    setTime += 30;
  }
  lastTime = now;  
  pre_sec = millis()/1000;
}

void timerDownInterrupt() {     // 타이머 시간 설정 감소 함수
  static uint32_t lastTime = 0;
  uint32_t now = millis();
  timerFlag = true;
  
  if ((now - lastTime) > debounceTime) {
    setTime -= 30;
    if (setTime < 0) {
      setTime = 0;
    }
  }
  lastTime = now;  
  pre_sec = millis()/1000;
}

void serialEvent() {
  unsigned char ch = Serial.read();
  if (ch == '0') {      // Power on
    if (rockFlag == false) {
      powerFlag = true;
      powerState = HIGH;      
    }
  }
  else if(ch == '1'){   // Power off
    powerFlag = false;
    powerState = LOW;      
  }
  else if(ch == '2'){   // Rock on
    if (powerFlag == false) {
      rockFlag = true;
      rockState = HIGH;      
    }
  }
  else if(ch == '3'){   // Rock off
    rockFlag = false;
    rockState = LOW;     
  }
  else if (ch == '4') { // Temperature up
    if (powerFlag == true && weightFlag == true) {
      thermo += 1;
      if (thermo > 8) {
        thermo = 8;
      }      
    }
  }
  else if(ch == '5'){   // Temperature down 
    if (powerFlag == true && weightFlag == true) {
      thermo -= 1;
      if (thermo < 0) {
        thermo = 0;
      }
    }
  }
  else if(ch == '6'){   // Timer up
    timerFlag = true;
    setTime += 30;
    pre_sec = millis()/1000;
  }
  else if(ch == '7'){   // Timer down
    timerFlag = true;
    setTime -= 30;
    if (setTime < 0) {
      setTime = 0;
    }
    pre_sec = millis()/1000;
  } 
  else if(ch == '8'){   // 
    //weightFlag = true;
  }
  else if(ch == '9'){   // 
    //weightFlag = false;
  }   
}

void connectWifi() {
  String join = "AT+CWJAP=\""+ssid+"\",\""+PASSWORD+"\"";
  
  Serial.println("Connect Wifi...");
  Serial3.println(join);
  delay(10000);
  
  if (Serial3.find("OK")) {
    Serial.print("WIFI connect\n");
  }
  else {
    Serial.println("connect timeout\n");
  }
  delay(1000);    
}

void httpclient(String char_input) {
  delay(100);
  Serial.println("connect TCP...");
  Serial3.println("AT+CIPSTART=\"TCP\",\""+host+"\",11540");
  delay(500);
  if (Serial.find("ERROR")) return;

  Serial.println("Send Data...");
  String url = char_input;
  String cmd = "GET /process.php?time="+url+"HTTP/1.0\r\n\r\n";
  Serial3.print("AT+CIPSEND=");
  Serial3.println(cmd.length());

  if (Serial3.find(">")) {
    Serial.print(">");
  }
  else {
    Serial3.println("AT+CIPCLOSE");
    Serial.println("connect timeout");
    delay(1000);
    return;
  }
  delay(500);

  Serial3.println(cmd);
  Serial.println(cmd);
  delay(100);
  if (Serial.find("ERROR")) return;
  Serial3.println("AT+CIPCLOSE");
  delay(100);
}
