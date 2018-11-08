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

#include <assert.h>
#include "protocol/Protocol.h"
#include <drive/kad/EventLoop.h>
#include <drive/kad/KBuckets.h>
#include <drive/kad/Thread.h>
#include <drive/kad/PackageDispatcher.h>
#include <drive/kad/FindNodeAction.h>
#include <drive/kad/FindValueAction.h>
#include <drive/kad/QueryAction.h>
#include <drive/kad/StoreAction.h>
#include <drive/kad/QueryLogAction.h>
#include <drive/kad/StoreLogAction.h>
#include <drive/kad/PingAction.h>
#include <drive/kad/Storage.h>
#include <drive/kad/Config.h>
#include <drive/kad/Timer.h>
#include <drive/kad/Kademlia.h>

#include <fstream>
#include <json/json.h>
#include <arpa/inet.h>


namespace kad
{
  Kademlia::Kademlia()
    : kBuckets(nullptr)
  {
    this->kBuckets = std::unique_ptr<KBuckets>(new KBuckets(Config::NodeId()));

    this->thread = std::unique_ptr<Thread>(new Thread("Main"));

    this->dispatcher = std::unique_ptr<PackageDispatcher>(new PackageDispatcher(this->thread.get()));

    using namespace std::placeholders;

    this->dispatcher->SetRequestHandler(std::bind(&Kademlia::OnRequest, this, _1, _2));

    this->dispatcher->SetContactHandler(std::bind(&Kademlia::OnMessage, this, _1, _2));

    this->refreshTimer = std::unique_ptr<Timer>(new Timer());
  }

  Kademlia::~Kademlia()
  {
    this->SaveBuckets();
  }


  void Kademlia::Initialize()
  {
    THREAD_ENSURE(this->thread.get(), Initialize);

    if (!this->InitBuckets())
    {
      // TODO: error handling
      return;
    }

    Storage::Persist()->Initialize(true);
    Storage::Cache()->Initialize(false);
    Storage::Log()->Initialize(true);

    std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

    // this->kBuckets->FindClosestContacts(Config::NodeId(), nodes);
    this->kBuckets->GetAllContacts(nodes);

    if (nodes.empty())
    {
      // This is the initial root node
      this->OnInitialized();
      return;
    }

    auto result = AsyncResultPtr(new AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>>());

    this->FindNode(Config::NodeId(), result,
      [this](AsyncResultPtr result)
      {
        auto rtn = dynamic_cast<AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>> *>(result.get());
        if (rtn && rtn->GetResult().size() > 0)
        {
          this->OnInitPing(
            new std::vector<std::pair<KeyPtr, ContactPtr>>(std::move(rtn->GetResult())),
            new std::set<KeyPtr, KeyCompare>(),
            new std::set<KeyPtr, KeyCompare>()
          );
        }
        else
        {
          this->OnInitialized();
        }
      }
    );
  }


  void Kademlia::OnInitPing(const std::vector<std::pair<KeyPtr, ContactPtr>> * targets, std::set<KeyPtr, KeyCompare> * validating, std::set<KeyPtr, KeyCompare> * validated)
  {
    THREAD_ENSURE(this->thread.get(), OnInitPing, targets, validating, validated);

    for (const auto & node : (*targets))
    {
      if (validating->size() >= Config::Parallelism())
      {
        break;
      }

      if (validated->find(node.first) == validated->end() && validating->find(node.first) == validating->end())
      {
        auto result = AsyncResultPtr(new AsyncResult<bool>());

        auto key = node.first;

        validating->emplace(key);

        this->Ping(node.second, result,
          [this, key, targets, validating, validated](AsyncResultPtr result)
          {
            validating->erase(key);

            validated->emplace(key);

            auto rtn = dynamic_cast<AsyncResult<bool> *>(result.get());

            if (!rtn || !rtn->GetResult())
            {
              // Mark the node as unreachable
              this->kBuckets->EraseContact(key);
            }

            if (validated->size() < targets->size())
            {
              this->OnInitPing(targets, validating, validated);
            }
            else
            {
              delete targets;
              delete validating;
              delete validated;

              this->OnInitialized();
            }
          }
        );
      }
    }
  }


  void Kademlia::OnInitialized()
  {
    this->ready = true;

    using namespace std::placeholders;

    this->refreshTimer->Reset(
      Config::RefreshTimerInterval(),
      true,
      std::bind(&Kademlia::OnRefreshTimer, this, _1, _2),
      this,
      nullptr,
      this->thread.get()
    );
  }


  void Kademlia::OnRefreshTimer(void * sender, void * args)
  {
    THREAD_ENSURE(this->thread.get(), OnRefreshTimer, sender, args);

    auto targets = std::make_shared<std::vector<KeyPtr>>();

    this->kBuckets->GetRefreshTargets(*(Config::NodeId()), std::chrono::milliseconds(Config::RefreshInterval()), *targets);

    this->OnRefresh(targets, 0);
  }


  void Kademlia::OnRefresh(std::shared_ptr<std::vector<KeyPtr>> targets, size_t idx)
  {
    if (idx < targets->size())
    {
      this->FindNode((*targets)[idx], nullptr,
        [this, targets, idx](AsyncResultPtr)
        {
          this->OnRefresh(targets, idx + 1);
        },
        true
      );
    }
    else
    {
      // Refresh bucket completes. Now start to replicate old data
      targets->clear();

      // Remove expired persistent data since we do not need to replicate those
      Storage::Persist()->Invalidate();

      Storage::Persist()->GetIdleKeys(*targets, Config::ReplicateTTL());

      this->OnReplicate(targets, 0);

      SaveBuckets();
    }
  }


  void Kademlia::OnReplicate(std::shared_ptr<std::vector<KeyPtr>> targets, size_t idx)
  {
    if (idx < targets->size())
    {
      bool async = false;

      auto key = (*targets)[idx];

      auto buffer = Storage::Persist()->Load(key);

      Storage::Persist()->UpdateTimestamp(key);

      if (buffer)
      {
        std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

        this->kBuckets->FindClosestContacts(key, nodes, true);

        if (nodes.size() > 0)
        {
          auto store = std::unique_ptr<StoreAction>(new StoreAction(this->thread.get(), this->dispatcher.get()));

          uint64_t version;
          int64_t ttl;
          if (Storage::Persist()->GetVersion(key, &version) && Storage::Persist()->GetTTL(key, &ttl))
          {
            store->Initialize(nodes, key, version, buffer, ttl, false);

            store->SetOnCompleteHandler(
              [this, targets, idx](void * sender, void * args)
              {
                this->OnReplicate(targets, idx + 1);
              },
              this
            );

            if (store->Start())
            {
              store.release();
            }

            async = true;
          }
        }
      }

      if (!async)
      {
        this->OnReplicate(targets, idx + 1);
      }
    }
    else
    {
      // Replicate completes. Clean up expired cache
      Storage::Cache()->Invalidate();
    }
  }


  void Kademlia::FindNode(KeyPtr target, AsyncResultPtr result, CompleteHandler handler, bool restrictBucket)
  {
    THREAD_ENSURE(this->thread.get(), FindNode, target, result, handler);

    std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

    this->kBuckets->FindClosestContacts(target, nodes, restrictBucket);

    if (nodes.empty())
    {
      if (result)
      {
        result->Complete();
      }

      if (handler)
      {
        handler(result);
      }

      return;
    }

    this->kBuckets->UpdateLookupTime(target);

    auto action = std::unique_ptr<FindNodeAction>(new FindNodeAction(this->thread.get(), this->dispatcher.get()));

    action->Initialize(target, nodes);

    action->SetOnCompleteHandler(
      [this, result, handler](void * _sender, void * _args)
      {
        auto action = reinterpret_cast<FindNodeAction *>(_args);

        auto rtn = dynamic_cast<AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>> *>(result.get());

        if (rtn)
        {
          std::vector<std::pair<KeyPtr, ContactPtr>> nodes;
          action->GetResult(nodes);
          rtn->Complete(std::move(nodes));
        }
        else if (result)
        {
          result->Complete();
        }

        if (handler)
        {
          handler(result);
        }
      },
      this
    );

    if (action->Start())
    {
      action.release();
    }
  }


  void Kademlia::FindValue(KeyPtr target, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), FindValue, target, result, handler);

    if (!this->ready)
    {
      return;
    }

    // Try to search locally first to see if we already have the knowledge of the data
    auto buffer = Storage::Persist()->Load(target);
    if (!buffer)
    {
      buffer = Storage::Cache()->Load(target);
    }

    if (buffer)
    {
      auto rtn = dynamic_cast<AsyncResult<BufferPtr> *>(result.get());
      if (rtn)
      {
        rtn->Complete(buffer);
      }
      else if (result)
      {
        result->Complete();
      }

      if (handler)
      {
        handler(result);
      }

      return;
    }

    // Try to search from kademlia network

    std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

    this->kBuckets->FindClosestContacts(target, nodes);

    this->kBuckets->UpdateLookupTime(target);

    auto action = std::unique_ptr<FindValueAction>(new FindValueAction(this->thread.get(), this->dispatcher.get()));

    action->Initialize(target, nodes);

    action->SetOnCompleteHandler(
      [this, target, result, handler](void * _sender, void * _args)
      {
        auto action = reinterpret_cast<FindValueAction *>(_args);

        auto rtn = dynamic_cast<AsyncResult<BufferPtr> *>(result.get());

        auto buffer = action->GetResult();

        if (rtn)
        {
          rtn->Complete(buffer);
        }
        else if (result)
        {
          result->Complete();
        }

        if (buffer)
        {
          std::pair<KeyPtr, ContactPtr> missed;

          if (action->GetMissedNode(missed))
          {
            std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

            nodes.emplace_back(std::move(missed));

            auto store = std::unique_ptr<StoreAction>(new StoreAction(this->thread.get(), this->dispatcher.get()));

            store->Initialize(nodes, target, action->Version(), buffer, action->TTL(), false);

            if (store->Start())
            {
              store.release();
            }
          }
        }

        if (handler)
        {
          handler(result);
        }
      },
      this
    );

    if (action->Start())
    {
      action.release();
    }
  }


  void Kademlia::Query(KeyPtr target, std::string query, uint32_t limit, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), Query, target, query, limit, result, handler);

    if (!this->ready)
    {
      return;
    }

    auto storage = Storage::Persist();
    auto buffer = storage->MatchQuery(query);

    bool completed = false;
    Json::Value root;

    if (buffer)
    {
      char* data = (char*)buffer->Data();

      Json::Reader reader;
      if (reader.parse(data, buffer->Size(), root, false) && root.isArray())
      {
        completed = root.size() >= limit;
      }
    }

    if (completed)
    {
      auto rtn = dynamic_cast<AsyncResult<BufferPtr> *>(result.get());
      if (rtn)
      {
        rtn->Complete(buffer);
      }
      else if (result)
      {
        result->Complete();
      }

      if (handler)
      {
        handler(result);
      }

      return;
    }

    printf("Query: Not found locally, searching in network...\n");

    std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

    this->kBuckets->GetAllContacts(nodes);

    auto action = std::unique_ptr<QueryAction>(new QueryAction(this->thread.get(), this->dispatcher.get()));

    action->Initialize(target, query, nodes);

    action->root = root;

    action->limit = limit;

    action->SetOnCompleteHandler(
      [this, target, result, handler](void * _sender, void * _args)
      {
        auto action = reinterpret_cast<QueryAction *>(_args);

        auto rtn = dynamic_cast<AsyncResult<BufferPtr> *>(result.get());

        Json::FastWriter jw;
        std::string json = jw.write(action->root);

        auto buffer = std::make_shared<Buffer>((uint8_t*)json.c_str(), json.size(), true, true);

        if (buffer)
        {
          rtn->Complete(buffer);
        }
        else if (result)
        {
          result->Complete();
        }

        if (handler)
        {
          handler(result);
        }
      },
      this
    );

    if (action->Start())
    {
      action.release();
    }
  }


  void Kademlia::QueryLogs(KeyPtr target, std::string query, uint32_t limit, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), QueryLogs, target, query, limit, result, handler);

    if (!this->ready)
    {
      return;
    }

    std::vector<std::pair<KeyPtr, ContactPtr>> nodes;
    this->kBuckets->FindClosestContacts(target, nodes);

    bool searchLocal = false;

    if (nodes.size() > 0)
    {
      Key localDistance = target->GetDistance(*Config::NodeId());
      Key closestDistance = target->GetDistance(*(nodes[0]).first);

      searchLocal = localDistance <= closestDistance;
    }
    else
    {
      searchLocal = true;
    }

    if (searchLocal)
    {
      auto storage = Storage::Log();
      auto buffer = storage->MatchQuery(query);
      if (buffer)
      {
        auto rtn = dynamic_cast<AsyncResult<BufferPtr> *>(result.get());
        if (rtn)
        {
          rtn->Complete(buffer);
        }
        else if (result)
        {
          result->Complete();
        }

        if (handler)
        {
          handler(result);
        }

        return;
      }
    }

    auto action = std::unique_ptr<QueryLogAction>(new QueryLogAction(this->thread.get(), this->dispatcher.get()));

    action->Initialize(target, query, nodes);

    action->SetOnCompleteHandler(
      [this, target, result, handler](void * _sender, void * _args)
      {
        auto action = reinterpret_cast<QueryLogAction *>(_args);

        auto rtn = dynamic_cast<AsyncResult<BufferPtr> *>(result.get());

        auto buffer = action->GetResult();

        if (rtn)
        {
          rtn->Complete(buffer);
        }
        else if (result)
        {
          result->Complete();
        }

        if (buffer)
        {
          std::pair<KeyPtr, ContactPtr> missed;

          if (action->GetMissedNode(missed))
          {
            std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

            nodes.emplace_back(std::move(missed));

            auto store = std::unique_ptr<StoreLogAction>(new StoreLogAction(this->thread.get(), this->dispatcher.get()));

            store->Initialize(nodes, target, 0, buffer, 0, false);

            if (store->Start())
            {
              store.release();
            }

          }
        }

        if (handler)
        {
          handler(result);
        }
      },
      this
    );

    if (action->Start())
    {
      action.release();
    }
  }


  void Kademlia::Store(KeyPtr hash, BufferPtr data, uint32_t ttl, uint64_t version, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), Store, hash, data, ttl, version, result, handler);

    if (!this->ready)
    {
      return;
    }

    this->FindNode(hash, AsyncResultPtr(new AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>>()),
      [this, hash, data, ttl, version, result, handler](AsyncResultPtr rtn)
      {
        const auto & nodes = AsyncResultHelper::GetResult<std::vector<std::pair<KeyPtr, ContactPtr>>>(rtn.get());

        auto complete = [result, handler](bool val)
        {
          auto storeResult = dynamic_cast<AsyncResult<bool> *>(result.get());
          if (storeResult)
          {
            storeResult->Complete(val);
          }
          else if (result)
          {
            result->Complete();
          }

          if (handler)
          {
            handler(result);
          }
        };

        if (nodes.empty())
        {
          complete(false);
          return;
        }

        auto action = std::unique_ptr<StoreAction>(new StoreAction(this->thread.get(), this->dispatcher.get()));

        action->Initialize(nodes, hash, version, data, ttl, true);

        action->SetOnCompleteHandler(
          [hash, complete](void * sender, void * args)
          {
            auto action = reinterpret_cast<StoreAction *>(args);

            complete(action->GetResult());
          },
          this
        );

        if (action->Start())
        {
          action.release();
        }
      }
    );
  }


  void Kademlia::Publish(KeyPtr hash, BufferPtr data, uint32_t ttl, uint64_t version, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), Publish, hash, data, ttl, version, result, handler);

    if (!this->ready)
    {
      return;
    }

    this->FindNode(hash, AsyncResultPtr(new AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>>()),
      [this, hash, data, ttl, version, result, handler](AsyncResultPtr rtn)
      {
        const auto & nodes = AsyncResultHelper::GetResult<std::vector<std::pair<KeyPtr, ContactPtr>>>(rtn.get());

        auto complete = [result, handler](bool val)
        {
          auto storeResult = dynamic_cast<AsyncResult<bool> *>(result.get());
          if (storeResult)
          {
            storeResult->Complete(val);
          }
          else if (result)
          {
            result->Complete();
          }

          if (handler)
          {
            handler(result);
          }
        };

        if (nodes.empty())
        {
          complete(false);
          return;
        }

        auto action = std::unique_ptr<StoreAction>(new StoreAction(this->thread.get(), this->dispatcher.get()));

        action->Initialize(nodes, hash, version, data, ttl, true);

        action->SetOnCompleteHandler(
          [hash, complete](void * sender, void * args)
          {
            auto action = reinterpret_cast<StoreAction *>(args);

            complete(action->GetResult());
          },
          this
        );

        if (action->Start())
        {
          action.release();
        }
      }
    );
  }

  void Kademlia::StoreLog(KeyPtr hash, BufferPtr data, uint32_t ttl, uint64_t version, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), StoreLog, hash, data, ttl, version, result, handler);

    if (!this->ready)
    {
      return;
    }

    this->FindNode(hash, AsyncResultPtr(new AsyncResult<std::vector<std::pair<KeyPtr, ContactPtr>>>()),
      [this, hash, data, ttl, version, result, handler](AsyncResultPtr rtn)
      {
        const auto & nodes = AsyncResultHelper::GetResult<std::vector<std::pair<KeyPtr, ContactPtr>>>(rtn.get());

        auto complete = [result, handler](bool val)
        {
          auto storeResult = dynamic_cast<AsyncResult<bool> *>(result.get());
          if (storeResult)
          {
            storeResult->Complete(val);
          }
          else if (result)
          {
            result->Complete();
          }

          if (handler)
          {
            handler(result);
          }
        };

        if (nodes.empty())
        {
          complete(false);
          return;
        }

        auto action = std::unique_ptr<StoreLogAction>(new StoreLogAction(this->thread.get(), this->dispatcher.get()));

        action->Initialize(nodes, hash, version, data, ttl, true);

        action->SetOnCompleteHandler(
          [hash, complete](void * sender, void * args)
          {
            auto action = reinterpret_cast<StoreLogAction *>(args);

            complete(action->GetResult());
          },
          this
        );

        if (action->Start())
        {
          action.release();
        }
      }
    );
  }


  void Kademlia::Ping(ContactPtr target, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), Ping, target, result, handler);

    if (!target)
    {
      return;
    }

    auto action = std::unique_ptr<PingAction>(new PingAction(this->thread.get(), this->dispatcher.get()));

    action->Initialize(target);

    action->SetOnCompleteHandler(
      [result, handler](void * _sender, void * _args)
      {
        auto action = reinterpret_cast<PingAction *>(_args);

        auto rtn = dynamic_cast<AsyncResult<bool> *>(result.get());

        if (rtn)
        {
          rtn->Complete(action->GetResult());
        }
        else if (result)
        {
          result->Complete();
        }

        if (handler)
        {
          handler(result);
        }
      },
      this
    );

    if (action->Start())
    {
      action.release();
    }
  }


  void Kademlia::Ping(KeyPtr target, AsyncResultPtr result, CompleteHandler handler)
  {
    THREAD_ENSURE(this->thread.get(), Ping, target, result, handler);

    if (!target)
    {
      return;
    }

    auto contact = this->kBuckets->FindContact(target);

    if (!contact)
    {
      auto rtn = dynamic_cast<AsyncResult<bool> *>(result.get());

      if (rtn)
      {
        rtn->Complete(false);
      }
      else if (result)
      {
        result->Complete();
      }

      if (handler)
      {
        handler(result);
      }
    }
    else
    {
      this->Ping(contact, result, handler);
    }
  }


  void Kademlia::OnMessage(KeyPtr fromKey, ContactPtr fromContact)
  {
    THREAD_ENSURE(this->thread.get(), OnMessage, fromKey, fromContact);

    ContactPtr origin = this->kBuckets->UpdateContact(fromKey);

    if (origin)
    {
      *origin = *fromContact;
    }
    else
    {
      if (this->kBuckets->GetBucketSize(fromKey) < KBuckets::SizeK)
      {
        this->kBuckets->AddContact(fromKey, fromContact);
      }
      else
      {
        std::vector<std::pair<KeyPtr, ContactPtr>> nodes;
        this->kBuckets->GetOldContacts(fromKey, 1, nodes);

        auto oldKey = nodes[0].first;
        auto result = AsyncResultPtr(new AsyncResult<bool>());

        this->Ping(nodes[0].second, result,
          [this, oldKey, fromKey, fromContact, result](AsyncResultPtr)
          {
            if (!AsyncResultHelper::GetResult<bool>(result.get()))
            {
              this->kBuckets->EraseContact(oldKey);
              this->kBuckets->AddContact(fromKey, fromContact);
            }
          }
        );
      }
    }
  }


  void Kademlia::OnRequest(ContactPtr from, PackagePtr request)
  {
    THREAD_ENSURE(this->thread.get(), OnRequest, from, request);


    Instruction * instr = request->GetInstruction();

    switch (instr->Code())
    {
      case OpCode::PING:
      {
        this->OnRequestPing(from, request);
        break;
      }

      case OpCode::FIND_NODE:
      {
        this->OnRequestFindNode(from, request);
        break;
      }

      case OpCode::FIND_VALUE:
      {
        this->OnRequestFindValue(from, request);
        break;
      }

      case OpCode::QUERY:
      {
        this->OnRequestQuery(from, request);
        break;
      }

      case OpCode::QUERY_LOG:
      {
        this->OnRequestQueryLog(from, request);
        break;
      }

      case OpCode::STORE:
      {
        this->OnRequestStore(from, request);
        break;
      }

      case OpCode::STORE_LOG:
      {
        this->OnRequestStoreLog(from, request);
        break;
      }

      default:
        printf("Unknown instruction received: code=%u\n", static_cast<unsigned>(instr->Code()));
        break;
    }
  }


  void Kademlia::OnRequestPing(ContactPtr from, PackagePtr request)
  {
    this->dispatcher->Send(std::make_shared<Package>(
      Package::PackageType::Response,
      Config::NodeId(),
      request->Id(),
      from,
      std::unique_ptr<Instruction>(new protocol::Pong())
    ));
  }


  void Kademlia::OnRequestFindNode(ContactPtr from, PackagePtr request)
  {
    protocol::FindNode * reqInstr = static_cast<protocol::FindNode *>(request->GetInstruction());

    auto target = reqInstr->Key();

    std::vector<std::pair<KeyPtr, ContactPtr>> result;

    if (target)
    {
      this->kBuckets->FindClosestContacts(target, result);
    }
    else
    {
      this->kBuckets->GetAllContacts(result);
    }

    protocol::FindNodeResponse * resInstr = new protocol::FindNodeResponse();

    for (const auto & node : result)
    {
      resInstr->AddNode(node.first, node.second);
    }

    this->dispatcher->Send(std::make_shared<Package>(Package::PackageType::Response, Config::NodeId(), request->Id(), from, std::unique_ptr<Instruction>(resInstr)));
  }


  void Kademlia::OnRequestFindValue(ContactPtr from, PackagePtr request)
  {
    protocol::FindValue * reqInstr = static_cast<protocol::FindValue *>(request->GetInstruction());

    auto storage = Storage::Persist();
    auto buffer = storage->Load(reqInstr->Key());

    if (!buffer)
    {
      storage = Storage::Cache();
      buffer = storage->Load(reqInstr->Key());
    }

    uint64_t version;
    int64_t ttl;

    if (buffer && storage->GetVersion(reqInstr->Key(), &version) && storage->GetTTL(reqInstr->Key(), &ttl))
    {
      protocol::FindValueResponse * resInstr = new protocol::FindValueResponse();

      resInstr->SetData(buffer);
      resInstr->SetVersion(version);
      resInstr->SetTTL(ttl);

      this->dispatcher->Send(std::make_shared<Package>(Package::PackageType::Response, Config::NodeId(), request->Id(), from, std::unique_ptr<Instruction>(resInstr)));
    }
    else
    {
      this->OnRequestFindNode(from, request);
    }
  }


  void Kademlia::OnRequestQuery(ContactPtr from, PackagePtr request)
  {
    protocol::Query * reqInstr = static_cast<protocol::Query *>(request->GetInstruction());

    auto storage = Storage::Persist();
    auto buffer = storage->MatchQuery(reqInstr->query);


    bool completed = false;
    Json::Value root;

    if (buffer)
    {
      char* data = (char*)buffer->Data();

      Json::Reader reader;

      if (reader.parse(data, buffer->Size(), root, false) && root.isArray())
      {
        completed = root.size() >= reqInstr->limit;
      }
    }

    uint64_t version;
    int64_t ttl;

    storage->GetVersion(reqInstr->Key(), &version);
    storage->GetTTL(reqInstr->Key(), &ttl);

    if (buffer)
    {
      protocol::QueryResponse * resInstr = new protocol::QueryResponse();

      resInstr->SetData(buffer);
      resInstr->SetVersion(version);
      resInstr->SetTTL(ttl);

      this->dispatcher->Send(std::make_shared<Package>(Package::PackageType::Response, Config::NodeId(), request->Id(), from, std::unique_ptr<Instruction>(resInstr)));
    }

    if (!completed)
    {
      this->OnRequestFindNode(from, request);
    }
  }


  void Kademlia::OnRequestQueryLog(ContactPtr from, PackagePtr request)
  {
    protocol::Query * reqInstr = static_cast<protocol::Query *>(request->GetInstruction());

    auto storage = Storage::Log();
    auto buffer = storage->MatchQuery(reqInstr->query);

    uint64_t version;
    int64_t ttl;

    storage->GetVersion(reqInstr->Key(), &version);
    storage->GetTTL(reqInstr->Key(), &ttl);

    if (buffer)
    {
      protocol::QueryLogResponse * resInstr = new protocol::QueryLogResponse();

      resInstr->SetData(buffer);
      resInstr->SetVersion(version);
      resInstr->SetTTL(ttl);

      this->dispatcher->Send(std::make_shared<Package>(Package::PackageType::Response, Config::NodeId(), request->Id(), from, std::unique_ptr<Instruction>(resInstr)));
    }
    else
    {
      this->OnRequestFindNode(from, request);
    }
  }


  void Kademlia::OnRequestStore(ContactPtr from, PackagePtr request)
  {
    protocol::Store * reqInstr = static_cast<protocol::Store *>(request->GetInstruction());

    auto storage = reqInstr->IsOriginal() ? Storage::Persist() : Storage::Cache();

    auto code = protocol::StoreResponse::ErrorCode::SUCCESS;

    uint64_t version;
    if (storage->GetVersion(reqInstr->GetKey(), &version) && version > reqInstr->Version())
    {
      code = protocol::StoreResponse::ErrorCode::OUT_OF_DATE;
    }
    else if (!storage->Save(reqInstr->GetKey(), reqInstr->Version(), reqInstr->Data(), reqInstr->TTL()))
    {
      code = protocol::StoreResponse::ErrorCode::FAILED;
    }

    protocol::StoreResponse * resInstr = new protocol::StoreResponse();

    resInstr->SetResult(code);

    this->dispatcher->Send(std::make_shared<Package>(Package::PackageType::Response, Config::NodeId(), request->Id(), from, std::unique_ptr<Instruction>(resInstr)));
  }


  void Kademlia::OnRequestStoreLog(ContactPtr from, PackagePtr request)
  {
    protocol::StoreLog * reqInstr = static_cast<protocol::StoreLog *>(request->GetInstruction());

    auto storage = Storage::Log();

    auto code = protocol::StoreLogResponse::ErrorCode::SUCCESS;

    if (!storage->SaveLog(reqInstr->GetKey(), reqInstr->Version(), reqInstr->Data(), reqInstr->TTL()))
    {
      code = protocol::StoreLogResponse::ErrorCode::FAILED;
    }

    protocol::StoreLogResponse * resInstr = new protocol::StoreLogResponse();

    resInstr->SetResult(code);

    this->dispatcher->Send(std::make_shared<Package>(Package::PackageType::Response, Config::NodeId(), request->Id(), from, std::unique_ptr<Instruction>(resInstr)));
  }


  bool Kademlia::InitBuckets()
  {
    TSTRING bucketsFilePath = Config::RootPath() + _T(PATH_SEPERATOR_STR) + _T("contacts.json");

    FILE * file = _tfopen(bucketsFilePath.c_str(), _T("r"));
    if (!file)
    {
      bucketsFilePath = Config::DefcontPath();

      file = _tfopen(bucketsFilePath.c_str(), _T("r"));
      if (!file)
      {
        printf("Missing both contacts.json and default_contacts.json\n");
        return false;
      }
    }

    printf("opening file %s\n",bucketsFilePath.c_str());

    std::ifstream ifs(bucketsFilePath);

    std::string content( (std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>() );


    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(content, root, false) || !root.isArray())
    {
      printf("ERROR parsing json\n");
      return false;
    }

    for (uint32_t i = 0; i < root.size(); i++)
    {
      if (!root[i].isObject() ||
          !root[i]["node"].isString() ||
          !root[i]["endpoints"].isArray())
      {
        printf("WARNING: json record in contacts file is wrong\n");
        continue;
      }

      std::string keyStr = root[i]["node"].asString();
      auto key = std::make_shared<Key>();
      key->FromString(keyStr.c_str());

// TODO: taking first endpoint, in future support multiple endpoints
      uint32_t j = 0;
      auto endpoint = root[i]["endpoints"][j];

      if (!endpoint.isString())
      {
        printf("WARNING: contact endpoint is not string\n");
        continue;
      }

      std::string ep = endpoint.asString();

      auto pos = ep.find(':');
      std::string addr = ep.substr(0,pos);
      std::string port = ep.substr(pos+1,ep.size());

      auto contact = std::make_shared<Contact>();
      contact->addr = (long)inet_addr(addr.c_str());
      contact->port = (short)atoi(port.c_str());

      this->kBuckets->AddContact(key, contact);
    }

    fclose(file);

    if (this->kBuckets->Size() == 0)
    {
      printf("WARNING: no initial buckets found. This should only be used on the root node.\n");
    }

    return true;
  }


  void Kademlia::SaveBuckets()
  {
    TSTRING bucketsFilePath = Config::RootPath() + _T(PATH_SEPERATOR_STR) + _T("contacts.json");

    FILE * file = _tfopen(bucketsFilePath.c_str(), _T("w"));

    if (file)
    {
      std::vector<std::pair<KeyPtr, ContactPtr>> entries;

      this->kBuckets->GetAllContacts(entries);

      Json::Value root;

      for (const auto & entry : entries)
      {
        Json::Value node;
        node["node"] = entry.first->ToString();
        node["endpoints"].append(entry.second->ToString());

        root.append(node);
      }

      Json::FastWriter jw;

      std::string json = jw.write(root);
      fwrite(json.c_str(), 1, json.size(), file);

      fclose(file);
    }
  }

  void Kademlia::PrintNodes() const
  {
    if (this->thread.get() != Thread::Current())
    {
      this->thread->Invoke([this](void *, void *) { this->PrintNodes(); });
      return;
    }

    std::vector<std::pair<KeyPtr, ContactPtr>> nodes;

    this->kBuckets->GetAllContacts(nodes);

    printf("{ \"nodes\":\n");

    printf("[\n");

    for (const auto & node : nodes)
    {
      printf(" { \"key\" : \"%s\", \"addr\" : \"%s\" }\n", node.first->ToString().c_str(), node.second->ToString().c_str());
    }

    printf("], \"count\":%llu }\n", (long long unsigned)nodes.size());

  }
}
