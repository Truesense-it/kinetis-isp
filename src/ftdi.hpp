/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#ifndef _FTDI_INTERFACE_H_
#define _FTDI_INTERFACE_H_

#include <string>
#include <vector>
#include <cstdint>
using namespace std;
namespace FTDI{
  enum CBUSMode: int{INPUT=0,OUTPUT=1};
  struct CBUSPins{
    bool outputCBUS0;
    bool outputCBUS1;
    bool outputCBUS2;
    bool outputCBUS3;
    CBUSMode modeCBUS0;
    CBUSMode modeCBUS1;
    CBUSMode modeCBUS2;
    CBUSMode modeCBUS3;
  };

  class Interface{
    public:
    virtual void open(const int vid, const int pid) = 0;
    virtual void open(std::string dev) = 0;
    virtual bool is_open() = 0;

    virtual int setCBUSPins(const CBUSPins& pins) = 0;
    virtual int disableCBUSMode() = 0;

    virtual int writeData(std::vector<uint8_t> data) = 0;
    virtual std::vector<uint8_t> readData() = 0;
    virtual int setBaudrate(uint32_t speed) = 0;
};
}

#endif /* _FTDI_INTERFACE_H_ */