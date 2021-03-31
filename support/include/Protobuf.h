#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

enum wiretype
{
  Varint = 0,
  U64    = 1,
  Delim  = 2,
  U32    = 5
};

struct PBView
{
  PBView(const unsigned char* dat, size_t siz)
    : data(dat)
    , size(siz)
  {
  }
  template <typename T>
  PBView(T&& container)
    : data((const unsigned char*)container.data())
    , size(container.size())
  {
  }
  const unsigned char* data;
  size_t               size;
  class sentinel
  {
  };
  struct iterator
  {
    const unsigned char* data_;
    size_t               size_;
    wiretype             type;
    size_t               number;
    size_t               length;
    size_t               readVarint()
    {
      size_t   offset = 7;
      uint64_t val    = 0;
      val |= (*data_) & 0x7f;
      while (*data_ & 0x80)
      {
        ++data_;
        val |= ((*data_) & 0x7F) << offset;
        offset += 7;
        size_--;
        if (size_ == 0)
          throw std::runtime_error("Short packet");
      }
      ++data_;
      --size_;
      return val;
    }

    iterator(const unsigned char* data, size_t size)
      : data_(data)
      , size_(size)
    {
      length = 0;
      ++(*this);
    }
    auto& operator*()
    {
      return *this;
    }
    bool operator!=(const sentinel&) const
    {
      return size_ != 0;
    }
    bool operator==(const sentinel&) const
    {
      return size_ == 0;
    }
    iterator& operator++()
    {
      size_ -= length;
      data_ += length;
      if (size_)
      {
        size_t v1 = readVarint();
        type      = wiretype(v1 & 0x7);
        number    = v1 >> 3;
        switch (type)
        {
          case Delim:
            length = readVarint();
            break;
          case U32:
            length = 4;
            break;
          case U64:
            length = 8;
            break;
          case Varint:
            length = 0;
            while (length < size_ && data_[length] & 0x80)
              length++;
            if (length == size_)
              throw std::runtime_error("Short packet");
            length++;
            break;
          default:
            throw std::runtime_error("Invalid wiretype");
        }
        if (length > size_)
          throw std::runtime_error("Short packet");
      }
      return *this;
    }
    uint64_t read()
    {
      uint64_t val = 0;
      switch (type)
      {
        case Varint:
        {
          size_t n = 0;
          while (data_[n] & 0x80)
          {
            val |= ((data_[n]) & 0x7F) << (n * 7);
            n++;
          }
          val |= ((data_[n]) & 0x7F) << (n * 7);
          break;
        }
        case U64:
        {
          for (size_t n = 0; n < 8; n++)
          {
            val |= (uint64_t(data_[n]) << (8 * n));
          }
          break;
        }
        case U32:
        {
          for (size_t n = 0; n < 4; n++)
          {
            val |= (uint32_t(data_[n]) << (8 * n));
          }
        }
        break;
        default:
          throw std::runtime_error("Unknown/invalid wire type");
      }
      return val;
    }
    std::vector<uint8_t> readBytes()
    {
      return std::vector<uint8_t>(data_, data_ + length);
    }
    std::string readString()
    {
      return std::string(data_, data_ + length);
    }
    double readDouble()
    {
      uint64_t val = 0;
      switch (type)
      {
        case U64:
        {
          for (size_t n = 0; n < 8; n++)
          {
            val |= (uint64_t(data_[n]) << (8 * n));
          }
          double d;
          memcpy((void*)&d, (void*)&val, sizeof(d));
          return d;
        }
        case U32:
        {
          for (size_t n = 0; n < 4; n++)
          {
            val |= (uint32_t(data_[n]) << (8 * n));
          }
          float f;
          memcpy((void*)&f, (void*)&val, sizeof(f));
          return f;
        }
        default:
          throw std::runtime_error("Unknown/invalid wire type");
      }
    }
    PBView pbview()
    {
      return PBView{ data_, length };
    }
  };
  iterator begin()
  {
    return iterator(data, size);
  }
  sentinel end()
  {
    return {};
  }
};

class PBVector : public std::vector<uint8_t>
{
public:
  void writeVarint(uint64_t value)
  {
    while (value > 0x7F)
    {
      push_back((value & 0x7F) | 0x80);
      value >>= 7;
    }
    push_back((uint8_t)value);
  }
  void addData(wiretype wt, size_t number, const std::vector<uint8_t>& data)
  {
    if (data.empty())
      return;
    writeVarint((number << 3) | wt);
    if (wt == Delim)
      writeVarint(data.size());
    insert(end(), data.begin(), data.end());
  }
  void addVarint(size_t number, uint64_t value, bool addEvenIfZero = false)
  {
    if (!addEvenIfZero && value == 0)
      return;
    PBVector v;
    v.writeVarint(value);
    addData(Varint, number, v);
  }
  void addLengthDelim(size_t number, const std::vector<uint8_t>& value)
  {
    addData(Delim, number, value);
  }
  void addLengthDelim(size_t number, const std::string& value)
  {
    addData(Delim, number, std::vector<uint8_t>(value.data(), value.data() + value.size()));
  }
  void addInt64(size_t number, uint64_t value)
  {
    if (value == 0)
      return;
    std::vector<uint8_t> v;
    for (size_t n = 0; n < 8; n++)
    {
      v.push_back(value & 0xFF);
      value >>= 8;
    }
    addData(U64, number, v);
  }
  void addInt32(size_t number, uint32_t value)
  {
    if (value == 0)
      return;
    std::vector<uint8_t> v;
    for (size_t n = 0; n < 4; n++)
    {
      v.push_back(value & 0xFF);
      value >>= 8;
    }
    addData(U32, number, v);
  }
  void addFloat(size_t number, float value)
  {
    std::vector<uint8_t> v;
    v.resize(4);
    memcpy(v.data(), &value, 4);
    addData(U32, number, v);
  }
  void addDouble(size_t number, double value)
  {
    std::vector<uint8_t> v;
    v.resize(8);
    memcpy(v.data(), &value, 8);
    addData(U64, number, v);
  }
};

template <typename T>
T from_protobuf(PBView);

template <typename T>
PBVector to_protobuf(const T&);

template <typename T>
inline constexpr bool is_protobuf = false;
