#include <ArduinoJson.h>
#include <elapsedMillis.h>

#include "config.h"
#include "mqtt.h"

// If disconnected, how often to try to reconnect
#define RECONNECT_POLLING_PERIOD 5000

#define ON_MESSAGE "ON"
#define OFF_MESSAGE "OFF"
#define OFF_SPEED "0"
#define LOW_SPEED "1"
// #define MED_SPEED "medium"
#define HIGH_SPEED "2"

elapsedMillis reconnectTimeElapsed;

void processMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (strlen(mqtt_command_topic) && !strcmp(topic, mqtt_command_topic)) {
    boolean powerEnabled = !strcmp(ON_MESSAGE, message);
    updateFanPower(powerEnabled);
  // } else if (strlen(mqtt_percentage_command_topic) && !strcmp(topic, mqtt_percentage_command_topic)) {
  //   boolean powerEnabled = !strcmp(ON_MESSAGE, message);
  //   updateFanOsc(powerEnabled);    
  } else if (strlen(mqtt_percentage_command_topic) && !strcmp(topic, mqtt_percentage_command_topic)) {
    FanSpeeds newSpeed = unknown;
    if (!strcmp(OFF_SPEED, message)) {
      // turn off
      updateFanPower(false);
    } else if (!strcmp(LOW_SPEED, message)) {
      updateFanSpeed(low);
    // } else if (!strcmp(MED_SPEED, message)) {
    //   updateFanSpeed(medium);
    } else if (!strcmp(HIGH_SPEED, message)) {
      updateFanSpeed(high);
    } else {
      Serial.print("Error: Unknown fan speed: ");
      Serial.println(message);
    }
  } else {
    Serial.println("Error: Unrecognised message");
  }
}

void subscribeToTopic(const char *topicName, const char *topic) {
  if (!strlen(topic)) {
    Serial.print("Error: Topic for ");
    Serial.print(topicName);
    Serial.println(" not configured");
  } else {
    Serial.print("Subscribe to: ");
    Serial.println(topic);
    mqttClient.subscribe(topic);          
  }
}

void reconnectMqtt() {
  if (!mqttClient.connected()) {
    if (reconnectTimeElapsed > RECONNECT_POLLING_PERIOD) {
      reconnectTimeElapsed = 0;
      Serial.print("Attempting MQTT connection...");
      // ticker.attach(0.05, tick);
      // Attempt to connect
      if (mqttClient.connect(mqtt_device, mqtt_user, mqtt_password)) {
        Serial.println("connected");
        mqttClient.setCallback(processMessage);
        subscribeToTopic("mqtt_power_set_topic", mqtt_percentage_command_topic);
        subscribeToTopic("mqtt_speed_set_topic", mqtt_command_topic);
        // subscribeToTopic("mqtt_osc_set_topic", mqtt_osc_set_topic);        
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
      }
      // ticker.detach();
      // digitalWrite(CONFIG_PIN_STATUS, CONFIG_LED_OFF);
    }
  }
}

void publishWithDebug(const char *topic, const char *message) {
  if( mqttClient.connected() ) {
    mqttClient.publish(topic, message, true);
    Serial.print("Publish topic=");
    Serial.print(topic);
    Serial.print(" message=");
    Serial.println(message);
  }
}

void sendState(bool on_off) {
  publishWithDebug( mqtt_state_topic, on_off? "ON":"OFF" );
}
void sendPercentageState( FanSpeeds speed) {
  const char * msg = "1";
  switch( speed) {
    case unknown:
      break;
    case off:
      msg = "0";
    case low:
      break;
    case high:
      msg = "2";
      break;
  }
  publishWithDebug( mqtt_percentage_state_topic, msg );
}
/*
void sendPowerState(boolean enabled) {
  publishWithDebug(mqtt_power_state_topic, enabled ? ON_MESSAGE : OFF_MESSAGE);
  if (!enabled) {
    sendSpeedState(off);
  }
}

void sendOscState(boolean enabled) {
  publishWithDebug(mqtt_osc_state_topic, enabled ? ON_MESSAGE : OFF_MESSAGE);
}

void sendSpeedState(FanSpeeds speed) {
  char value[16];
  switch (speed) {
    case unknown:
      return;
    case off:
      strcpy(value, OFF_SPEED);
      break;
    case low:
      strcpy(value, LOW_SPEED);
      break;
    // case medium:
    //   strcpy(value, MED_SPEED);
    //   break;
    case high:
      strcpy(value, HIGH_SPEED);
      break;
    default:
      return;
  }
  
  publishWithDebug(mqtt_speed_state_topic, value);    
}*/

