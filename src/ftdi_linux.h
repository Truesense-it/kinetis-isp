#ifndef _FTDI_HPP_
#define _FTDI_HPP_

#include "ftdi.hpp"

#include <ftdi.h> // libftdi header
#include <memory>
#include <vector>

class FTDILinux : public FTDI::Interface {
public:
  FTDILinux();
  FTDILinux(const int vid, const int pid);
  virtual ~FTDILinux();

  int open(const int vid, const int pid);
  bool is_open();

  int setCBUSPins(const FTDI::CBUSPins& pins);
  int diableCBUSMode();

  int writeData(std::vector<uint8_t> data);
  std::vector<uint8_t> readData();

private:
  struct ftdi_context * ftdi;
};
#endif /* _FTDI_HPP_ */