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

#include <drive/kad/IInputStream.h>
#include <drive/kad/EndianUtil.h>

namespace kad
{
  int8_t IInputStream::ReadInt8()
  {
    int8_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return value;
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadInt8(int8_t * ary, size_t length)
  {
    return Read(ary, length);
  }

  int16_t IInputStream::ReadInt16()
  {
    int16_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return be16toh(value);
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadInt16(int16_t * ary, size_t length)
  {
    size_t read = Read(ary, length * sizeof(* ary)) / sizeof(* ary);

    if (read == length)
    {
      for (size_t i = 0; i < read; i++)
      {
        ary[i] = be16toh(ary[i]);
      }
    }

    return read;
  }

  int32_t IInputStream::ReadInt32()
  {
    int32_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return be32toh(value);
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadInt32(int32_t * ary, size_t length)
  {
    size_t read = Read(ary, length * sizeof(* ary)) / sizeof(* ary);

    if (read == length)
    {
      for (size_t i = 0; i < read; i++)
      {
        ary[i] = be32toh(ary[i]);
      }
    }

    return read;
  }

  int64_t IInputStream::ReadInt64()
  {
    int64_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return be64toh(value);
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadInt64(int64_t * ary, size_t length)
  {
    size_t read = Read(ary, length * sizeof(* ary)) / sizeof(* ary);

    if (read == length)
    {
      for (size_t i = 0; i < read; i++)
      {
        ary[i] = be64toh(ary[i]);
      }
    }

    return read;
  }

  uint8_t IInputStream::ReadUInt8()
  {
    int8_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return value;
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadUInt8(uint8_t * ary, size_t length)
  {
    return Read(ary, length);
  }

  uint16_t IInputStream::ReadUInt16()
  {
    int16_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return be16toh(value);
    }
    else
    {
      return 0;
    }
  }

  uint16_t IInputStream::PeekUInt16()
  {
    int16_t value;

    if (Peek(&value, sizeof(value)) == sizeof(value))
    {
      return be16toh(value);
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadUInt16(uint16_t * ary, size_t length)
  {
    size_t read = Read(ary, length * sizeof(* ary)) / sizeof(* ary);

    if (read == length)
    {
      for (size_t i = 0; i < read; i++)
      {
        ary[i] = be16toh(ary[i]);
      }
    }

    return read;
  }

  uint32_t IInputStream::ReadUInt32()
  {
    int32_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return be32toh(value);
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadUInt32(uint32_t * ary, size_t length)
  {
    size_t read = Read(ary, length * sizeof(* ary)) / sizeof(* ary);

    if (read == length)
    {
      for (size_t i = 0; i < read; i++)
      {
        ary[i] = be32toh(ary[i]);
      }
    }

    return read;
  }

  uint64_t IInputStream::ReadUInt64()
  {
    int64_t value;

    if (Read(&value, sizeof(value)) == sizeof(value))
    {
      return be64toh(value);
    }
    else
    {
      return 0;
    }
  }

  size_t IInputStream::ReadUInt64(uint64_t * ary, size_t length)
  {
    size_t read = Read(ary, length * sizeof(* ary)) / sizeof(* ary);

    if (read == length)
    {
      for (size_t i = 0; i < read; i++)
      {
        ary[i] = be64toh(ary[i]);
      }
    }

    return read;
  }

  size_t IInputStream::ReadString(std::string & str)
  {
    uint32_t strLength = ReadUInt32();

    size_t size = sizeof(uint32_t);

    if (IsValid() && strLength > 0)
    {
      str.resize(strLength);
      size += ReadInt8((int8_t*)str.data(), strLength);
    }

    return size;
  }
}
