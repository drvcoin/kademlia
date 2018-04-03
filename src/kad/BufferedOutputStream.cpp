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

#include "BufferedOutputStream.h"

#include <memory.h>
#include <stdlib.h>
#include <memory>

namespace kad
{
  BufferedOutputStream::BufferedOutputStream() : buffer(NULL), length(0), offset(0)
  {
  }

  BufferedOutputStream::~BufferedOutputStream()
  {
    free(this->buffer);
  }

  bool BufferedOutputStream::EnsureBuffer(size_t length)
  {
    if ((this->offset + length) > this->length)
    {
      this->length = this->length + length + 64;

      this->length &= ~63;

      this->buffer = (uint8_t *)realloc(this->buffer, this->length);

      if (this->buffer == NULL)
      {
        throw std::bad_alloc();
      }
    }

    return true;
  }

  size_t BufferedOutputStream::Write(const void * ary, size_t length)
  {
    EnsureBuffer(length);

    memcpy(buffer + offset, ary, length);

    this->offset += length;

    return length;
  }
}