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

#include <tuple>
#include <vector>
#include "BufferedOutputStream.h"
#include "BufferedInputStream.h"
#include "TransportFactory.h"
#include "Timer.h"
#include "Config.h"
#include "PackageDispatcher.h"

namespace kad
{
  PackageDispatcher::PackageDispatcher(Thread * owner)
    : owner(owner)
  {
    this->timer = std::unique_ptr<Timer>(new Timer());

    this->transport = TransportFactory::Instance()->Create();

    this->recvThread = std::thread(std::bind(&PackageDispatcher::RecvThreadProc, this));
  }


  PackageDispatcher::~PackageDispatcher()
  {
    // TODO: signal and quit the recv thread
    if (this->recvThread.joinable())
    {
      this->recvThread.join();
    }
  }


  void PackageDispatcher::Send(PackagePtr package, PackageHandler onResponse, int timeout)
  {
    Subscription * subscription = new Subscription();
    subscription->request = package;
    subscription->handler = onResponse;
    subscription->timeout = timeout;

    this->dispatcherThread->BeginInvoke(&PackageDispatcher::OnSend, this, subscription);
  }


  void PackageDispatcher::SetRequestHandler(RequestHandler handler)
  {
    this->requestHandler = handler;
  }


  void PackageDispatcher::SetContactHandler(ContactHandler handler)
  {
    this->contactHandler = handler;
  }


  void PackageDispatcher::RecvThreadProc()
  {
    while (true)
    {
      uint8_t * buffer = nullptr;
      size_t size = 0;

      ContactPtr contact = this->transport->Receive(&buffer, &size);

      if (!contact || !buffer)
      {
        continue;
      }

      auto args = new std::tuple<ContactPtr, uint8_t *, size_t>(contact, buffer, size);

      this->dispatcherThread->BeginInvoke(&PackageDispatcher::OnReceive, this, args);
    }
  }


  void PackageDispatcher::OnSend(void * sender, void * args)
  {
    PackageDispatcher * _this = reinterpret_cast<PackageDispatcher *>(sender);
    Subscription * subscription = reinterpret_cast<Subscription *>(args);

    bool managed = false;

    if (subscription->request->Type() == Package::PackageType::Request && (subscription->handler || subscription->timeout > 0))
    {
      SubscriptionId id;
      id.contact = *(subscription->request->Target());
      id.requestId = subscription->request->Id();

      _this->subscriptions[id] = subscription;
      managed = true;

      if (subscription->timeout > 0)
      {
        _this->expires[std::chrono::steady_clock::now() + std::chrono::milliseconds(subscription->timeout)] = id;

        auto first = _this->expires.begin();

        if (first->second == id)
        {
          _this->timer->Reset(subscription->timeout, false, &PackageDispatcher::OnCheckTimeout, _this, nullptr, Thread::Current());
        }
      }
    }

#ifdef DEBUG
    if (Config::Verbose())
    {
      printf("[SEND] target=%s id=%u\n", subscription->request->Target()->ToString().c_str(), subscription->request->Id());
      subscription->request->GetInstruction()->Print();
    }
#endif

    BufferedOutputStream buffer;
    if (subscription->request->Serialize(buffer))
    {
      _this->transport->Send(subscription->request->Target(), buffer.Buffer(), buffer.Offset());
    }

    if (!managed && subscription)
    {
      delete subscription;
    }
  }


  void PackageDispatcher::OnReceive(void * sender, void * args)
  {
    auto _this = reinterpret_cast<PackageDispatcher *>(sender);
    auto info = reinterpret_cast<std::tuple<ContactPtr, uint8_t *, size_t> *>(args);

    ContactPtr contact = std::get<0>(*info);
    uint8_t * buffer = std::get<1>(*info);
    size_t size = std::get<2>(*info);

    delete info;

    BufferedInputStream input(buffer, size);
    PackagePtr package = Package::Deserialize(contact, input);
    delete[] buffer;

    if (!package)
    {
      return;
    }

#ifdef DEBUG
    if (Config::Verbose())
    {
      printf("[RECV] from=%s id=%u\n", contact->ToString().c_str(), package->Id());
      package->GetInstruction()->Print();
    }
#endif

    if (_this->contactHandler)
    {
      auto from = package->From();

      if (_this->owner)
      {
        auto handler = _this->contactHandler;
        _this->owner->BeginInvoke([handler, from, contact](void *, void *) { handler(from, contact); });
      }
      else
      {
        _this->contactHandler(from, contact);
      }
    }

    if (package->Type() == Package::PackageType::Response)
    {
      SubscriptionId id;
      id.contact = *contact;
      id.requestId = package->Id();

      auto iter = _this->subscriptions.find(id);
      if (iter == _this->subscriptions.end())
      {
        // No one is expecting this response
        return;
      }

      auto subscription = std::shared_ptr<Subscription>(iter->second);

      _this->subscriptions.erase(iter);

      if (subscription->handler)
      {
        if (_this->owner)
        {
          _this->owner->BeginInvoke([subscription, package](void *, void *) { subscription->handler(subscription->request, package); });
        }
        else
        {
          subscription->handler(subscription->request, package);
        }
      }
    }
    else if (_this->requestHandler)
    {
      _this->requestHandler(contact, package);
    }
  }


  void PackageDispatcher::OnCheckTimeout(void * sender, void * args)
  {
    auto _this = reinterpret_cast<PackageDispatcher *>(sender);

    auto now = std::chrono::steady_clock::now();

    std::vector<TimePoint> expired;

    for (const auto & item : _this->expires)
    {
      if (item.first > now)
      {
        break;
      }
      else
      {
        expired.emplace_back(item.first);
      }
    }

    for (const auto & time : expired)
    {
      auto iterExpires = _this->expires.find(time);

      auto iterSubscriptions = _this->subscriptions.find(iterExpires->second);

      if (iterSubscriptions != _this->subscriptions.end())
      {
        // A package has expired
        auto subscription = std::shared_ptr<Subscription>(iterSubscriptions->second);

        _this->subscriptions.erase(iterSubscriptions);

        if (subscription->handler)
        {
          if (_this->owner)
          {
            _this->owner->BeginInvoke([subscription](void *, void *) { subscription->handler(subscription->request, nullptr); });
          }
          else
          {
            subscription->handler(subscription->request, nullptr);
          }
        }
      }

      _this->expires.erase(iterExpires);
    }

    auto first = _this->expires.begin();

    if (first != _this->expires.end())
    {
      _this->timer->Reset(
        std::chrono::duration_cast<std::chrono::milliseconds>(first->first - now).count(),
        false,
        &PackageDispatcher::OnCheckTimeout,
        _this,
        nullptr,
        Thread::Current()
      );
    }
  }
}
