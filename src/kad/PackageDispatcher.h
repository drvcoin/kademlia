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

#include <functional>
#include <thread>
#include <map>
#include <chrono>
#include "Contact.h"
#include "Thread.h"
#include "Package.h"
#include "ITransport.h"

namespace kad
{
  class DelayEvent;

  class PackageDispatcher
  {
  public:

    using PackageHandler = std::function<void(PackagePtr request, PackagePtr response)>;

    using RequestHandler = std::function<void(ContactPtr sender, PackagePtr request)>;

    using ContactHandler = std::function<void(KeyPtr, ContactPtr)>;

  private:

    struct Subscription
    {
      PackagePtr request;
      PackageHandler handler;
      int timeout;
    };

    struct SubscriptionId
    {
      Contact contact;
      uint16_t requestId;

     inline bool operator<(const SubscriptionId & value) const
      {
        if (this->requestId < value.requestId)
        {
          return true;
        }
        else if (this->requestId == value.requestId)
        {
          if (this->contact.addr < value.contact.addr)
          {
            return true;
          }
          else if (this->contact.addr == value.contact.addr)
          {
            return this->contact.port < value.contact.port;
          }
        }

        return false;
      }

      inline bool operator==(const SubscriptionId & value) const
      {
        return this->requestId == value.requestId && this->contact.addr == value.contact.addr && this->contact.port == value.contact.port;
      }
    };

    using TimePoint = std::chrono::steady_clock::time_point;

  public:

    explicit PackageDispatcher(Thread * owner = nullptr);

    ~PackageDispatcher();

    void Send(PackagePtr package, PackageHandler onResponse = nullptr, int timeout = 5000);

    void SetRequestHandler(RequestHandler handler);

    void SetContactHandler(ContactHandler handler);

  private:

    void RecvThreadProc();

    static void OnSend(void * sender, void * args);

    static void OnReceive(void * sender, void * args);

    static void OnCheckTimeout(void * sender, void * args);

  private:

    std::unique_ptr<Thread> dispatcherThread = std::unique_ptr<Thread>(new Thread("Dispatcher"));

    std::thread recvThread;

    std::map<SubscriptionId, Subscription *> subscriptions;

    std::map<TimePoint, SubscriptionId> expires;

    std::unique_ptr<ITransport> transport;

    RequestHandler requestHandler = nullptr;

    std::unique_ptr<DelayEvent> delay;

    Thread * owner;

    ContactHandler contactHandler = nullptr;
  };
}