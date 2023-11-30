// /*
//  *      Author: jm
//  */

#include "packet.h"

const char* ScheduleConfig::Name() 
{
  return "Schedule Config";
}

void ScheduleConfig::Read(const uint8_t *data_) 
{
  circuit_ = data_[0];
  start_hour_ = data_[2];
  start_minute_ = data_[3];
  stop_hour_ = data_[4];
  stop_minute_ = data_[5];
  active_days_ = data_[6];
}

void ScheduleConfig::PrintTo()
{
  PrintLabel("circuit");
  printf("%d\n",circuit_);

  PrintLabel("start");
  PrintTime(start_hour_, start_minute_);
  printf("\n");

  PrintLabel("stop");
  PrintTime(stop_hour_, stop_minute_);
  printf("\n");

  PrintLabel("active");
  const char DAYS[] = "MTWTFSS";
  for (int i = 0; i < 7; i++) 
  {
    bool on = active_days_ & (1 << i);
    printf("%d",on ? DAYS[i] : '-');
  }
  printf("\n");
}