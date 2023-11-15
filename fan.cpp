#include "config.h"
#include "mqtt.h"

class FanState {
  public:
    int st_speed;
    int st_heating;

    FanState():st_speed(0),st_heating(0) {

    }

    void set_speed(int new_speed) {
      bool dirty = this->st_speed != new_speed;
      bool onoff = ( this->st_speed * new_speed == 0 );
      this->st_speed = new_speed;

      if( dirty ) {
        this->percentage_state_notify();
        if( onoff )
          this->state_notify();
        if( this->st_speed == 0 )
          this->st_heating = 0;
      }

    }

    void percentage_state_notify() {
      sendPercentageState( this->st_speed == 0? off :
                           this->st_speed == 1? low :
                           this->st_speed == 2? high : unknown );
    }

    void state_notify() {
      sendState( this->st_speed != 0 );
    }

    void set_heating(int new_heating) {
      bool dirty = this->st_heating != new_heating ;
      this->st_heating = new_heating;
    }

    void dump() {
      Serial.print( this->st_speed );
      Serial.print( " " );
      Serial.print( this->st_heating );
      Serial.println( "" );
    }

    void fan_toggle() {
      int speed = this->st_speed +1;
      if(speed > 2) speed = 0;
      this->set_speed(speed);
    }

    void heat_toggle() {
      this->st_heating = (this->st_heating + 1)&1;
    }

};

FanState fanState;

void syncFanState() {
  fanState.state_notify();
  fanState.percentage_state_notify();
}

void state_set(int pin, bool enable) {
  digitalWrite( pin, enable?HIGH:LOW ); // high enable
}

void updateFanPower(bool on_off) {
  Serial.print( "updateFanPower:" );
  Serial.println( on_off );

  fanState.set_speed(on_off?1:0);
  fan_update();
}

void updateFanSpeed(FanSpeeds speed) {
  int v = 0;
  switch( speed) {
    case unknown:
      v = 1;
      break;
    case off:
      v = 0;
      break;
    case low:
      v = 1;
      break;
    case high:
      v = 2;
      break;
  }
  fanState.set_speed( v );
  fan_update();
}

void fan_toggle_speed() {
  fanState.fan_toggle();
  fan_update();

}

void fan_toggle_heat() {
  fanState.heat_toggle();
  state_set( PIN_HEATING, fanState.st_heating==1);
  fanState.dump();
}

void fan_update() {
  switch( fanState.st_speed ) {
    case 0:
      state_set(PIN_HEATING, false);    // shutdown heating!!!
      state_set(PIN_SPEED, false);
      state_set(PIN_ONOFF, false);
      break;
    case 1:
      // state_set(PIN_HEATING, false);
      state_set(PIN_SPEED, false);
      state_set(PIN_ONOFF, true);
      break;
    case 2:
      // state_set(PIN_HEATING, false);
      state_set(PIN_SPEED, true);
      state_set(PIN_ONOFF, true);
      break;
    default:
      break;
  }

  
  // sendPercentageState( speed );
  // if( st_speed == 0 || st_speed == 1) {
  //     // 启动停止，均延迟3s方可操作
  //     for( int i=1;i<=30;i++) {
  //       state_set(STARTUP_LED, i&1?true:false );
  //       delay(100);
  //     }
  // }
  fanState.dump();
}