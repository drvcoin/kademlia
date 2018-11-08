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

#include <string>
#include <drive/kad/ITransport.h>

namespace kad
{
  class TcpTransport : public ITransport
  {
  public:

    explicit TcpTransport();

    ~TcpTransport() override;

    void Send(ContactPtr target, const void * data, size_t len) override;

    ContactPtr Receive(uint8_t ** buffer, size_t * len) override;

  private:
#pragma pack(1)
    struct Header
    {

      uint32_t size;
      uint32_t addr;
      uint16_t port;

      std::string ToString() const
      {
        char buffer[128];
        sprintf(buffer, "header=[%d,%08X,%d]", size, addr, port);
        return buffer;
      }

    };
#pragma pack()

    int InitSocket();

    int sockfd;

  private:

    static const std::string transportRoot;

  };
}
