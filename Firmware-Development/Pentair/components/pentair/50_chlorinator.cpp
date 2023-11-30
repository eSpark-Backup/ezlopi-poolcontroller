// /*
//  *      Author: jm
//  */

#include "packet.h"

const char* ChlorinatorRequest::Name() 
{
  return "Chlorinator Request";
}

void ChlorinatorRequest::Read(const uint8_t *data) 
{
  mode_ = (data[1] == 0x11) ? true : false;
  percent_ = data[2];
}

void ChlorinatorRequest::PrintTo()
{
  PrintLabel("running");
  printf("%s\n",mode_ ? "Yes" : "No");

  PrintLabel("percent");
  printf("%d\n",percent_);
}

const char* ChlorinatorResponse::Name() 
{
  return "Chlorinator Response";
}

void ChlorinatorResponse::Read(const uint8_t *data) 
{
  salinity_ = (uint16_t) data[2] * (uint16_t) 50;
  inspect_cell_ = data[3] & INSPECT_CELL_MASK;
}

void ChlorinatorResponse::PrintTo()
{
  PrintLabel("ppm");
  printf("%d\n",salinity_);
  PrintLabel("inspect");
  printf("%s\n",inspect_cell_ ? "Yes" : "No");
}