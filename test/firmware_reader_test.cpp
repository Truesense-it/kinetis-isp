/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */

#include <firmware_reader.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ContainerEq;
using ::testing::AllOf;

template <typename T>
inline void serialize (std::vector< uint8_t >& dst, const T& data) {
    const uint8_t * src = reinterpret_cast<const uint8_t*>(&data);
    dst.insert (dst.end (), src, src + sizeof (T));
}   

TEST(FirmwareReader_data, canOpenStringStream){
  const std::uint64_t data = 1234567890;
  std::stringstream sstr{};
  sstr.write(reinterpret_cast<const char*>(&data), sizeof(data));
  FirmwareReader fw(sstr);
}

TEST(FirmwareReader_data, returnsInputDataSize){
  const std::uint64_t data = 1234567890;
  std::stringstream sstr{};
  sstr.write(reinterpret_cast<const char*>(&data), sizeof(data));

  FirmwareReader fw(sstr);
  auto bin = fw.data();
  EXPECT_EQ(bin.size(), sizeof(data));
}

TEST(FirmwareReader_data, returnsInputData){
  const std::uint64_t data = 1234567890;
  std::stringstream sstr{};
  sstr.write(reinterpret_cast<const char*>(&data), sizeof(data));

  std::vector<uint8_t> data_vec{};
  serialize<uint64_t>(data_vec, data);

  FirmwareReader fw(sstr);
  auto bin = fw.data();
  EXPECT_THAT(bin, testing::ContainerEq(data_vec));
}

TEST(FirmwareReader_size, returnsSize){
  const std::uint64_t data = 1234567890;
  std::stringstream sstr{};
  sstr.write(reinterpret_cast<const char*>(&data), sizeof(data));

  FirmwareReader fw(sstr);
  EXPECT_EQ(fw.size(), sizeof(data));
}