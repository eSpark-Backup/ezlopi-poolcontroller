// /*
//  *      Author: jm
//  */

#include "packet.h"

const char* HeatStatus::Name() 
{
  return "Heat Status";
}

void HeatStatus::Read(const uint8_t *data) 
{
  pool_temp_ = data[0];
  spa_temp_ = data[1];
  air_temp_ = data[2];
  solar_temp_ = data[8];
  pool_temp_target_ = data[3];
  pool_heat_source_ = data[5] & 0x03;
  spa_temp_target_ = data[4];
  spa_heat_source_ = (data[5] >> 2) & 0x03;
}

void HeatStatus::PrintTo()
{
  PrintLabel("pool T");
  printf("%d\n",pool_temp_);

  PrintLabel("spa T");
  printf("%d\n",spa_temp_);

  PrintLabel("air T");
  printf("%d\n",air_temp_);

  PrintLabel("solar T");
  printf("%d\n",solar_temp_);

  PrintLabel("pool heat");
  printf("%d/%s\n",pool_temp_target_,HeatSourceName(pool_heat_source_));

  PrintLabel("spa heat");
  printf("%d/%s\n",spa_temp_target_,HeatSourceName(spa_heat_source_));
}