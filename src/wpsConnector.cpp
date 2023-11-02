#include "wpsConnector.h"

namespace ztglib {
  static esp_wps_config_t config;

  void wpsInitConfig() {
    config.wps_type = ESP_WPS_MODE;
    strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
    strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
    strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
    strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
  }

  void wpsStart() {
    if (esp_wifi_wps_enable(&config)) {
      DEBUG_PRINTLN("WPS Enable Failed");
    }
    else if (esp_wifi_wps_start(0)) {
      DEBUG_PRINTLN("WPS Start Failed");
    }
  }

  void wpsStop() {
    if (esp_wifi_wps_disable()) {
      DEBUG_PRINTLN("WPS Disable Failed");
    }
  }

  String wpspin2string(uint8_t a[]) {
    char wps_pin[9];
    for (int i = 0; i < 8; i++) {
      wps_pin[i] = a[i];
    }
    wps_pin[8] = '\0';
    return (String)wps_pin;
  }

  void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info) {
    switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      DEBUG_PRINTLN("Station Mode Started");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DEBUG_PRINTLN("Connected to :" + String(WiFi.SSID()));
      DEBUG_PRINT("Got IP: ");
      DEBUG_PRINTLN(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DEBUG_PRINTLN("Disconnected from station, attempting reconnection");
      WiFi.reconnect();
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      DEBUG_PRINTLN("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      wpsStop();
      delay(10);
      WiFi.begin();
      break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
      DEBUG_PRINTLN("WPS Failed, retrying");
      wpsStop();
      wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      DEBUG_PRINTLN("WPS Timedout, retrying");
      wpsStop();
      wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_PIN:
      DEBUG_PRINTLN("WPS_PIN = " + wpspin2string(info.wps_er_pin.pin_code));
      break;
    default:
      break;
    }
  }

  void wpsConnect() {
    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);
    DEBUG_PRINTLN("Starting WPS");
    wpsInitConfig();
    wpsStart();
  }
}