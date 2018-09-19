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

#include "protocol/QueryLog.h"
#include "protocol/QueryLogResponse.h"
#include "protocol/FindNodeResponse.h"
#include "Package.h"
#include "Config.h"
#include "KBuckets.h"
#include "PackageDispatcher.h"
#include "QueryLogAction.h"

namespace kad
{
  QueryLogAction::QueryLogAction(Thread * owner, PackageDispatcher * dispatcher)
    : Action(owner, dispatcher)
  {
  }


  void QueryLogAction::Initialize(KeyPtr target, std::string query, const std::vector<std::pair<KeyPtr, ContactPtr>> & nodes)
  {
    this->target = target;
    this->query = query;

    for (const auto & node : nodes)
    {
      KeyPtr distance = std::make_shared<Key>(target->GetDistance(*node.first));

      this->candidates[distance] = node;
    }
  }


  bool QueryLogAction::Start()
  {
    size_t idx = 0;

    for (const auto & candidate : this->candidates)
    {
      if (idx++ >= Config::Parallelism())
      {
        break;
      }

      this->SendCandidate(candidate.second);
    }

    return idx > 0;
  }


  BufferPtr QueryLogAction::GetResult() const
  {
    if (!this->IsCompleted())
    {
      return nullptr;
    }

    return this->result;
  }


  uint64_t QueryLogAction::Version() const
  {
    return this->version;
  }


  uint32_t QueryLogAction::TTL() const
  {
    return this->ttl;
  }


  bool QueryLogAction::GetMissedNode(std::pair<KeyPtr, ContactPtr> & result) const
  {
    if (this->missed.empty())
    {
      return false;
    }

    result = this->missed.begin()->second;

    return true;
  }


  void QueryLogAction::SendCandidate(std::pair<KeyPtr, ContactPtr> candidate)
  {
    THREAD_ENSURE(this->owner, SendCandidate, candidate);

    using namespace std::placeholders;

    this->validating.emplace(candidate.first);

    protocol::QueryLog * findValue = new protocol::QueryLog();

    findValue->SetKey(this->target);

    findValue->query = this->query;

    findValue->limit = this->limit;

    PackagePtr package = std::make_shared<Package>(Package::PackageType::Request, Config::NodeId(), candidate.second, std::unique_ptr<Instruction>(findValue));

    this->dispatcher->Send(package, std::bind(&QueryLogAction::OnResponse, this, candidate.first, _1, _2));
  }


  void QueryLogAction::OnResponse(KeyPtr key, PackagePtr request, PackagePtr response)
  {
    THREAD_ENSURE(this->owner, OnResponse, key, request, response);

    this->validating.erase(key);

    if (!this->IsCompleted())
    {
      if (!response)
      {
        this->offline.emplace(key);
      }
      else
      {
        this->validated[key] = request->Target();

        Instruction * instr = response->GetInstruction();

        if (instr->Code() == OpCode::QUERY_LOG_RESPONSE)
        {
          protocol::QueryLogResponse * findValueResponse = static_cast<protocol::QueryLogResponse *>(instr);

          this->result = findValueResponse->Data();

          this->version = findValueResponse->Version();

          this->ttl = findValueResponse->TTL();

          if (this->result)
          {
            this->Complete();
          }
        }
        else if (instr->Code() == OpCode::FIND_NODE_RESPONSE)
        {
          this->missed[std::make_shared<Key>(this->target->GetDistance(*key))] = std::make_pair(key, request->Target());

          protocol::FindNodeResponse * findNodeResponse = static_cast<protocol::FindNodeResponse *>(instr);

          for (const auto & node : findNodeResponse->Nodes())
          {
            KeyPtr distance = std::make_shared<Key>(this->target->GetDistance(*node.first));

            this->candidates[distance] = node;
          }
        }
      }
    }

    if (!this->IsCompleted())
    {
      size_t idx = 0;

      for (const auto & candidate : this->candidates)
      {
        if (this->offline.find(candidate.second.first) != this->offline.end())
        {
          continue;
        }

        if ((idx++) >= KBuckets::SizeK)
        {
          break;
        }

        if (this->validated.find(candidate.second.first) == this->validated.end() &&
            this->validating.find(candidate.second.first) == this->validating.end())
        {
          this->SendCandidate(candidate.second);
          break;
        }
      }

      if (this->validating.empty())
      {
        this->Complete();
      }
    }

    if (this->IsCompleted())
    {
      if (this->onComplete)
      {
        this->onComplete(this->onCompleteSender, this);
        this->onComplete = nullptr;
      }

      if (this->validating.empty())
      {
        // No more on-fly requests. Safe to destroy the handler.
        delete this;
      }
    }
  }
}
