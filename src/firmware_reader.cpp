#include "firmware_reader.h"

#include <iterator>

FirmwareReader::FirmwareReader(std::istream& _is) : is(_is)
{
}

FirmwareReader::~FirmwareReader()
{
}

std::vector<uint8_t> FirmwareReader::data() const{
  is.unsetf(std::ios::skipws);

    std::vector<uint8_t> vec{};
  vec.reserve(size());

  vec.insert(vec.begin(),
             std::istream_iterator<uint8_t>(is),
             std::istream_iterator<uint8_t>());

  return vec;
}

std::size_t FirmwareReader::size() const{
  is.seekg(0, std::ios::end);
  auto size = is.tellg();
  is.seekg(0, std::ios::beg);

  return size;
}