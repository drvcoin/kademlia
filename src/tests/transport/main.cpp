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
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iterator>
#include "Config.h"
#include "Instruction.h"
#include "protocol/Ping.h"
#include "protocol/Pong.h"
#include "PackageDispatcher.h"
#include "TransportFactory.h"
#include "LinuxFileTransport.h"

using namespace kad;


class TransportFactoryImpl : public TransportFactory
{
public:

  std::unique_ptr<ITransport> Create() override
  {
    return std::unique_ptr<ITransport>(new LinuxFileTransport(50));
  }
};


static PackageDispatcher * dispatcher = nullptr;

static void onRequest(ContactPtr sender, PackagePtr request)
{
  printf("recv request: id=%u addr=%08X port=%04X\n", request->Id(), (unsigned)sender->addr, (unsigned)sender->port);

  PackagePtr response = std::make_shared<Package>(Package::PackageType::Response, Config::NodeId(), request->Id(), sender, std::unique_ptr<Instruction>(new protocol::Pong()));

  dispatcher->Send(response);
}


static void onResponse(PackagePtr request, PackagePtr response)
{
  if (response)
  {
    printf("recv response: id=%u addr=%08X port=%04X\n", response->Id(), (unsigned)request->Target()->addr, (unsigned)request->Target()->port);
  }
  else
  {
    printf("request lost: id=%u addr=%08X port=%04X\n", request->Id(), (unsigned)request->Target()->addr, (unsigned)request->Target()->port);
  }
}


int main(int argc, char ** argv)
{
  if (argc < 3)
  {
    printf("Usage: %s <addr> <port>\n", argv[0]);
    return -1;
  }

  Contact self;
  self.addr = (long)atol(argv[1]);
  self.port = (short)atoi(argv[2]);

  Key key = {};

  Config::Initialize(key, self);

  TransportFactory::Reset(new TransportFactoryImpl());

  dispatcher = new PackageDispatcher(nullptr);

  dispatcher->SetRequestHandler(onRequest);

  while (true)
  {
    std::string line;

    std::getline(std::cin, line);

    std::vector<std::string> words;

    std::istringstream iss(line);

    std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(words));

    if (words.size() == 3)
    {
      if (words[0] == "send")
      {
        ContactPtr contact = std::make_shared<Contact>();
        contact->addr = atol(words[1].c_str());
        contact->port = (short)atoi(words[2].c_str());

        PackagePtr package = std::make_shared<Package>(Package::PackageType::Request, Config::NodeId(), contact, std::unique_ptr<Instruction>(new protocol::Ping()));
        dispatcher->Send(package, onResponse, 2000);
      }
    }
  }

  delete dispatcher;

  return 0;
}