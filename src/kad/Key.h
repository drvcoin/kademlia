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

#pragma once

#include <stdlib.h>
#include <memory>
#include <string>
#include "IOutputStream.h"
#include "IInputStream.h"

namespace kad
{
  class Key
  {
  public:

    static const size_t KEY_LEN_BITS = 160;

    static const size_t KEY_LEN = KEY_LEN_BITS / 8;

  public:

    Key() = default;

    explicit Key(const void * buf);

    Key(const Key & val);

    Key GetDistance(const Key & target) const;

    bool operator<(const Key & target) const;

    bool operator>(const Key & target) const;

    bool operator<=(const Key & target) const;

    bool operator>=(const Key & target) const;

    bool operator==(const Key & target) const;

    bool Bit(size_t idx) const;

    void SetBit(size_t idx, bool value);

    int GetHighestBit() const;

    const uint8_t * Buffer() const    { return this->key; }

    std::string ToString() const;

    bool FromString(const char * str);

    bool Serialize(IOutputStream & output) const;

    bool Deserialize(IInputStream & input);

  private:

    uint8_t key[KEY_LEN] = {};
  };

  using KeyPtr = std::shared_ptr<Key>;

  struct KeyCompare
  {
    bool operator()(const KeyPtr & lhs, const KeyPtr & rhs) const
    {
      if (lhs == nullptr)
      {
        return true;
      }
      else if (rhs == nullptr)
      {
        return false;
      }
      else
      {
        return (* lhs) < (* rhs);
      }
    }
  };
}