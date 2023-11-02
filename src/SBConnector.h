#pragma once
#include <Arduino.h>
#include <Time.h>
#include "WiFi.h"
#include "WiFiClientSecure.h"
#include "mbedtls/md.h" 
#include "UUID.h"
#include "arduino_base64.hpp"
#include "HTTPClient.h"

// https://github.com/OpenWonderLabs/SwitchBotAPI
// https://qiita.com/masaki12-s/items/0295a96fd5a70442f7d5

namespace ztglib {
  #define DEBUG_PRINTLN(x) USBSerial.println(x)
  #define DEBUG_PRINT(x) USBSerial.print(x)
  #define SB_URL "https://api.switch-bot.com"
  #define SB_DEVLIST_URL SB_URL "/v1.1/devices"
  #define SB_DEVTYPE "Smart Lock"
  #define SB_LOCKCMD "{\"commandType\":\"command\",\"command\":\"lock\",\"parameter\":\"default\"}"
  #define SB_UNLOCKCMD "{\"commandType\":\"command\",\"command\":\"unlock\",\"parameter\":\"default\"}"
  #ifdef USE_ROOT_CA
  #define SB_ROOT_CA \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
    "-----END CERTIFICATE-----\n"
  #endif

  class SBConnector {
  public:
    SBConnector(const char *, const char *);
    ~SBConnector();
    int lock(const char *[], const int);
    int unlock(const char *[], const int);
    int get_device_list(const char *);

  private:
    HTTPClient https;       // HTTPコネクション
    String token;      // Switch Bot token
    String secret;     // Switch Bot secret
    char ti[16];      // header 用 timeInfo 文字列
    char sign[64];    // header 用 sign 文字列
    char nonce[64];   // header 用 nonce 文字列
    char tmpBuf[256]; // 汎用バッファ
    //const char *sb_root_ca;
    String devList;   // get_device_list()で利用するデバイスリスト文字列(JSONでリストが格納される)

    int hmac_sha256(const char *p_key, const char *p_payload, unsigned char *result);
    int make_request_header();
    int send_cmd(const char *[], const int, const char *);
    int get_device_list();    // SwitchBotデバイスの一覧取得
    int get_device(String, const char *, int);
  };
}