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

namespace kad
{
  class BufferedInputStream : public IInputStream
  {
  private:
    const uint8_t * buffer;
    size_t length;
    size_t offset;
    bool needFree;
    bool isValid;

  public:
    BufferedInputStream() : buffer(NULL), length(0), offset(0), needFree(false), isValid(true) { }
    BufferedInputStream(const uint8_t * buffer, size_t length) : buffer(buffer), length(length), offset(0), needFree(false), isValid(true) {}
    ~BufferedInputStream(void);

    bool Initialize(IInputStream * stream);
    bool Initialize(const uint8_t * buffer, size_t length);

    const void * Buffer() { return buffer; }
    const void * BufferAt(size_t offset) { return buffer + offset; }
    void Rewind(size_t bytes) { offset -= bytes; }
    void Skip(size_t bytes) { offset += bytes; }

    // IInputStream members
    size_t Length() { return length; }
    size_t Offset() { return offset; }
    size_t Remainder() { return length - offset; }
    size_t Read(void * buffer, size_t length);
    size_t Peek(void * buffer, size_t length);
    bool IsValid() { return isValid; }
  };
}