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
  // int state = digitalRead(CONFIG_PIN_STATUS);  // get the current state of GPIO1 pin
  // digitalWrite(CONFIG_PIN_STATUS, !state);     // set pin to the opposite state
}
TickTwo ticker(tick,500);
// WiFiServer server(80);


OneButton button(PIN_BUTTON, true); // low enabled

hw_timer_t *watchdog_timer = NULL;    // watchdog timer



void click() {
  Serial.println("button pressed");
  fan_toggle_speed();
}



void longPressed() {
  fan_toggle_heat();
  
}

void watchDogInterrupt()
{
   Serial.println("time out");
   esp_restart();
}

  // char temp[] = R"({
  //  "device_class": "temperature",
  //  "name": "office_Temperature",
  //  "state_topic": "homeassistant/sensor/office_pc/state",
  //  "unit_of_measurement": "°C","value_template": "{{ value_json.temperature}}"})";

  // char Humidity[] = R"({
  //  "device_class": "humidity",
  //  "name": "office_Humidity",
  //  "state_topic": "homeassistant/sensor/office_pc/state",
  //  "unit_of_measurement": "%",
  //  "value_template": "{{ value_json.humidity}}"})";

// void reconnect() {
//   // Loop until we're reconnected
//   while (!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
//       if(client.connect("homeassistant/sensor/", "coolcall", "123123"))//clientID, userName, userPassword
//       {
//       client.publish("homeassistant/sensor/office_pc_H/config", Humidity);
//       client.publish("homeassistant/sensor/office_pc_T/config", temp);
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       // Wait 5 seconds before retrying
//       delay(5000);
//     }
//   }
// }

void setup_mqtt() {
    mqttClient.setServer(mqtt_server,1883);
    // client is now configured for use
    // if (!mqttClient.connected()) {
    //   reconnect();
    // } 
}

void setup_wifi()
{
    // We start by connecting to a WiFi network
    WiFi.begin(ssid, password);
    // int count = 0;
    // while (  WiFi.status() != WL_CONNECTED && count < 10) {
    //     delay(500);
    //     count++;
    //     Serial.print(".");
    // }

    if( WiFi.status() == WL_CONNECTED ) {
      // digitalWrite( WIFI_LED, HIGH );
      // server.begin();
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    
    }

}

//

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
  button.attachClick(click);

  button.setPressTicks(2000);      // 长按时间判定，800ms后算长按
  button.attachLongPressStart(longPressed);

  // setup watchdog
  watchdog_timer = timerBegin(1, 80, true);
  timerAttachInterrupt(watchdog_timer, watchDogInterrupt, true);
  timerAlarmWrite(watchdog_timer,10000*1000,true); // 10s触发 
  timerAlarmEnable(watchdog_timer);
}

void loop()
{
  timerWrite(watchdog_timer, 0);  // reset watchdog
  button.tick();
  // Serial.print(".");
  // if( st_speed == 2 )
  //   delay(6000); // this will trigger watchdog

  // if( network_ready == WL_CONNECTED )
    // loop_server();

  // char msg[100];
  // sprintf(msg,"{\"temperature\":%d, \"humidity\":%d}",(int)37,(int)80);//{"temperature":30, "humidity":40}
  // client.publish("homeassistant/sensor/office_pc/state", msg); 

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

  ticker.update();
  // updateFanSpeedFromSwitch();  
  // delay(10);
}
