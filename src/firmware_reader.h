/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */

#ifndef _FIRMWARE_READER_H_
#define _FIRMWARE_READER_H_

#include <fstream>
#include <vector>

class FirmwareReader
{
public:
  FirmwareReader(std::istream& is);
  ~FirmwareReader();

  std::vector<uint8_t> data() const;
  std::size_t size() const;

private:
  std::istream& is;
};


#endif /* _FIRMWARE_READER_H_ */