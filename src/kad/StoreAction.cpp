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

#include "Thread.h"
#include "PackageDispatcher.h"
#include "Config.h"
#include "Package.h"
#include "protocol/Store.h"
#include "protocol/StoreResponse.h"
#include "StoreAction.h"


namespace kad
{
  StoreAction::StoreAction(Thread * owner, PackageDispatcher * dispatcher)
    : Action(owner, dispatcher)
  {
  }


  void StoreAction::Initialize(const std::vector<std::pair<KeyPtr, ContactPtr>> & nodes, KeyPtr key, BufferPtr data, uint32_t ttl)
  {
    for (const auto & node : nodes)
    {
      this->targets[node.first] = node.second;
    }

    this->key = key;
    this->data = data;
    this->ttl = ttl;
  }


  bool StoreAction::Start()
  {
    size_t idx = 0;

    for (const auto & target : this->targets)
    {
      if (idx++ >= Config::Parallelism())
      {
        break;
      }

      this->Send(target);
    }

    return idx > 0;
  }


  bool StoreAction::GetResult() const
  {
    return this->IsCompleted() && this->result;
  }


  void StoreAction::Send(std::pair<KeyPtr, ContactPtr> node)
  {
    THREAD_ENSURE(this->owner, Send, node);

    using namespace std::placeholders;

    protocol::Store * instr = new protocol::Store();

    instr->SetKey(this->key);

    instr->SetData(this->data);

    instr->SetTTL(this->ttl);

    PackagePtr package = std::make_shared<Package>(Package::PackageType::Request, Config::NodeId(), node.second, std::unique_ptr<Instruction>(instr));

    this->processing.emplace(node.first);

    this->dispatcher->Send(package, std::bind(&StoreAction::OnResponse, this, node.first, _1, _2));
  }


  void StoreAction::OnResponse(KeyPtr key, PackagePtr request, PackagePtr response)
  {
    THREAD_ENSURE(this->owner, OnResponse, key, request, response);

    this->processing.erase(key);

    this->targets.erase(key);

    if (response)
    {
      Instruction * instr = response->GetInstruction();
      if (instr->Code() == OpCode::STORE_RESPONSE)
      {
        protocol::StoreResponse * res = static_cast<protocol::StoreResponse *>(instr);
        if (res->Result() == protocol::StoreResponse::ErrorCode::SUCCESS)
        {
          this->result = true;
        }
      }
    }

    if (this->targets.empty())
    {
      this->Complete();
    }

    if (!this->IsCompleted())
    {
      for (const auto & node : this->targets)
      {
        if (this->processing.find(node.first) == this->processing.end())
        {
          this->Send(node);
          break;
        }
      }
    }
    else
    {
      if (this->onComplete)
      {
        this->onComplete(this->onCompleteSender, this);
      }

      delete this;
    }
  }
}