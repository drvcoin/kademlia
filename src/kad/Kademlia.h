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


#include <memory>
#include <set>
#include <vector>
#include "Key.h"
#include "Contact.h"
#include "Package.h"
#include "Buffer.h"
#include "AsyncResult.h"
#include "Instruction.h"

namespace kad
{
  class EventLoop;
  class KBuckets;
  class Thread;
  class PackageDispatcher;
  class Timer;

  class Kademlia
  {
  public:

    using CompleteHandler = std::function<void(AsyncResultPtr)>;

  public:

    Kademlia();

    ~Kademlia();

    void Initialize();

    void Store(KeyPtr hash, BufferPtr data, AsyncResultPtr result = nullptr, CompleteHandler handler = nullptr);

    void FindNode(KeyPtr target, AsyncResultPtr result = nullptr, CompleteHandler handler = nullptr, bool restrictBucket = false);

    void FindValue(KeyPtr target, AsyncResultPtr result = nullptr, CompleteHandler handler = nullptr);

    void Ping(ContactPtr target, AsyncResultPtr result = nullptr, CompleteHandler handler = nullptr);

    void Ping(KeyPtr target, AsyncResultPtr result = nullptr, CompleteHandler handler = nullptr);

    bool IsReady() const        { return this->ready; }

    void PrintNodes() const;

  private:

    void OnMessage(KeyPtr fromKey, ContactPtr fromContact);

    void OnRequest(ContactPtr from, PackagePtr request);

    void OnRequestPing(ContactPtr from, PackagePtr request);

    void OnRequestFindNode(ContactPtr from, PackagePtr request);

    void OnRequestFindValue(ContactPtr from, PackagePtr request);

    void OnRequestStore(ContactPtr from, PackagePtr request);

  private:

    void OnInitPing(const std::vector<std::pair<KeyPtr, ContactPtr>> * targets, std::set<KeyPtr, KeyCompare> * validating, std::set<KeyPtr, KeyCompare> * validated);

    bool InitBuckets();

    void SaveBuckets();

    void RefreshBucket(size_t idx, CompleteHandler handler);

    void OnRefreshTimer(void * sender, void * args);

    void OnRefresh(std::shared_ptr<std::vector<KeyPtr>> targets, size_t idx);

    void OnReplicate(std::shared_ptr<std::vector<KeyPtr>> targets, size_t idx);

  private:

    std::unique_ptr<Thread> thread; 

    bool ready = false;

    std::unique_ptr<KBuckets> kBuckets;

    std::unique_ptr<PackageDispatcher> dispatcher;

    std::unique_ptr<Timer> refreshTimer;
  };
}