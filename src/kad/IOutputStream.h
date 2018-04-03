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
  class IOutputStream
  {
  public:
    virtual ~IOutputStream() { }

    virtual size_t Write(const void * buffer, size_t size) = 0;

    virtual bool WriteInt8(int8_t value);
    virtual bool WriteInt8(const int8_t * ary, size_t length);
    virtual bool WriteUInt8(uint8_t value);
    virtual bool WriteUInt8(const uint8_t * ary, size_t length);
    virtual bool WriteInt16(int16_t value);
    virtual bool WriteInt16(const int16_t * ary, size_t length);
    virtual bool WriteUInt16(uint16_t value);
    virtual bool WriteUInt16(const uint16_t * ary, size_t length);
    virtual bool WriteInt32(int32_t value);
    virtual bool WriteInt32(const int32_t * ary, size_t length);
    virtual bool WriteUInt32(uint32_t value);
    virtual bool WriteUInt32(const uint32_t * ary, size_t length);
    virtual bool WriteInt64(int64_t value);
    virtual bool WriteInt64(const int64_t * ary, size_t length);
    virtual bool WriteUInt64(uint64_t value);
    virtual bool WriteUInt64(const uint64_t * ary, size_t length);
    virtual bool WriteString(std::string & str);
  };
}