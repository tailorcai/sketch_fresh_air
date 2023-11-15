#include <esp32-hal.h>
#include <OneButton.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "configure_wifi.h"
#include "mqtt.h"
#include "local_config.h"

char mqtt_server[40] = MQTT_SERVER; 
char mqtt_password[16] = MQTT_PASSWORD;
char mqtt_user[16] = MQTT_USER;
char mqtt_device[20] = MY_DEVICE_ID;
char mqtt_command_topic[40] = "living_room_fan/on/set";
char mqtt_state_topic[40] = "living_room_fan/on/state";
char mqtt_percentage_command_topic[40] = "living_room_fan/percentage/set";
char mqtt_percentage_state_topic[40] = "living_room_fan/percentage/state";

const char* ssid     = SSID;
const char* password = SSID_PASSWORD;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void tick() {
  //toggle state
  int state = digitalRead(STARTUP_LED);  // get the current state of GPIO1 pin
  digitalWrite(STARTUP_LED, !state);     // set pin to the opposite state
}
class LEDIndicator {
  private:
    bool op_pending;
    TickTwo ticker;
    elapsedSeconds peroid;

  public:
    LEDIndicator():op_pending(false), 
                  ticker(tick,1000,0,MILLIS) {
                    ticker.start();
                  }
    void set_pending() {
      if( !this->op_pending ) {
          ticker.interval(100);
          this->op_pending = true;
          this->peroid = 0;
      }
    }
    void loop() {
      this->ticker.update();

      if( this->op_pending ) {
        if( this->peroid > 5 ) {
          ticker.interval(1000);
          this->op_pending = false;
        }
      }
    }
} indicator;
// TickTwo ticker(tick,1000, 0, MILLIS);

void led_set_pending() {
  indicator.set_pending();
}

OneButton button(PIN_BUTTON, true); // low enabled
hw_timer_t *watchdog_timer = NULL;    // watchdog timer

void button_click() {
  Serial.println("button pressed");
  fan_toggle_speed();
}

void button_longPressed() {
  Serial.println("button long_pressed");
  fan_toggle_heat();  
}

void watchDogInterrupt()
{
   Serial.println("time out");
   esp_restart();   // ESP only
}

void setup_mqtt() {
    mqttClient.setServer(mqtt_server,1883);
}

void setup_wifi()
{
    // We start by connecting to a WiFi network
    WiFi.begin(ssid, password);
    delay(100);

    if( WiFi.status() == WL_CONNECTED ) {
      // digitalWrite( WIFI_LED, HIGH );
      // server.begin();
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
}

void setup()
{
  Serial.begin(115200);
  delay(100); // give me time to bring up serial monitor

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_ONOFF, OUTPUT);
  pinMode(PIN_HEATING, OUTPUT);
  pinMode(PIN_SPEED, OUTPUT);
  pinMode( WIFI_LED,OUTPUT);
  pinMode( STARTUP_LED, OUTPUT);
  digitalWrite( STARTUP_LED, HIGH);

  setup_wifi();
  setup_mqtt();
  
  Serial.println("Starting");
  button.reset();//清除一下按钮状态机的状态
  button.setDebounceTicks(50);  // 设置按键消抖时长
  button.setClickTicks(500);        // 设置点击时长
  button.attachClick(button_click);

  button.setPressTicks(2000);      // 长按时间判定，800ms后算长按
  button.attachLongPressStart(button_longPressed);

  // setup watchdog
  watchdog_timer = timerBegin(1, 80, true);
  timerAttachInterrupt(watchdog_timer, watchDogInterrupt, true);
  timerAlarmWrite(watchdog_timer,10000*1000,true); // 10s触发 
  timerAlarmEnable(watchdog_timer);

  // ticker.start();
}

void loop()
{
  timerWrite(watchdog_timer, 0);  // reset watchdog
  button.tick();

  static bool just_connected = false;
  // network maintainness
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  } else {
    if (!mqttClient.connected()) {
      just_connected = true;
      reconnectMqtt();
    }
    if (mqttClient.connected()) {
      if( just_connected ) {
        just_connected = false;
        syncFanState();           // update state 
      }
      mqttClient.loop();
    }
  }

  // ticker.update();
  indicator.loop();
}
