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

#include "IInputStream.h"
#include "IOutputStream.h"
#include "OpCode.h"

namespace kad
{
  class Instruction
  {
  public:

    explicit Instruction(OpCode code)
      : code(code)
    {
    }

    virtual ~Instruction() = default;

    OpCode Code() const     { return this->code; }

    virtual bool Serialize(IOutputStream & output) const = 0;

    virtual bool Deserialize(IInputStream & input) = 0;

    virtual void Print() const = 0;

  protected:

    inline bool SerializeOpCode(IOutputStream & output) const
    {
      return output.WriteUInt16(static_cast<uint16_t>(this->code));
    }

    inline bool DeserializeOpCode(IInputStream & input)
    {
      return input.Remainder() >= sizeof(uint16_t) && input.ReadUInt16() == static_cast<uint16_t>(this->code);
    }

  protected:

    OpCode code;
  };
}