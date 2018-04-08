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

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <assert.h>
#include <string.h>
#include "Key.h"

namespace kad
{
  const size_t Key::KEY_LEN_BITS;

  const size_t Key::KEY_LEN;


  Key::Key(const void * buf)
  {
    assert(buf);

    memcpy(this->key, buf, KEY_LEN);
  }


  Key::Key(const Key & val)
  {
    memcpy(this->key, val.key, KEY_LEN);
  }


  Key Key::GetDistance(const Key & target) const
  {
    static_assert(KEY_LEN % sizeof(uint32_t) == 0, "KEY_LEN should be a multiply of 4");

    Key result;

    const uint32_t * x = reinterpret_cast<const uint32_t *>(this->key);
    const uint32_t * y = reinterpret_cast<const uint32_t *>(target.key);
    uint32_t * ptr = reinterpret_cast<uint32_t *>(result.key);

    for (size_t i = 0; i < KEY_LEN / sizeof(uint32_t); ++i)
    {
      * ptr = ((* x) ^ (* y));
      ++x;
      ++y;
      ++ptr;
    }

    return result;
  }


  bool Key::operator<(const Key & target) const
  {
    return memcmp(this->key, target.key, KEY_LEN) < 0;
  }


  bool Key::operator>(const Key & target) const
  {
    return target < (* this);
  }


  bool Key::operator<=(const Key & target) const
  {
    return !(target < (* this));
  }


  bool Key::operator>=(const Key & target) const
  {
    return !((* this) < target);
  }


  bool Key::operator==(const Key & target) const
  {
    return memcmp(this->key, target.key, KEY_LEN) == 0;
  }


  bool Key::Bit(size_t idx) const
  {
    assert(idx < KEY_LEN_BITS);

    // reverse order access, bit 0 is the last one
    idx = KEY_LEN_BITS - 1 - idx;

    return (this->key[idx / 8] & (1 << (7 - idx % 8))) != 0;
  }


  void Key::SetBit(size_t idx, bool value)
  {
    assert(idx < KEY_LEN_BITS);

    idx = KEY_LEN_BITS - 1 - idx;

    uint8_t mask = static_cast<uint8_t>(1 << (7 - idx % 8));

    if (value)
    {
      this->key[idx / 8] |= mask;
    }
    else
    {
      this->key[idx / 8] &= (~mask);
    }
  }


  int Key::GetHighestBit() const
  {
    for (size_t i = 0; i < KEY_LEN; ++i)
    {
      if (this->key[i])
      {
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
        return KEY_LEN_BITS - 1 - (i * 8 + __builtin_clz((unsigned)this->key[i]) - (sizeof(unsigned) - 1) * 8);
#elif defined(_MSC_VER)
        unsigned long idx = 0;
        _BitScanReverse(& idx, (unsigned long)this->key[i]);
        return KEY_LEN_BITS - 1 - (i * 8 + idx);
#else
        static_assert(false, "Not implemented");
#endif
      }
    }

    return -1;
  }


  std::string Key::ToString() const
  {
    char buffer[KEY_LEN * 2 + 1];

    buffer[KEY_LEN * 2] = '\0';

    for (size_t i = 0; i < KEY_LEN; ++i)
    {
      sprintf(buffer + i * 2, "%02X", this->key[i]);
    }

    return buffer;
  }


  bool Key::FromString(const char * str)
  {
    if (!str || strlen(str) != KEY_LEN * 2)
    {
      return false;
    }

    for (size_t i = 0; i < KEY_LEN; ++i)
    {
      unsigned value = 0;
      if (sscanf(str + i * 2, "%02X", &value) <= 0)
      {
        return false;
      }

      this->key[i] = static_cast<uint8_t>(value);
    }

    return true;
  }


  bool Key::Serialize(IOutputStream & output) const
  {
    return output.Write(this->key, KEY_LEN) == KEY_LEN;
  }


  bool Key::Deserialize(IInputStream & input)
  {
    if (input.Remainder() < KEY_LEN)
    {
      return false;
    }

    return input.Read(this->key, KEY_LEN) == KEY_LEN;
  }
}