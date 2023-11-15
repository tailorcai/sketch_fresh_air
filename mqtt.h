#ifndef _mqtt_h
#define _mqtt_h

extern void processMessage(char* topic, byte* payload, unsigned int length);
extern void reconnectMqtt();
extern void sendPercentageState(FanSpeeds);
extern void sendState(bool);
// extern void sendPowerState(boolean enabled);
// extern void sendOscState(boolean enabled);
// extern void sendSpeedState(FanSpeeds speed);

#endif _mqtt_h