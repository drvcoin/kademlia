/**
 *
 * MIT License
 * 
 * Copyright (c) 2018 drvcoin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * =============================================================================
 */

#include <drive/kad/IOutputStream.h>
#include <drive/kad/EndianUtil.h>

namespace kad
{
  static unsigned long signature= 0x01020304UL; 

  bool IOutputStream::WriteInt8(int8_t value)
  {
    return Write(&value, sizeof(int8_t));
  }

  bool IOutputStream::WriteInt8(const int8_t * ary, size_t length)
  {
    return Write(ary, sizeof(* ary) * length);
  }

  bool IOutputStream::WriteUInt8(uint8_t value)
  {
    return Write(&value, sizeof(value));
  }

  bool IOutputStream::WriteUInt8(const uint8_t * ary, size_t length)
  {
    return Write(ary, sizeof(* ary) * length);
  }

  bool IOutputStream::WriteInt16(int16_t value)
  {
    if (1 != (unsigned char&)signature)
    {
      value = htobe16(value);
    }
    return Write(&value, sizeof(value));
  }

  bool IOutputStream::WriteInt16(const int16_t * ary, size_t length)
  {
    int16_t * _ary = (int16_t *)ary;
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = htobe16(_ary[i]);
      }
    }
    bool result = Write(ary, sizeof(* ary) * length);
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = be16toh(_ary[i]);
      }
    }
    return result;
  }

  bool IOutputStream::WriteUInt16(uint16_t value)
  {
    if (1 != (unsigned char&)signature)
    {
      value = htobe16(value);
    }
    return Write(&value, sizeof(value));
  }

  bool IOutputStream::WriteUInt16(const uint16_t * ary, size_t length)
  {
    uint16_t * _ary = (uint16_t *)ary;
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = htobe16(_ary[i]);
      }
    }
    bool result = Write(ary, sizeof(* ary) * length);
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = be16toh(_ary[i]);
      }
    }
    return result;
  }

  bool IOutputStream::WriteInt32(int32_t value)
  {
    if (1 != (unsigned char&)signature)
    {
      value = htobe32(value);
    }
    return Write(&value, sizeof(value));
  }

  bool IOutputStream::WriteInt32(const int32_t * ary, size_t length)
  {
    int32_t * _ary = (int32_t *)ary;
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = htobe32(_ary[i]);
      }
    }
    bool result = Write(ary, sizeof(* ary) * length);
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = be32toh(_ary[i]);
      }
    }
    return result;
  }

  bool IOutputStream::WriteUInt32(uint32_t value)
  {
    if (1 != (unsigned char&)signature)
    {
      value = htobe32(value);
    }
    return Write(&value, sizeof(value));
  }

  bool IOutputStream::WriteUInt32(const uint32_t * ary, size_t length)
  {
    uint32_t * _ary = (uint32_t *)ary;
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = htobe32(_ary[i]);
      }
    }
    bool result = Write(ary, sizeof(* ary) * length);
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = be32toh(_ary[i]);
      }
    }
    return result;
  }

  bool IOutputStream::WriteInt64(int64_t value)
  {
    if (1 != (unsigned char&)signature)
    {
      value = htobe64(value);
    }
    return Write(&value, sizeof(value));
  }

  bool IOutputStream::WriteInt64(const int64_t * ary, size_t length)
  {
    int64_t * _ary = (int64_t *)ary;
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = htobe64(_ary[i]);
      }
    }
    bool result = Write(ary, sizeof(* ary) * length);
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = be64toh(_ary[i]);
      }
    }
    return result;
  }

  bool IOutputStream::WriteUInt64(uint64_t value)
  {
    if (1 != (unsigned char&)signature)
    {
      value = htobe64(value);
    }
    return Write(&value, sizeof(value));
  }

  bool IOutputStream::WriteUInt64(const uint64_t * ary, size_t length)
  {
    uint64_t * _ary = (uint64_t *)ary;
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = htobe64(_ary[i]);
      }
    }
    bool result = Write(ary, sizeof(* ary) * length);
    if (1 != (unsigned char&)signature)
    {
      for (size_t i = 0; i < length; i++)
      {
        _ary[i] = be64toh(_ary[i]);
      }
    }
    return result;
  }

  bool IOutputStream::WriteString(std::string & str)
  {
    return (
      WriteUInt32(str.length()) &&
      WriteInt8((int8_t*)str.data(), str.length())
    );
  }
}
