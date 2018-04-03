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

#include "protocol/FindNode.h"
#include "protocol/FindNodeResponse.h"
#include "KBuckets.h"
#include "Package.h"
#include "Config.h"
#include "PackageDispatcher.h"
#include "FindNodeAction.h"

namespace kad
{
  FindNodeAction::FindNodeAction(Thread * owner, PackageDispatcher * dispatcher)
    : Action(owner, dispatcher)
  {
  }


  void FindNodeAction::Initialize(KeyPtr target, const std::vector<std::pair<KeyPtr, ContactPtr>> & nodes)
  {
    this->target = target;

    for (const auto & node : nodes)
    {
      KeyPtr distance = std::make_shared<Key>(target->GetDistance(*node.first));

      this->candidates[distance] = node;
    }
  }


  bool FindNodeAction::Start()
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


  void FindNodeAction::GetResult(std::vector<std::pair<KeyPtr, ContactPtr>> & result)
  {
    if (!this->IsCompleted())
    {
      return;
    }

    for (const auto & node : this->candidates)
    {
      if (result.size() >= KBuckets::SizeK)
      {
        break;
      }

      if (this->validated.find(node.second.first) != this->validated.end())
      {
        result.emplace_back(std::move(node.second));
      }
    }
  }


  void FindNodeAction::SendCandidate(std::pair<KeyPtr, ContactPtr> candidate)
  {
    THREAD_ENSURE(this->owner, SendCandidate, candidate);

    using namespace std::placeholders;

    this->validating.emplace(candidate.first);

    protocol::FindNode * findNode = new protocol::FindNode();

    findNode->SetKey(this->target);

    PackagePtr package = std::make_shared<Package>(Package::PackageType::Request, Config::NodeId(), candidate.second, std::unique_ptr<Instruction>(findNode));

    this->dispatcher->Send(package, std::bind(&FindNodeAction::OnResponse, this, candidate.first, _1, _2));
  }


  void FindNodeAction::OnResponse(KeyPtr key, PackagePtr request, PackagePtr response)
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
        Instruction * instr = response->GetInstruction();

        if (instr && instr->Code() == OpCode::FIND_NODE_RESPONSE)
        {
          this->validated[key] = request->Target();

          protocol::FindNodeResponse * findNodeResponse = static_cast<protocol::FindNodeResponse *>(instr);

          for (const auto & node : findNodeResponse->Nodes())
          {
            KeyPtr distance = std::make_shared<Key>(this->target->GetDistance(*node.first));

            this->candidates[distance] = node;
          }
        }
      }

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

        if (this->onComplete)
        {
          this->onComplete(this->onCompleteSender, this);
        }

        delete this;
      }
    }
  }
}