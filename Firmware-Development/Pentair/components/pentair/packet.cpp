#include "packet.h"
#include "string.h"
#include "driver/uart.h"

#define DEFAULT_PAD 10

const char* DeviceName(uint8_t id) 
{
  switch ((DeviceID)id) 
  {
    case DEVICE_CONTROL_CENTER:
      return "Control Center";
    case DEVICE_BROADCAST:
      return "Broadcast";
    case DEVICE_PUMP1:
      return "Pump1";
    case DEVICE_PUMP2:
      return "Pump2";
    case DEVICE_CHLORINATOR:
      return "Chlorinator";
    case DEVICE_WIRED_REMOTE:
      return "Wired Remote";
    case DEVICE_WIRELESS_REMOTE:
      return "Wireless Remote";
    case DEVICE_REMOTE:
      return "Remote";
    case DEVICE_90:
      return "Device 0x90";
  }
  return "Unknown";
}

void Packet::Reset() 
{
  valid_ = false;
  state_ = preamble_state;
  pos_ = 0;
  buffer_pos_ = 0;
  version_ = 0;
  target_ = 0;
  source_ = 0;
  cid_ = 0;
  command_ = &unknown;
  data_len_ = 0;
  checksum_ = 0;
  memset(buffer_, 0, sizeof(buffer_));
  memset(checksum_data_, 0, sizeof(checksum_data_));
}

bool Packet::Make(uint8_t cid, const char* data) 
{
  Reset();
  PushHex("ff 00 ff a5 07 10 20");
  Push(cid);
  PushHex(data);
  checksum_data_[0] = (checksum_ / 256) & 0xFF;
  checksum_data_[1] = checksum_ & 0xFF;
  return IsValid();
}

// Read a Frame formatted as an hex string, e.g. "ff 10 02 50 00 00 62 10 03"
bool Packet::ReadFrom(const char *s) 
{
  Reset();
  return PushHex(s);
}

bool Packet::PushHex(const char *f) 
{
  for (int i = 0; i < strlen(f); i++) 
  {
    if (f[i] != ' ' && f[i] != '\t')
    {
      char hi = f[i++];
      char lo = f[i];
      uint8_t b = htoi(hi) * 16 + htoi(lo);
      if (Push(b)) 
      {
        return true;
      }
      printf("%X ",b);
    }
  }
  return false;
}

bool Packet::Push(uint8_t c) 
{
  bool complete = false;
  // First, make sure we don't overrun the internal buffer. Drop the buffer and continue if we do.
  if (buffer_pos_ > sizeof(buffer_)) 
  {
    printf("   ***   BUFFER OVERRUN   ***");
    // Can't make this available to the caller; just debug print for now.
    for (unsigned int i = 0; i < sizeof(buffer_); i++) 
    {
      uint8_t c = buffer_[i];
      PrintHex(c);
    }
    Reset();
  }
  const uint8_t p = buffer_pos_;  // last char in buffer
  buffer_[buffer_pos_++] = c;
  switch (state_) 
  {
    case preamble_state:
      // There's 2 ways we can detect a packet start:
      // 1 - pentair preamble: 0xFF 0x00 0xFF 0xA5
      // 2 - intellichlor preamble: 0x10 0x02
      // Check the we've read at least enough characters for the preamble and
      // see if the 4 last we have match the expected preamble.
      
      if (p >= 3 && buffer_[p] == 0xA5 && buffer_[p - 1] == 0xFF
          && buffer_[p - 2] == 0x00 && buffer_[p - 3] == 0xFF) 
          {
        // 1 - pentair preamble: 0xFF 0x00 0xFF 0xA5
        nextState(version_state, 0xA5);  // This last preamble byte is part of the checksum
      } else if (p >= 1 && buffer_[p] == 0x02 && buffer_[p - 1] == 0x10) 
      {
        // 2 - intellichlor preamble: 0x10 0x02
        nextState(intellichlor_state, 0x12);  // Start sequence "10 02" is part of the checksum.
        data_ = buffer_ + buffer_pos_;      // Data starts on the next byte.
      }
      // Stay in this state until we find a valid header.
      break;
      
    case version_state:
      version_ = c;
      nextState(destination_state, c);
      break;
    case destination_state:
      target_ = c;
      nextState(source_state, c);
      break;
      
    case source_state:
      source_ = c;
      nextState(command_state, c);
      break;
      
    case command_state:
      cid_ = c;
      nextState(length_state, c);
      break;
      
    case length_state:
      data_len_ = c;
      if (data_len_ > 0)
      {
        // keep a pointer to the beginning of the actual data inside the buffer we already have.
        data_ = buffer_ + buffer_pos_;
        nextState(data_state, c);
      } else 
      {
        nextState(checksum_state, c);
      }
      break;
     
    case data_state:
      pos_++;
      checksum_ += (uint16_t) c;
      if (pos_ >= data_len_) 
      {
        // we've read all the data bytes
        nextState(checksum_state, 0);
      }
      break;
    case checksum_state:
      checksum_data_[pos_++] = c;
      if (pos_ >= 2) 
      {
        complete = true;
        nextState(preamble_state, 0);
        // verify checksum
        uint16_t chk = toUint16(checksum_data_[0], checksum_data_[1]);
        if (checksum_ == chk) 
        {
          valid_ = true;
          ReadCommand();
        } else 
        {
          valid_ = false;
        }
      }
      break;
    case intellichlor_state:
      data_len_++;
      const int p = buffer_pos_ - 1;  // last char in buffer
      if (p >= 1 && buffer_[p] == 0x03 && buffer_[p - 1] == 0x10) 
      {
        // Found the end of the packet.
        complete = true;
        nextState(preamble_state, 0);
        // Data ended 3 bytes ago (1 bytes of checksum + 2 bytes of terminator).
        data_len_ -= 3;
        checksum_data_[0] = 0x00;
        checksum_data_[1] = buffer_[p - 2];
        uint8_t chk = checksum_data_[1];
        // the checksum includes the packet header "10 02" to the end of the data.
        uint8_t checksum = 0x12;
        for (int i = 0; i < data_len_; i++) 
        {
          checksum += data_[i];
        }
        if (checksum == chk) 
        {
          valid_ = true;
          // Make the packet data look like the other packets by setting command/target/source.
          cid_ = COMMAND_CHLORINATOR;
          if (data_[0] == 0x50) 
          {
            source_ = DEVICE_CONTROL_CENTER;
            target_ = DEVICE_CHLORINATOR;
          } else 
          {
            source_ = DEVICE_CHLORINATOR;
            target_ = DEVICE_CONTROL_CENTER;
          }
          ReadCommand();
        } else 
        {
          valid_ = false;
        }
        checksum_ = checksum;
      }
      break;
  }
  return complete;
}

void Packet::ReadCommand() 
{
  switch (cid_) 
  {
    case COMMAND_SYSTEM_STATUS:
      command_ = &system_status;
      break;
    case COMMAND_PUMP_STATUS:
      if (DEVICE_CONTROL_CENTER == source_) 
      {
        command_ = &pump_status_request;
      } 
      else 
      {
        command_ = &pump_status_response;
      }
      break;
    case COMMAND_PUMP_CONTROL:
      if (DEVICE_CONTROL_CENTER == source_) 
      {
        command_ = &pump_control_request;
      } 
      else 
      {
        command_ = &pump_control_response;
      }
      break;
    case COMMAND_CHLORINATOR:
      if (data_[0] == 0x50) 
      {
        command_ = &chlorinator_request;
      } 
      else 
      {
        command_ = &chlorinator_response;
      }
      break;
    case COMMAND_SCHEDULE_CONFIG:
      command_ = &schedule_config;
      break;
    case COMMAND_TIME_SETTINGS:
      command_ = &time_settings;
      break;
    case COMMAND_HEAT_STATUS:
      command_ = &heat_status;
      break;
  }

  command_->Read(data_);

  if (COMMAND_SYSTEM_STATUS == cid_) 
  {
    // initialize & maintain the clock from the time received in the command center status sent every 2 seconds.
    if (clock_.minute < system_status.minutes_) 
    {
      clock_.initialize(system_status.hour_, system_status.minutes_);
    }
  }
  if (clock_.initialized) 
  {
    clock_.time();
  }
}

uint8_t htoi(char c) 
{
  if (c >= '0' && c <= '9') 
  {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') 
  {
    return 10 + c - 'a';
  }
  if (c >= 'A' && c <= 'F') 
  {
    return 10 + c - 'A';
  }
  return 0;  // not reached
}

uint16_t toUint16(uint8_t hi, uint8_t lo) 
{
  return (uint16_t) hi * 256 + (uint16_t) lo;
}

void PrintLabel(const char *l) 
{
  PrintLabel(l, DEFAULT_PAD);
}

void PrintLabel(const char *l, int pad) 
{
  printf("%s:",l);
  for (int i = strlen(l) + 1; i < pad; i++) 
  {
    printf(" ");
  }
}

void PrintHex(uint8_t c) 
{
  // static char digits[] = "0123456789abcdef";
  printf("%X", c);
}
// p.print(digits[(c >> 4) & 0x0f]);
//   p.print(digits[c & 0x0f]);

void PrintTime(uint8_t h, uint8_t m) 
{
  printf("%d %s %d", h, (m < 10) ? ":0" : ":", m);
}

void printPad(int n) 
{
  if (n < 10)
    printf("0");
  printf("%d",n);
}

void printPad(long n) 
{
  if (n < 100)
    printf("0");
  if (n < 10)
    printf("0");
  printf("%ld",n);
}

void PrintDate(uint8_t day, uint8_t month, uint8_t year) 
{
  printPad(month);
  printf("/");
  printPad(day);
  printf("/20");
  printPad(year);
}

void PrintClock(Clock &c) 
{
  printPad(c.hour);
  printf(":");
  printPad(c.minute);
  printf(":");
  printPad(c.second);
  printf(".");
  printPad(c.millisecond);
}

void Packet::PrintHeader() 
{
  PrintLabel("millis");
  printf("%lld\n",millis());

  PrintLabel("version");
  PrintHex(version_);
  printf("\n");

  PrintLabel("source");
  PrintHex(source_);
  printf(" (");
  printf(DeviceName(source_));
  printf(")");
  printf("\n");

  PrintLabel("target");
  PrintHex(target_);
  printf(" (");
  printf(DeviceName(target_));
  printf(")");
  printf("\n");

  PrintLabel("command");
  PrintHex(cid_);
  printf(" (");
  printf(command_->Name());
  printf(")");
  printf("\n");
}

void Packet::PrintTo() 
{
  PrintHeader();
  command_->PrintTo();
  printf("- Packet Data [%d]\n",data_len_);
  if (data_len_ > 0) 
  {
    printf("\t\t");
    for (int i = 0; i < data_len_; i++) 
    {
      printf("\t%d",i);
    }
    printf("\n");

    printf("data(0x):\t");
    for (int i = 0; i < data_len_; i++) 
    {
      PrintHex(data_[i]);
      printf("\t");
    }
    printf("\n");

    printf("data(10):");
    for (int i = 0; i < data_len_; i++) 
    {
      printf("\t%d",data_[i]);
    }
    printf("\n");
  }
// print single line data header with clock,src,tgt,command, so data dumps can be grep-ed for specific commands.
  if (clock_.initialized) 
  {
    PrintClock(clock_);
    printf(" ");
    PrintHex(source_);
    PrintHex(target_);
    PrintHex(cid_);
    printf(":: ");
    for (int i = 0; i < data_len_; i++) 
    {
      PrintHex(data_[i]);
      printf(" ");
    }
    printf("\n");
  }

  printf("checkdat:");
  for (int i = 0; i < 2; i++) 
  {
    printf("\t");
    PrintHex(checksum_data_[i]);
  }
  printf("\n");
  printf("checksum:");
  uint8_t hi = (checksum_ / 256) & 0xFF;
  uint8_t lo = checksum_ & 0xFF;
  printf("\t");
  PrintHex(hi);
  printf("\t");
  PrintHex(lo);
  printf("\n");

  printf("bufferPos:\t%d\n",buffer_pos_);
  printf("buffer:\t");
  for (int i = 0; i < buffer_pos_; i++) 
  {
    printf("\t");
    PrintHex(buffer_[i]);
  }
  printf("\n");
  printf("---------------------------------\n");
}

// Function to write data to UART
void write_to_uart(const uint8_t *data, size_t len) 
{
  uart_write_bytes(UART_NUM_2, (const char *)data, len);
}

void Packet::WriteTo() 
{
  if (version_ == 0) 
  {
  //   // chlorinator protocol
  //   const uint8_t prefix[] = {0x10, 0x02};
  //   const uint8_t suffix[] = {0x10, 0x03};
  //   p.write(prefix, sizeof(prefix));
  //   for(int i = 0; i < data_len_; i++) 
  //   {
  //     p.write(data_[i]);
  //   }
  //   p.write(checksum_data_[1]);
  //   p.write(suffix, sizeof(suffix));
    const uint8_t prefix[] = {0xff, 0x00, 0xff, 0xa5};
      //uart_write_bytes(UART_NUM_2,prefix,sizeof(prefix));
      write_to_uart(prefix, sizeof(prefix));
      write_to_uart(&version_, sizeof(version_));
      write_to_uart(&target_, sizeof(target_));
      write_to_uart(&source_, sizeof(source_));
      write_to_uart(&cid_, sizeof(cid_));
      write_to_uart(&data_len_, sizeof(data_len_));

      for(int i = 0; i < data_len_; i++) 
      {
        write_to_uart(&data_[i],sizeof(data_len_));
      }
      write_to_uart(&checksum_data_[0],sizeof(checksum_data_[0]));
      write_to_uart(&checksum_data_[1],sizeof(checksum_data_[1]));
    }
  } 
  // else 
  // {
    // intellitouch protocol
    // const uint8_t prefix[] = {0xff, 0x00, 0xff, 0xa5};
    // //uart_write_bytes(UART_NUM_2,prefix,sizeof(prefix));
    // write_to_uart(prefix, sizeof(prefix));
    // write_to_uart(&version_, sizeof(version_));
    // write_to_uart(&target_, sizeof(target_));
    // write_to_uart(&source_, sizeof(source_));
    // write_to_uart(&cid_, sizeof(cid_));
    // write_to_uart(&data_len_, sizeof(data_len_));

    // for(int i = 0; i < data_len_; i++) 
    // {
    //   write_to_uart(&data_[i],sizeof(data_len_));
    // }
    // write_to_uart(&checksum_data_[0],sizeof(checksum_data_[0]));
    // write_to_uart(&checksum_data_[1],sizeof(checksum_data_[1]));
  // }
// }

