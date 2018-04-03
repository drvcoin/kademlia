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
#include <stdint.h>
#include <string>

namespace kad
{
  class IInputStream
  {
  public:
    virtual ~IInputStream() { }

    virtual const void * BufferAt(size_t offset) = 0;
    virtual size_t Length() =  0;
    virtual size_t Offset() =  0;
    virtual size_t Remainder() = 0;
    virtual void Rewind(size_t bytes) = 0;
    virtual void Skip(size_t bytes) = 0;
    virtual size_t Read(void * buffer, size_t length) = 0;
    virtual size_t Peek(void * buffer, size_t length) = 0;
    virtual bool IsValid() = 0;

    virtual int8_t ReadInt8();
    virtual size_t ReadInt8(int8_t * ary, size_t length);
    virtual int16_t ReadInt16();
    virtual size_t ReadInt16(int16_t * ary, size_t length);
    virtual int32_t ReadInt32();
    virtual size_t ReadInt32(int32_t * ary, size_t length);
    virtual int64_t ReadInt64();
    virtual size_t ReadInt64(int64_t * ary, size_t length);
    virtual uint8_t ReadUInt8();
    virtual size_t ReadUInt8(uint8_t * ary, size_t length);
    virtual uint16_t PeekUInt16();
    virtual uint16_t ReadUInt16();
    virtual size_t ReadUInt16(uint16_t * ary, size_t length);
    virtual uint32_t ReadUInt32();
    virtual size_t ReadUInt32(uint32_t * ary, size_t length);
    virtual uint64_t ReadUInt64();
    virtual size_t ReadUInt64(uint64_t * ary, size_t length);
    virtual size_t ReadString(std::string & str);
  };
}