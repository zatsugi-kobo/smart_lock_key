#include "SBConnector.h"

namespace ztglib {
  /// @brief コンストラクタ, WiFiに接続し、時刻設定を行う
  /// @param t トークン
  /// @param s シークレット
  SBConnector::SBConnector(const char *t, const char *s)
  {
    int to_cnt = 0;
    UUID uuid;

    WiFi.disconnect(); 
    WiFi.begin();
    while (WiFi.status() != WL_CONNECTED) {
      delay(100); // 500ms毎に.を表示
      DEBUG_PRINT(".");
      to_cnt++;
      if (to_cnt > 10000) {
        DEBUG_PRINTLN("wifi connection timeout.");
        token = "";
        secret = "";
        return;
      }
    }
    DEBUG_PRINTLN("connect.");

    snprintf(nonce, 64, "%s", uuid.toCharArray());
    token = t;
    secret = s;
    #ifdef USE_ROOT_CA
    sb_root_ca = SB_ROOT_CA;
    #endif

    configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
    https.setReuse(true);
  }

  SBConnector::~SBConnector()
  {
    WiFi.disconnect(); 
  }

  int SBConnector::hmac_sha256(const char *p_key, const char *p_payload, unsigned char *result)
  {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)p_key, strlen(p_key));
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)p_payload, strlen(p_payload));
    mbedtls_md_hmac_finish(&ctx, (unsigned char *)result); // 32 bytes
    mbedtls_md_free(&ctx);

    return 0;
  }

  int SBConnector::make_request_header()
  {
    if (token == NULL || secret == NULL) {
      DEBUG_PRINTLN("Illeagal token/secret.");
      return -1;
    }
    memset(ti, 0, 16);
    memset(sign, 0, 64);
    memset(tmpBuf, 0, 256);

    struct tm timeInfo;
    getLocalTime(&timeInfo);
    sprintf(ti, "%010d000", mktime(&timeInfo));

    int tokenLen = token.length()+1;
    char tok[tokenLen + 1];
    token.toCharArray(tok, tokenLen);
    snprintf(tmpBuf, 256, "%s%s%s", tok, ti, nonce);

    unsigned char hash[32];
    int secLength = secret.length() + 1;
    char sec[secLength+1];
    secret.toCharArray(sec, secLength);
    hmac_sha256(sec, tmpBuf, hash);

    size_t hashLength = sizeof(hash);
    //char sign[base64::encodeLength(hashLength)];
    base64::encode(hash, hashLength, sign);

    return 0;
  }

  int SBConnector::get_device_list()
  {
    int ret = 0;
    if (WiFi.status() != WL_CONNECTED) return -1;

    if (make_request_header() < 0) {
      DEBUG_PRINTLN("header error.");
      return -1;
    }
    DEBUG_PRINT("Authorization: "); DEBUG_PRINTLN(token);
    DEBUG_PRINT("time: "); DEBUG_PRINTLN(ti);
    DEBUG_PRINT("sign: "); DEBUG_PRINTLN(sign);
    DEBUG_PRINT("nonce: "); DEBUG_PRINTLN(nonce);

#ifdef USE_ROOT_CA
    if (https.begin(SB_DEVLIST_URL, sb_root_ca))
#else
    if (https.begin(SB_DEVLIST_URL))
#endif
    {
      https.addHeader("Content-Type", "application/json");
      https.addHeader("Authorization", token);
      https.addHeader("t", ti);
      https.addHeader("sign", sign);
      https.addHeader("nonce", nonce);
      int httpResponseCode = https.GET();
      DEBUG_PRINT("Response code: ");
      DEBUG_PRINTLN(httpResponseCode);
      if (httpResponseCode > 0) {
        devList = https.getString();
        //DEBUG_PRINTLN(devList);
      } else {
        DEBUG_PRINTLN("https.GET() error.");
        ret = -1;
      }
    } else {
      DEBUG_PRINTLN("https error.");
      ret = -1;
    }
    https.end();

    return ret;
  }
  
  int SBConnector::get_device(String str, const char *devtype, int pos)
  {
    int idx = str.indexOf(devtype, pos);
    if (idx < 0) return -1;

    int sp = str.lastIndexOf('{', idx);
    int ep = str.indexOf('}', idx);

    if (sp < 0 || ep < 0) return -1;
    DEBUG_PRINTLN(str.substring(sp, ep+1));
    return ep;
  }

  int SBConnector::get_device_list(const char *devtype)
  {
    int ret = get_device_list();
    if (ret == 0) {
      int idx = 0;
      while (idx != -1) {
        idx = get_device(devList, devtype, idx);
      }
      DEBUG_PRINTLN("done.");
    }
  }

  /// @brief idで指定されたデバイスにコマンドを送信する
  /// @param id 操作対象デバイスのIDリスト
  /// @param cmd デバイスに送信するコマンド(JSON)
  /// @return 0:成功, 0以外:失敗
  int SBConnector::send_cmd(const char *id[], const int num, const char *cmd)
  {
    int ret = 0;
    if (WiFi.status() != WL_CONNECTED) return -1;

    if (make_request_header() < 0) {
      DEBUG_PRINTLN("header error.");
      return -1;
    }  
    DEBUG_PRINT("Authorization: "); DEBUG_PRINTLN(token);
    DEBUG_PRINT("time: "); DEBUG_PRINTLN(ti);
    DEBUG_PRINT("sign: "); DEBUG_PRINTLN(sign);
    DEBUG_PRINT("nonce: "); DEBUG_PRINTLN(nonce);

    // idリスト分繰り返す
    for (int i=0; i<num; i++) {
      char cmd_url[128];
      snprintf(cmd_url, 128, "%s/%s/commands", SB_DEVLIST_URL, id[i]);

#ifdef USE_ROOT_CA
      if (https.begin(cmd_url, sb_root_ca))
#else
      if (https.begin(cmd_url))
#endif
      {
        https.addHeader("Content-Type", "application/json");
        https.addHeader("Authorization", token);
        https.addHeader("t", ti);
        https.addHeader("sign", sign);
        https.addHeader("nonce", nonce);
        int httpResponseCode = https.POST(cmd);
        DEBUG_PRINT("Response code: ");
        DEBUG_PRINTLN(httpResponseCode);
        if (httpResponseCode > 0) {
          String ret_msg = https.getString();
          DEBUG_PRINTLN(ret_msg);
        } else {
          DEBUG_PRINTLN("https.POST() error.");
          ret = -1;
        }
      } else {
        DEBUG_PRINTLN("https error.");
        ret = -1;
      }
      https.end();
    }

    return ret;
  }

  /// @brief SwitchBot Lock の施錠をおこなう
  /// @param id SwitchBot LockデバイスのIDリスト
  /// @return 0:成功, 0以外:失敗
  int SBConnector::lock(const char *id[], const int num)
  {
    int ret = send_cmd(id, num, SB_LOCKCMD);
    return ret;
  }

  /// @brief SwitchBot Lock の開錠をおこなう
  /// @param id SwitchBot LockデバイスのIDリスト
  /// @return 0:成功, 0以外:失敗
  int SBConnector::unlock(const char *id[], const int num)
  {
    int ret = send_cmd(id, num, SB_UNLOCKCMD);
    return ret;
  }
}