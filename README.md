# smart_lock_key
SwitchBot Lock の鍵
## やること/できること
- SwitchBot Lock を M5Capsule (M5Stamp S3) で制御 (施錠/開錠) する
- ツインロック設定されていない複数の SwitchBot Lock を制御する
## 事前準備
- SwitchBot Lock が SwitchBot ハブと連携されており、スマホアプリから施錠/開錠できること
- API token/secret を取得する, 取得方法は以下
  1. スマホにアプリをインストール
  1. SwitchBotのアカウント登録し、スマホアプリにログインする
  1. スマホアプリ内の下にあるタブのプロフィール→設定 の順にタップ
  1. アプリバージョンを10回タップすると、開発者向けオプションが表示される
  1. 開発者向けオプションを選択
  1. token、secretが表示されるのでコピーする
- ソースコードに token/secret をセットする
  1. token/secret を記載した env.h を用意するか、src.ino 内の sb_token/sb_secret をセットする
- SwitchBot Lock の ID を取得する, 取得方法は以下
  1. ソースコードをビルドし、M5Capsuleに書き込む
  1. PC側でUSBシリアルコンソールを開く
  1. 電源投入し、WPSの設定を行う (WAKEボタン押下で起動後、WAKEボタン+S3オンボードボタン同時押し)
  1. 電源投入し、WAKEボタンを押す (WAKEボタン押下で起動後、もう一度WAKEボタンを押す)
  1. シリアルコンソールに検出された SwitchBot Lock の情報が表示されるので、deviceID の値をメモする
- 操作したい SwitchBot Lock の ID を登録する
  1. sb_lockdevid/sb_lockdevnum を記載した env.h を用意するか、src.ino 内の sb_lockdevid/sb_lockdevnum をセットする
- ソースコードをビルドし、M5Capsuleに書き込む

## ハードウェア
- M5Capsule (M5Stamp S3)
- M5Stamp S3の場合は何かしらの電源
## ソフトウェア
## ビルド
公開しているソースは Visual Studio Code + Arduino IDE ですが、Arduino IDE だけでビルドできるはずです。
1. 必要なライブラリのインストール
    - FastLED
    - [ButtonFever](https://github.com/mickey9801/ButtonFever)
    - [Seeed_Arduino_mbedtls](https://github.com/Seeed-Studio/Seeed_Arduino_mbedtls)
    - [UUID](https://github.com/RobTillaart/UUID)
    - [base64_encode](https://github.com/dojyorin/arduino_base64)

## 使い方
1. 電源投入 (WAKEボタン押下, LED:赤色点滅 0.1秒周期)
2. 電源投入後 
    - 開錠 (WAKEボタン2度押し, LED:緑色点灯)
      - WiFiにつながらなかった場合、電源OFF (LED : 消灯)
      - WiFiにつながった場合、開錠コマンドを送信後、電源OFF  (LED : 緑色点灯→消灯)
    - 施錠 (WAKEボタン長押し, LED:黄点灯)
      - WiFiにつながらなかった場合、電源OFF  (LED : 消灯)
      - WiFiにつながった場合、施錠コマンドを送信後、電源OFF  (LED : 橙色点灯→消灯)
    - WPSモード (WAKEボタン+S3オンボードボタン同時押し, LED:青色点滅 0.5秒周期)
      - 設定成功の時は LED 青点灯 1秒後、電源OFF
      - 設定失敗の時は LED 赤点灯 1秒後、電源OFF 
    - SwitchBot Lock のデバイスID表示 (WAKEボタン押下)
      - WiFiにつながらなかった場合、電源OFF (LED : 消灯)
      - WiFiにつながった場合、シリアルコンソールにデバイスIDを出力後、電源OFF  (LED : 水色点灯→消灯)

## 参考
- [APIでSwitchBotロックを遠隔操作する](https://qiita.com/masaki12-s/items/0295a96fd5a70442f7d5)
- [arduino-multi-button](https://github.com/poelstra/arduino-multi-button)
- [ButtonFever](https://github.com/mickey9801/ButtonFever)