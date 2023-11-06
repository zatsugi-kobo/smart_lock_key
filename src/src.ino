#include "WiFi.h"
#include "wpsConnector.h"
#include "FastLED.h" 
#include "WiFiClientSecure.h"
#include "WiFiUDP.h"
#include "BfButton.h"
#include "SBConnector.h"

#define USE_ENVFILE
#ifdef USE_ENVFILE
#include "env.h"
#else
const char *sb_token = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char *sb_secret "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char *sb_lockdevid[] = {"xxxxxxxxxxxxxxx", "xxxxxxxxxxxxxxx"};
const int sb_lockdevnum = 2;
#endif

#define DEBUG_PRINTBEGIN(x) USBSerial.begin(x)
#define DEBUG_PRINTLN(x) USBSerial.println(x)
#define DEBUG_PRINT(x) USBSerial.print(x)

#define ONBSW_PIN 42    // M5Capsule オンボードスイッチ(GPIO42)
#define S3LED_PIN 21    // M5Stamp S3 オンボードLED(GPIO21)
#define S3SW_PIN 0      // M5Stamp S3 オンボードスイッチ(GPIO0)
#define NUM_LEDS 1
#define PWRCTL_PIN 46   // 電源制御
#define TIMEOUT 5000    // 電源ONにした後、再度スリープするまでの時間(ms)

#define LED_OFF 0
#define LED_PWRON 1
#define LED_WPS 2
#define LED_OPEN 3
#define LED_CLOSE 4
#define LED_GETDEVID 5
CRGB leds[NUM_LEDS];
CRGB colors[] = {
  CRGB(0, 0, 0),
  CRGB(125, 0, 0),      // 起動
  CRGB(0, 0, 150),      // WPS 
  CRGB(0, 125, 0),      // 開錠
  CRGB(125, 125, 0),    // 施錠
  CRGB(0, 125, 125)     // SwitchBot Lock のデバイスID取得
  }; 
BfButton btn(BfButton::STANDALONE_DIGITAL, ONBSW_PIN, true, LOW); 
unsigned long timeout;

void setup()
{
  DEBUG_PRINTBEGIN(115200);
  FastLED.addLeds<WS2812B, S3LED_PIN, GRB>(leds, NUM_LEDS); 

  pinMode(PWRCTL_PIN, OUTPUT);
  digitalWrite(PWRCTL_PIN, HIGH);   // 電源ON状態を保持
  pinMode(S3SW_PIN, INPUT); 

  while(digitalRead(ONBSW_PIN)==LOW);

  btn.onPress(pressHandler)
    .onDoublePress(pressHandler)     // default timeout
    .onPressFor(pressHandler, 2000); // custom timeout for 2 second
  timeout = millis();
}

void wps_setup()
{
  unsigned int count = 0;

  WiFi.disconnect();   // 一度WiFiを切断してwpsConnect()を使う
  ztglib::wpsConnect();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // 500ms毎に.を表示
    DEBUG_PRINT(".");

    if (count % 2 == 0) {
      leds[0] = colors[LED_WPS];
    } else {
      leds[0] = colors[LED_OFF];
    }
    FastLED.show();

    count++;
    if (count > 60) {
      DEBUG_PRINTLN("timeout.");
      leds[0] = colors[LED_PWRON];
      FastLED.show();
      delay(2000);
      leds[0] = colors[LED_OFF];
      FastLED.show();
    }
  }
  // 接続成功
  DEBUG_PRINTLN("connect.");
  leds[0] = colors[LED_WPS];
  FastLED.show();
  delay(2000);
  leds[0] = colors[LED_OFF];
  FastLED.show();
}

void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) 
{
  int ret;
  leds[0] = colors[LED_OFF];
  FastLED.show();
  ztglib::SBConnector sb(sb_token, sb_secret);
  disableCore0WDT();  // レスポンス待ちでCore 0 のWDTが発動しないように一旦止める

  //DEBUG_PRINT(btn->getID());
  switch (pattern)  {
    case BfButton::SINGLE_PRESS:
      DEBUG_PRINTLN("pressed.");    
      if (digitalRead(S3SW_PIN) == LOW) {
        // wps モードに入る
        wps_setup();
      } else {
        // SwitchBotデバイスのリストを取得
        leds[0] = colors[LED_GETDEVID];
        FastLED.show();
        ret = sb.get_device_list("Smart Lock");
        if (ret < 0) {
          DEBUG_PRINTLN("get_device_list() error.");
        }
      }
      break;
    case BfButton::DOUBLE_PRESS:
      // unlock
      DEBUG_PRINTLN("double pressed.");
      leds[0] = colors[LED_OPEN];
      FastLED.show();
      sb.unlock(sb_lockdevid, sb_lockdevnum);
      break;
    case BfButton::LONG_PRESS:
      // lock
      DEBUG_PRINTLN("long pressed.");
      leds[0] = colors[LED_CLOSE];
      FastLED.show();
      sb.lock(sb_lockdevid, sb_lockdevnum);
      break;
  }

  enableCore0WDT();   // WDT有効化
  delay(1000);
  digitalWrite(PWRCTL_PIN, LOW);   // 電源OFF
}

void loop()
{
  unsigned long ms = millis();
  btn.read();

  // Lチカ
  if (int(ms / 100) % 2 == 0) {
    leds[0] = colors[LED_PWRON];
  } else {
    leds[0] = colors[LED_OFF];
  }
  FastLED.show();

  if (ms - timeout > TIMEOUT) {
    digitalWrite(PWRCTL_PIN, LOW);   // 電源OFF
  }

  delay(1);   // CPUリソースをリリース
}
