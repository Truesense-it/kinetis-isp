/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#ifndef _FTDI_MOCK_H_
#define _FTDI_MOCK_H_

#include <ftdi.hpp>

#include <gmock/gmock.h>

class FTDIMock : public FTDI::Interface {
public:
  MOCK_METHOD2(open, int(const int vid, const int pid));
  MOCK_METHOD0(is_open, bool());
  MOCK_METHOD1(setCBUSPins, int(const FTDI::CBUSPins& pins));
  MOCK_METHOD0(diableCBUSMode, int());

  MOCK_METHOD1(writeData, int(std::vector<uint8_t> data));
  MOCK_METHOD0(readData, std::vector<uint8_t>());
};

#endif /* _FTDI_MOCK_H_ */