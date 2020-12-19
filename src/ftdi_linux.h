/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#ifndef _FTDI_HPP_
#define _FTDI_HPP_

#include "ftdi.hpp"

#include <libftdi1/ftdi.h> // libftdi header
#include <memory>
#include <vector>

class FTDILinux : public FTDI::Interface {
public:
  FTDILinux();
  FTDILinux(const int vid, const int pid);
  virtual ~FTDILinux();

  void open(const int vid, const int pid);
  bool is_open();

  int setCBUSPins(const FTDI::CBUSPins& pins);
  int disableCBUSMode();

  int writeData(std::vector<uint8_t> data);
  std::vector<uint8_t> readData();

private:
  struct ftdi_context * ftdi;
};
#endif /* _FTDI_HPP_ */