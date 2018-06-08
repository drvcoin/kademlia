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


// --- temp ---

#include <stdbool.h>
#include <netdb.h>
#include <sys/un.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <list>
#include <string>
#include <stdint.h>
#include <stdlib.h>

// --- temp ---

namespace kad
{
  TcpTransport::TcpTransport(int reliability)
    : ITransport()
    , reliability(reliability)
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

    //printf("InitSocket %s\n", self.ToString().c_str());

    unsigned int port = (unsigned)self.port;

    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
      printf("ERROR opening socket\n");
      exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);


    int yes=1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
      printf("ERROR on binding\n");
      exit(0);
    }


    listen(sockfd,5);

    this->sockfd = sockfd;

    return sockfd;
  }


  void TcpTransport::Send(ContactPtr target, const void * data, size_t size)
  {
    static std::atomic<uint32_t> packageId{0};

    printf("TcpTransport::Send to %s\n", target->ToString().c_str());


    // TODO: package loss calculation


    // TODO: separate thread


    int sockfd;
    struct sockaddr_in serv_addr;
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
      printf("ERROR opening socket\n");
      exit(0);
    }
  
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = target->addr;
    serv_addr.sin_port = htons(target->port);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    {
      printf("ERROR connecting\n");
      exit(0);
    }


    const Contact & self = Config::ContactInfo();
    Header header;

    header.size = size;
    header.addr = self.addr;
    header.port = self.port;

    UNUSED_RESULT(write(sockfd, &header, sizeof(Header)));
    UNUSED_RESULT(write(sockfd, data, size));

    close(sockfd);
  }


  ContactPtr TcpTransport::Receive(uint8_t ** buffer, size_t * len)
  {
    ContactPtr result = nullptr;

    const Contact & self = Config::ContactInfo();

    int newsockfd;
    struct sockaddr_in cli_addr;
    int sockfd = this->sockfd;
    unsigned int clilen = sizeof(cli_addr);
  
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    printf("TcpTransport::Receive on %s\n", self.ToString().c_str());

    Header header;

    if (read(newsockfd, &header, sizeof(Header)) == sizeof(Header))
    {
      *len = header.size;
      *buffer = new uint8_t[*len];
      if (read(newsockfd, *buffer, *len) == static_cast<ssize_t>(*len))
      {
        result = std::make_shared<Contact>();
        result->addr = header.addr;
        result->port = header.port;
      }
    }
    close(newsockfd);  
    return result;
  }
}
