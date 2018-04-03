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

#include <memory.h>
#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "BufferedInputStream.h"

namespace kad
{
  bool BufferedInputStream::Initialize(const uint8_t * buffer, size_t length)
  {
    this->buffer = buffer;

    this->offset = 0;

    this->length = length;

    this->needFree = false;

    this->isValid = true;

    return true;
  }


  BufferedInputStream::~BufferedInputStream(void)
  {
    if (this->needFree)
    {
      delete[] this->buffer;
    }
  }

  size_t BufferedInputStream::Read(void * buffer, size_t length)
  {
    size_t remainder = this->length - this->offset;

    if (remainder < length)
    {
      this->isValid = false;

      length = remainder;
    }

    memcpy(buffer, this->buffer + this->offset, length);

    this->offset += length;

    return length;
  }

  size_t BufferedInputStream::Peek(void * buffer, size_t length)
  {
    size_t remainder = this->length - this->offset;

    if (remainder < length)
    {
      length = remainder;
    }

    memcpy(buffer, this->buffer + this->offset, length);

    return length;
  }
}