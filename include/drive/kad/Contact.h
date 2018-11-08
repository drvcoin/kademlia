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

#include <string.h>
#include <memory>
#include <string>
#include "IOutputStream.h"
#include "IInputStream.h"

namespace kad
{
  struct Contact
  {
    unsigned long addr = 0;
    unsigned short port = 0;

    bool Serialize(IOutputStream & output) const
    {
      output.WriteInt32(static_cast<int32_t>(addr));
      output.WriteInt16(static_cast<int16_t>(port));
      return true;
    }


    bool Deserialize(IInputStream & input)
    {
      if (input.Remainder() < sizeof(int32_t) + sizeof(int16_t))
      {
        return false;
      }

      this->addr = static_cast<unsigned long>(input.ReadInt32());
      this->port = static_cast<unsigned short>(input.ReadInt16());
      return true;
    }

    std::string ToString() const
    {
      char buffer[32];
      sprintf(buffer, "%ld.%ld.%ld.%ld:%u", (addr & 0xFF), ((addr >> 8) & 0xFF), ((addr >> 16) & 0xFF), ((addr >> 24) & 0xFF), port);
      return buffer;
    }
  };

  using ContactPtr = std::shared_ptr<Contact>;
}
