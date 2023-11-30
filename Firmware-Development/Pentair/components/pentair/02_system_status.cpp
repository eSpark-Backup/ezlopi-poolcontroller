#include "packet.h"

// Define bytes from a intellitouch touch message.
#define HOUR 0
#define MINUTES 1
#define CIRCUITS 2
#define MODE 9
#define SOLAR 10
#define POOL_TEMP 14
#define SPA_TEMP 15
#define AIR_TEMP 18
#define SOLAR_TEMP 19
#define HEAT_SETTINGS 22

#define CIRCUITS_COUNT 8

static const char *CIRCUITS_NAMES[CIRCUITS_COUNT] = { "SPA", "CLEANER", "??",
    "??", "??", "POOL", "??", "??" };

const char* SystemModeName(uint8_t mode) 
{
  switch (mode) {
    case SYSTEM_MODE_AUTO:
      return "Auto";
    case SYSTEM_MODE_SERVICE:
      return "Service";
    case SYSTEM_MODE_TIMEOUT:
      return "Timeout";
  }
  return "Unknown";
}

const char* HeatSourceName(uint8_t settings) 
{
  const char *NAMES[] = { "Off", "Heater", "Solar Pref", "Solar" };
  if (settings < sizeof(NAMES)) 
  {
    return NAMES[settings];
  }
  return "Unknown";
}

const char* tempUnit(int celcius) 
{
  return celcius ? "C" : "F";
}

// 1 bit specifies if temperatures are in F (0) or C (1).
#define CELCIUS_MASK 0x4
// 2 bits are on when solar is on
#define SOLAR_MASK 0x30
// mask to extract the control center's current mode
#define MODE_MASK 0x81

const char* SystemStatus::Name() 
{
  return "System Status Broadcast";
}

void SystemStatus::Read(const uint8_t *data_) 
{
  hour_ = data_[HOUR];
  minutes_ = data_[MINUTES];
  for (uint8_t c = 0; c < CIRCUITS_COUNT; c++) 
  {
    circuits_[c] = data_[CIRCUITS] & (1 << c);
  }
//  uint8_t mode = data_[MODE] & MODE_MASK;
  mode_ = data_[MODE] & MODE_MASK;
  celcius_ = (data_[MODE] & CELCIUS_MASK) == CELCIUS_MASK;
  solar_ = (data_[SOLAR] & SOLAR_MASK) == SOLAR_MASK;
  // bits 1 & 2 contain the pool heat settings
  pool_heat_source_ = data_[HEAT_SETTINGS] & 0x03;
  // bits 3 & 4 contain the spa heat settings
  spa_heat_source_ = ((data_[HEAT_SETTINGS] >> 2) & 0x03);
  air_temp_ = data_[AIR_TEMP];
  pool_temp_ = data_[POOL_TEMP];
  spa_temp_ = data_[SPA_TEMP];
  solar_temp_ = data_[SOLAR_TEMP];
}

void SystemStatus::PrintTo() 
{
  PrintLabel("time");
  PrintTime(hour_, minutes_);
  printf("\n");

  PrintLabel("mode");
  printf("%s\n",SystemModeName(mode_));

  PrintLabel("solar");
  printf("%s\n", solar_ ? "On" : "Off");

  PrintLabel("air T");
  printf("%d",air_temp_);
  printf("%s\n",tempUnit(celcius_));

  PrintLabel("pool T");
  printf("%d",pool_temp_);
  printf("%s\n",tempUnit(celcius_));

  PrintLabel("spa T");
  printf("%d",spa_temp_);
  printf("%s\n",tempUnit(celcius_));

  PrintLabel("solar T");
  printf("%d",solar_temp_);
  printf("%s\n",tempUnit(celcius_));

  PrintLabel("circuits");
  for (int c = 0; c < 8; c++) 
  {
    printf("%s %s",CIRCUITS_NAMES[c], circuits_[c] ? "=ON" : "=OFF");
  }
  printf("\n");

  PrintLabel("pool heat");
  printf("%s\n",HeatSourceName(pool_heat_source_));

  PrintLabel("spa heat");
  printf("%s\n",HeatSourceName(spa_heat_source_));
}
