// /*
//  *      Author: jm
//  */

#include "packet.h"

const char* PumpControlRequest::Name() 
{
  return "Pump Control Request";
}

void PumpControlRequest::Read(const uint8_t *data) 
{
  running_ = (data[0] == 0x10) ? true : false;
}

void PumpControlRequest::PrintTo()
{
  PrintLabel("running");
  printf("%s\n",running_ ? "Yes" : "No");
}

const char* PumpControlResponse::Name()
{
  return "Pump Control Response";
}

void PumpControlResponse::Read(const uint8_t *data)
{
  running_ = (data[0] == 0x10) ? true : false;
}

void PumpControlResponse::PrintTo()
{
  PrintLabel("running");
  printf("%s\n",running_ ? "Yes" : "No");
}
