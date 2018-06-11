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
#include <stdio.h>
#include <thread>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/limits.h>
#include <atomic>
#include "Config.h"
#include "PlatformUtils.h"
#include "TcpTransport.h"

#include<sys/socket.h>
#include <netinet/in.h>


namespace kad
{
  TcpTransport::TcpTransport()
    : ITransport()
  {
    std::srand(std::time(nullptr));

    InitSocket();
  }

  TcpTransport::~TcpTransport()
  {
    if (this->sockfd)
    {
      close(this->sockfd);
    }
  }

  int  TcpTransport::InitSocket()
  {
    const Contact & self = Config::ContactInfo();

    unsigned int port = (unsigned)self.port;

    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
      printf("ERROR opening socket\n");
      return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);


    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
      printf("ERROR on binding\n");
      return -1;
    }


    if (listen(sockfd, SOMAXCONN) < 0)
    {
      printf("ERROR on listening\n");
      return -1;
    }

    this->sockfd = sockfd;

    return 0;
  }


  void TcpTransport::Send(ContactPtr target, const void * data, size_t size)
  {
    static std::atomic<uint32_t> packageId{0};

#ifdef DEBUG
    printf("TcpTransport::Send to %s\n", target->ToString().c_str());
#endif

    // TODO: write in separate thread

    int sockfd;
    struct sockaddr_in serv_addr;
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
      printf("ERROR opening socket\n");
      return;
    }
  
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = target->addr;
    serv_addr.sin_port = htons(target->port);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    {
      printf("ERROR connecting\n");
      return;
    }


    const Contact & self = Config::ContactInfo();
    Header header;

    header.size = htonl(size);
    header.addr = htonl(self.addr);
    header.port = htons(self.port);

    UNUSED_RESULT(write(sockfd, &header, sizeof(Header)));
    UNUSED_RESULT(write(sockfd, data, size));

    close(sockfd);
  }


  ContactPtr TcpTransport::Receive(uint8_t ** buffer, size_t * len)
  {
    ContactPtr result = nullptr;

    int newsockfd;
    struct sockaddr_in cli_addr;
    int sockfd = this->sockfd;
    unsigned int clilen = sizeof(cli_addr);
  
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

#ifdef DEBUG
    const Contact & self = Config::ContactInfo();
    printf("TcpTransport::Receive on %s\n", self.ToString().c_str());
#endif

    Header header;

    if (read(newsockfd, &header, sizeof(Header)) == sizeof(Header))
    {
      *len = ntohl(header.size);
      *buffer = new uint8_t[*len];

      int readret = 0;
      uint32_t totalread = 0;
      uint8_t * dst = *buffer;

      while(totalread < *len)
      {
        readret = read(newsockfd, dst, *len - totalread);

        if (readret > 0)
        {
          dst += readret;
          totalread += readret;
        }
        else if (readret < 0)
        {
          printf("ERROR reading\n");
          break;
        }
      }

      if (totalread == *len)
      {
        result = std::make_shared<Contact>();
        result->addr = ntohl(header.addr);
        result->port = ntohs(header.port);
      }
      else
      {
        delete[] (*buffer);
        *buffer = nullptr;
        *len = 0;
      }
    }
    close(newsockfd);  
    return result;
  }
}
