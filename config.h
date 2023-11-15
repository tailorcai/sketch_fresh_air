#ifndef _config_h
#define _config_h

#include <TickTwo.h>
#include <PubSubClient.h>

// Pins
// for ESP32-WROOM 普中版本
//=====================
#define PIN_BUTTON 23

const int PIN_ONOFF = 26; // 需要加偏置电阻，确保启动时的电平
const int PIN_HEATING = 33;
const int PIN_SPEED = 25; 
const int WIFI_LED = 22;
const int STARTUP_LED = 2;

// ESP32C3, 目前使用下面的pin
//======================
// #define PIN_BUTTON 8
// #define PIN_ONOFF 18    // 兼容板载LED
// #define PIN_HEATING 00
// #define PIN_SPEED 01    // 兼容板载LED
// #define WIFI_LED 12
// #define STARTUP_LED 13

// What value to set to turn the config LED on/off.
#define CONFIG_LED_ON LOW
#define CONFIG_LED_OFF HIGH

enum FanSpeeds {unknown, off, low, high};

extern TickTwo ticker;
extern void tick();

extern char mqtt_server[];
extern char mqtt_port[];
extern char mqtt_user[];
extern char mqtt_password[];
extern char mqtt_command_topic[];
extern char mqtt_state_topic[];
extern char mqtt_percentage_command_topic[];
extern char mqtt_percentage_state_topic[];
extern char mqtt_device[];

extern PubSubClient mqttClient;

extern void updateFanPower(boolean enabled);
extern void updateFanSpeed(FanSpeeds newSpeed);
// extern void updateFanOsc(boolean enabled);

// extern FanSpeeds readSpeedSwitch();
// extern void debugSpeed(FanSpeeds value);
// extern void updateFanSpeedFromSwitch();

extern void fan_toggle_speed();
extern void fan_toggle_heat();
extern void fan_update();

extern void syncFanState();

#endif _config_h
