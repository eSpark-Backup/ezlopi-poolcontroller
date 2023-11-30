#ifndef PACKET_H_millisbyt
#define PACKET_H_

#include "clock.h"

// command types
#define COMMAND_SYSTEM_STATUS     0x02
#define COMMAND_TIME_SETTINGS     0x05
#define COMMAND_PUMP_CONTROL      0x06
#define COMMAND_PUMP_STATUS       0x07
#define COMMAND_HEAT_STATUS       0x08
#define COMMAND_CHLORINATOR       0x50
#define COMMAND_SCHEDULE_CONFIG   0x11

// Device Addresses/IDs
typedef enum 
{
 DEVICE_CONTROL_CENTER   =  0x10,  // IntelliTouch Control Center
 DEVICE_BROADCAST        =  0x0f,
 DEVICE_WIRED_REMOTE     =  0x20,
 DEVICE_WIRELESS_REMOTE  =  0x22,  // e.g. Screen Logic
 DEVICE_REMOTE           =  0x48,  // QuickTouch remote?
 DEVICE_PUMP1            =  0x60,
 DEVICE_PUMP2            =  0x61,
 DEVICE_CHLORINATOR      =  0x70 , // Not in the protocol. Assigned by me to make packets searchable
 DEVICE_90              =   0x90
} DeviceID;

//Command Ids for remote 
typedef enum
{
  POOL_ON        = 'a',
  POOL_OFF       = 'b',
  POOL_LIGHT_ON  = 'c',
  POOL_LIGHT_OFF = 'd',
  CLEANER_ON     = 'e',
  CLEANER_OFF    = 'f',
  WATER_FALL_ON  = 'g',
  WATER_FALL_OFF = 'h',
  SALT_PCT_QUERY = 'i',
  CHLOR_STEP_1   = 'j',
  CHLOR_STEP_2   = 'k',
  CHLOR_STEP_3   = 'l',
  CHLOR_STEP_4   = 'm',
  CHLOR_STEP_5   = 'n',
  CHLOR_STEP_6   = 'o',
  CHLOR_STEP_7   = 'p',
  CHLOR_STEP_8   = 'q',
} CIDNum;

const char* DeviceName(uint8_t device);

// Control Center's current mode
#define SYSTEM_MODE_AUTO          0x00
#define SYSTEM_MODE_SERVICE       0x01
#define SYSTEM_MODE_TIMEOUT       0x81

#define HEAT_SOURCE_OFF           0x00
#define HEAT_SOURCE_HEATER        0x01
#define HEAT_SOURCE_SOLAR_PREF    0x02
#define HEAT_SOURCE_SOLAR         0x03

const char* HeatSourceName(uint8_t heat_setting);

// struct Command {  // @suppress("Class has a virtual method and non-virtual destructor")
//   virtual void Read(const uint8_t *data) {
//   }
//   virtual void PrintTo(Print &printer) {
//   }
//   virtual const char* Name() {
//     return "Unknown";
//   }
// };

struct Command 
{
  virtual void Read(const uint8_t *data) {
  }
  virtual void PrintTo() 
  {
    printf("Command Information:\n");
    printf("Name: %s\n", Name());
  }
  virtual const char* Name() {
    return "Unknown";
  }
};

// extracted from packet data for easy touch status
struct SystemStatus : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  uint8_t hour_;
  uint8_t minutes_;
  bool circuits_[8];
  uint8_t mode_;
  bool celcius_;
  bool solar_;
  uint8_t air_temp_;
  uint8_t pool_temp_;
  uint8_t spa_temp_;
  uint8_t solar_temp_;
  uint8_t pool_heat_source_;
  uint8_t spa_heat_source_;
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

struct ChlorinatorRequest : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  bool mode_;      // on/off
  uint8_t percent_;   // percent chlorination requested by the controller
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};


#define INSPECT_CELL_MASK    0x10

struct ChlorinatorResponse : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  uint16_t salinity_;  // salinity in ppm measure by the cell
  uint8_t inspect_cell_; // flashing green status light - inspect cell
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

#define DAY_OF_WEEK_SUNDAY    0x01
#define DAY_OF_WEEK_MONDAY    0x02
#define DAY_OF_WEEK_TUESDAY   0x04
#define DAY_OF_WEEK_WEDNESDAY 0x08
#define DAY_OF_WEEK_THURSDAY  0x10
#define DAY_OF_WEEK_FRIDAY    0x20
#define DAY_OF_WEEK_SATURDAY  0x40

struct TimeSettings : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  uint8_t hour_;
  uint8_t minute_;
  uint8_t week_day_;
  uint8_t day_;
  uint8_t month_;
  uint8_t year_;
  int clock_accuracy_offset_;  // in seconds, -300 to +300
  bool auto_adjust_dst_;
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

struct PumpControlRequest : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  // extracted from packet data for intelliflow pump status
  bool running_;
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

struct PumpControlResponse : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  // extracted from packet data for intelliflow pump status
  bool running_;
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

struct PumpStatusRequest : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  // extracted from packet data for intelliflow pump status
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

struct PumpStatusResponse : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  // extracted from packet data for intelliflow pump status
  bool running_;
  uint16_t watts_;
  uint16_t rpms_;
  uint8_t hour_;
  uint8_t minutes_;
  bool priming_;
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

struct HeatStatus : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  uint8_t pool_temp_;
  uint8_t spa_temp_;
  uint8_t air_temp_;
  uint8_t solar_temp_;
  uint8_t pool_temp_target_;
  uint8_t pool_heat_source_;
  uint8_t spa_temp_target_;
  uint8_t spa_heat_source_;
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

struct ScheduleConfig : Command {  // @suppress("Class has a virtual method and non-virtual destructor")
  uint8_t circuit_;
  uint8_t start_hour_;
  uint8_t start_minute_;
  uint8_t stop_hour_;
  uint8_t stop_minute_;
  uint8_t active_days_;
  const char* Name();
  void Read(const uint8_t *data);
  void PrintTo();
};

class Packet {
 private:
  // packet parsing state
  enum State {
    preamble_state,
    version_state,
    destination_state,
    source_state,
    command_state,
    length_state,
    data_state,
    checksum_state,
    intellichlor_state,
  };

  //switch to next state and update the running checksum with what we just read.
  void nextState(State s, uint16_t check) {
    state_ = s;
    pos_ = 0;
    checksum_ += check;
  }

  bool valid_;
  State state_;
  uint8_t pos_;  // position within a state.

  // keep track of time, so we can print it in protocol dumps.
  //
  // since we have no clock, we are going to use the time provided by the control center in the first status
  // broadcast we see. we use the on board millis() tracker to keep track of elapsed time from that point.
  Clock clock_;

 public:
  // packet data
  uint8_t version_;
  uint8_t target_;
  uint8_t source_;
  uint8_t cid_;
  Command *command_;
  uint8_t data_len_;
  uint8_t *data_;
  uint8_t buffer_[256];
  uint8_t buffer_pos_;
  uint8_t checksum_data_[2];
  uint16_t checksum_;  // running checksum computed as bytes are read

  // Keep one instance of each command so we never have to allocate one.
  Command unknown;
  SystemStatus system_status;
  PumpStatusRequest pump_status_request;
  PumpStatusResponse pump_status_response;
  PumpControlRequest pump_control_request;
  PumpControlResponse pump_control_response;
  ScheduleConfig schedule_config;
  TimeSettings time_settings;
  HeatStatus heat_status;
  ChlorinatorRequest chlorinator_request;
  ChlorinatorResponse chlorinator_response;

  Packet() {
    Reset();
  }

  void Reset();

  // Read a fully formatted packet buffer. Must contain preamble, header and checksum.
  bool ReadFrom(const char *hex);
  // Read a packet from its command id and data in hex form. packet is from remote to control center. checksum is added automatically.
  bool Make(uint8_t cid, const char* data);
  bool PushHex(const char *hex);
  bool Push(uint8_t c);

  void ReadCommand();
  // Write packet as a protocol frame.
  void WriteTo();
  // Print a human readable version, for debugging.
  void PrintTo();
  void PrintHeader();

  bool IsValid() {
    return valid_;
  }
};

uint8_t htoi(char c);
uint16_t toUint16(uint8_t hi, uint8_t lo);

void PrintLabel(const char *l);
void PrintLabel( const char *l, int pad);
void PrintHex( uint8_t c);
void PrintTime(uint8_t h, uint8_t m);
void PrintDate( uint8_t day_, uint8_t month_, uint8_t year_);
void PrintClock(Clock &c);

#endif // PACKET_H_
