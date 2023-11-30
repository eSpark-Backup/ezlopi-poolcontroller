// /*
//  *      Author: jm
//  */

#include "packet.h"

const char* PumpStatusRequest::Name() 
{
  return "Pump Status Request";
}

void PumpStatusRequest::Read(const uint8_t *data) 
{
  // nothing
}

void PumpStatusRequest::PrintTo() 
{
  // nothing
}

const char* PumpStatusResponse::Name() 
{
  return "Pump Status Response";
}

void PumpStatusResponse::Read(const uint8_t *data_) 
{
  running_ = (data_[0] == 0x0a) ? true : false;
  watts_ = toUint16(data_[3], data_[4]);
  rpms_ = toUint16(data_[5], data_[6]);
  priming_ = (data_[12] == 0x0b) ? true : false;
  hour_ = data_[13];
  minutes_ = data_[14];
}

void PumpStatusResponse::PrintTo()
{
  PrintLabel("time");
  PrintTime(hour_, minutes_);
  printf("\n");

  PrintLabel("running");
  printf("%s\n",(running_ ? "Yes" : "No"));

  PrintLabel("priming");
  printf("%s\n",(priming_ ? "Yes" : "No"));

  PrintLabel("watts");
  printf("%d\n",watts_);

  PrintLabel("rpms");
  printf("%d\n",rpms_);
}
