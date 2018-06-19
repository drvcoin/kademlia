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
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <iterator>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Contact.h"
#include "Config.h"
#include "TransportFactory.h"
#include "LinuxFileTransport.h"
#include "TcpTransport.h"
#include "Digest.h"
#include "Kademlia.h"

#include <arpa/inet.h>
#include <json/json.h>

using namespace kad;


class TransportFactoryImpl : public TransportFactory
{
public:

  std::unique_ptr<ITransport> Create() override
  {
    return std::unique_ptr<ITransport>(new TcpTransport());
  }
};


static void SetValue(Kademlia & controller, const std::string & keyStr, const std::string & path, uint64_t version, uint32_t ttl)
{
  sha1_t digest;
  Digest::Compute(keyStr.c_str(), keyStr.size(), digest);

  KeyPtr key = std::make_shared<Key>(digest);

  uint8_t * buffer = nullptr;

  size_t size = 0;

  FILE * file = fopen(path.c_str(), "r");

  if (file)
  {
    fseek(file, 0, SEEK_END);

    size = static_cast<size_t>(std::max<long>(0, ftell(file)));

    if (size > 0)
    {
      rewind(file);

      buffer = new uint8_t[size];

      UNUSED_RESULT(fread(buffer, 1, size, file));
    }

    fclose(file);
  }

  if (size == 0)
  {
    printf("ERROR: file not found or empty.\n");
    return;
  }

  auto data = std::make_shared<Buffer>(buffer, size, false, true);

  auto result = AsyncResultPtr(new AsyncResult<bool>());

  controller.Store(key, data, ttl, version, result);

  result->Wait();

  if (!AsyncResultHelper::GetResult<bool>(result.get()))
  {
    printf("ERROR: failed to set value.\n");
  }
}


static void GetValue(Kademlia & controller, const std::string & keyStr)
{
  sha1_t digest;
  Digest::Compute(keyStr.c_str(), keyStr.size(), digest);

  KeyPtr key = std::make_shared<Key>(digest);

  auto result = AsyncResultPtr(new  AsyncResult<BufferPtr>());

  controller.FindValue(key, result);

  result->Wait();

  auto buffer = AsyncResultHelper::GetResult<BufferPtr>(result.get());

  if (buffer && buffer->Size() > 0)
  {
    printf("%s\n", std::string(reinterpret_cast<const char *>(buffer->Data()), buffer->Size()).c_str());
  }
}


static void Ping(Kademlia & controller, const std::string & keyStr)
{
  sha1_t digest;
  Digest::Compute(keyStr.c_str(), keyStr.size(), digest);

  KeyPtr key = std::make_shared<Key>(digest);

  auto result = AsyncResultPtr(new AsyncResult<bool>());

  controller.Ping(key, result);

  result->Wait();

  if (AsyncResultHelper::GetResult<bool>(result.get()))
  {
    printf("Success\n");
  }
  else
  {
    printf("Unknown or unreachable node.\n");
  }
}


static void Locate(Kademlia & controller, const std::string & keyStr)
{
  sha1_t digest;
  Digest::Compute(keyStr.c_str(), keyStr.size(), digest);

  KeyPtr key = std::make_shared<Key>(digest);

  auto rtn = new AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>>();

  auto result = AsyncResultPtr(rtn);

  controller.FindNode(key, result);

  result->Wait();

  ContactPtr contact = nullptr;

  for (const auto & node : rtn->GetResult())
  {
    if (*node.first == *key)
    {
      contact = node.second;
      break;
    }
  }

  if (contact)
  {
    printf("Found node: %s. Pinging... ", contact->ToString().c_str());

    auto rtn2 = new AsyncResult<bool>();

    result = AsyncResultPtr(rtn2);

    controller.Ping(contact, result);

    result->Wait();

    if (rtn2->GetResult())
    {
      printf("Success\n");
    }
    else
    {
      printf("Failed\n");
    }
  }
  else
  {
    printf("Node not found.\n");
  }
}


static void GetHash(const std::string & keyStr)
{
  sha1_t digest;
  Digest::Compute(keyStr.c_str(), keyStr.size(), digest);

  Key key(digest);

  printf("%s\n", key.ToString().c_str());
}


static void GetNode(Kademlia & controller, const std::string & keyStr)
{
  sha1_t digest;
  Digest::Compute(keyStr.c_str(), keyStr.size(), digest);

  KeyPtr key = std::make_shared<Key>(digest);

  auto rtn = new AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>>();

  auto result = AsyncResultPtr(rtn);

  controller.FindNode(key, result);

  result->Wait();

  printf("{\n  nodes: {\n");

  auto nodes = rtn->GetResult();

  bool first = true;

  for (const auto & node : nodes)
  {
    if (first)
    {
      first = false;
    }
    else
    {
      printf(",\n");
    }

    printf("    { \"key\" : \"%s\", \"contact\" : \"%s\" }", node.first->ToString().c_str(), node.second->ToString().c_str());
  }

  printf("\n  }\n  \"count\" : %u\n]\n", (unsigned)nodes.size());
}


static void PrintNodes(Kademlia & controller)
{
  controller.PrintNodes();
}


static void InitKey(const char * rootPath)
{
  sha1_t digest;
  Digest::Compute(rootPath, strlen(rootPath), digest);

  KeyPtr key = std::make_shared<Key>(digest);

  FILE * file = fopen((std::string(rootPath) + "/key").c_str(), "w");

  if (file)
  {
    fwrite(key->Buffer(), 1, Key::KEY_LEN, file);
    fclose(file);
  }
}


static void InitBuckets(const char * rootPath)
{
  printf("InitBuckets DISABLED\n");
  return;

  std::string bucketsPath = std::string(rootPath) + "/contacts";

  FILE * file = fopen(bucketsPath.c_str(), "r");

  if (file)
  {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    if (size > 0)
    {
      return;
    }
  }

  file = fopen(bucketsPath.c_str(), "w");

  if (file)
  {
    sha1_t digest;

    Digest::Compute("root", sizeof("root") - 1, digest);

    fwrite(digest, 1, sizeof(digest), file);

    Contact contact;
    contact.addr = (long)inet_addr("127.0.0.1");
    contact.port = (short) 1100;

    fwrite(&contact, 1, sizeof(contact), file);

    fclose(file);
  }
}

static void InitBucketsJson(const char * rootPath)
{
  std::string bucketsPath = std::string(rootPath) + "/contacts.json";

  FILE * file = fopen(bucketsPath.c_str(), "r");

  if (file)
  {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    if (size > 0)
    {
      return;
    }
  }

  file = fopen(bucketsPath.c_str(), "w");

  if (file)
  {
    sha1_t digest;

    Digest::Compute("root", sizeof("root") - 1, digest);

//    fwrite(digest, 1, sizeof(digest), file);

    Contact contact;
    contact.addr = (long)inet_addr("127.0.0.1");
    contact.port = (short) 1100;

//    fwrite(&contact, 1, sizeof(contact), file);

    Json::Value root;

    Json::Value node;
    node["node"] = Digest::ToString(digest);
    node["endpoints"].append(contact.ToString());

    root.append(node);

    Json::StyledWriter jw;

    std::string json = jw.write(root);
    printf("%s\n", json.c_str());
    fwrite(json.c_str(), 1, json.size(), file);

    fclose(file);
  }
}


static void Initialize(const char * rootPath)
{
  mkdir(rootPath, 0755);

  InitKey(rootPath);

  InitBuckets(rootPath);
  InitBucketsJson(rootPath);
}


static void SetVerbose(const std::string & option)
{
  if (option == "on")
  {
    Config::SetVerbose(true);
  }
  else if (option == "off")
  {
    Config::SetVerbose(false);
  }
  else
  {
    printf("ERROR: Unknown verbose option.\n");
  }
}


static void RecordReady(const Contact & contact)
{
  char path[1024];
  sprintf(path, "/tmp/test-dht/%u.%u.%u.%u-%u",
    (unsigned)(contact.addr & 0xFF),
    (unsigned)((contact.addr >> 8) & 0xFF),
    (unsigned)((contact.addr >> 16) & 0xFF),
    (unsigned)((contact.addr >> 24) & 0xFF),
    (unsigned)contact.port
  );

  FILE * file = fopen(path, "w");
  if (file)
  {
    fwrite("1", 1, 1, file);
    fclose(file);
  }
}


int main(int argc, char ** argv)
{
  if (argc < 4)
  {
    printf("%s <path> <addr> <port>\n", argv[0]);
    return -1;
  }

  const char * rootPath = argv[1];

  Initialize(rootPath);

  SetVerbose("on");

  Contact self;
  self.addr = (long)inet_addr(argv[2]);
  self.port = (short)atoi(argv[3]);

  Config::Initialize(rootPath);

  Key selfKey{Config::NodeId()->Buffer()};

  Config::Initialize(selfKey, self);

  TransportFactory::Reset(new TransportFactoryImpl());

  printf("Initializing...\n");

  Kademlia controller;

  controller.Initialize();

  while(!controller.IsReady())
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  RecordReady(self);

  printf("Initialization complete. Waiting for commands.\n");

  while (true)
  {
    printf(">");

    std::string line;

    std::getline(std::cin, line);

    std::vector<std::string> words;

    std::istringstream iss(line);

    std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(words));

    bool handled = true;

    if (words.size() >= 3 && words[0] == "set")
    {
      uint64_t version = 0;
      uint32_t ttl = (uint32_t)(-1);

      if (words.size() > 3)
      {
        version = (uint64_t)strtoull(words[3].c_str(), nullptr, 10);
      }

      if (words.size() > 4)
      {
        ttl = (uint32_t)strtoul(words[4].c_str(), nullptr, 10);
      }

      SetValue(controller, words[1], words[2], version, ttl);
    }
    else if (words.size() == 2 && words[0] == "get")
    {
      GetValue(controller, words[1]);
    }
    else if (words.size() == 2 && words[0] == "ping")
    {
      Ping(controller, words[1]);
    }
    else if (words.size() == 2 && words[0] == "locate")
    {
      Locate(controller, words[1]);
    }
    else if (words.size() == 2 && words[0] == "verbose")
    {
      SetVerbose(words[1]);
    }
    else if (words.size() == 2 && words[0] == "hash")
    {
      GetHash(words[1]);
    }
    else if (words.size() == 2 && words[0] == "node")
    {
      GetNode(controller, words[1]);
    }
    else if (words.size() == 1 && words[0] == "print")
    {
      PrintNodes(controller);
    }
    else if (words.size() != 0)
    {
      handled = false;
    }

    if (!handled)
    {
      printf("ERROR: unknown command.\n");
    }
  }

  return 0;
}
