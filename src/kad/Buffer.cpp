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

#include <string.h>
#include <drive/kad/Buffer.h>

namespace kad
{
  Buffer::Buffer(const uint8_t * buffer, size_t len, bool copy, bool owned)
  {
    if (!copy)
    {
      this->owned = owned;
      this->data = const_cast<uint8_t *>(buffer);
      this->size = len;
    }
    else
    {
      this->owned = true;
      this->data = new uint8_t[len];
      memcpy(this->data, buffer, len);
      this->size = len;
    }
  }


  Buffer::~Buffer()
  {
    if (this->owned && this->data)
    {
      delete[] this->data;
      this->data = nullptr;
    }
  }
}
