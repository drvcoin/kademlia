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

#include <atomic>
#include <thread>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <drive/common/LockFreeQueue.h>

namespace kad
{
  using EventHandler = std::function<void(void *, void *)>;

  class EventLoop
  {
  private:

    struct EventHandlerEntry
    {
      EventHandler handler = nullptr;
      void * sender = nullptr;
      void * args = nullptr;
      std::shared_ptr<std::atomic<bool>> completed;
      std::shared_ptr<std::mutex> mutex;
      std::shared_ptr<std::condition_variable> cond;
    };

  public:

    ~EventLoop();

    void Run();

    void Quit();

    void BeginInvoke(EventHandler handler, void * sender = nullptr, void * args = nullptr);

    void Invoke(EventHandler handler, void * sender = nullptr, void * args = nullptr);

    bool IsRunning() const { return this->alive; }

  private:

    std::mutex mutex;

    std::condition_variable cond;

    std::atomic<bool> quit{false};

    std::atomic<bool> alive{false};

    bdfs::LockFreeQueue<EventHandlerEntry *> events;
  };
}
