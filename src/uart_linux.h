/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#ifndef _UARTLINUX_HPP_
#define _UARTLINUX_HPP_

#include "ftdi.hpp"

#include <libftdi1/ftdi.h> // libftdi header
#include <memory>
#include <vector>

class UARTLinux : public FTDI::Interface {
public:
  UARTLinux();
  UARTLinux(const int vid, const int pid);
  virtual ~UARTLinux();

  void open(const int vid, const int pid);
  void open(std::string dev);
  bool is_open();

  int setCBUSPins(const FTDI::CBUSPins& pins);
  int disableCBUSMode();

  int writeData(std::vector<uint8_t> data);
  std::vector<uint8_t> readData();
  int setBaudrate(uint32_t speed);
private:
  [[maybe_unused]]struct ftdi_context * ftdi = nullptr;
  int fd;
};
#endif /* _UARTLINUX_HPP_ */