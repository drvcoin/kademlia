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

#include <thread>
#include <memory>
#include <set>
#include <mutex>
#include <atomic>
#include "EventLoop.h"

namespace kad
{
  class Thread
  {
  public:

    static Thread * Current();

  public:

    explicit Thread(const char * name = nullptr);

    ~Thread();

    bool IsRunning() const;

    void BeginInvoke(EventHandler handler, void * sender = nullptr, void * args = nullptr);

    void Invoke(EventHandler handler, void * sender = nullptr, void * args = nullptr);

  private:

    static void ThreadProc(Thread * _this, EventLoop * eventLoop);
    
  private:

    static thread_local Thread * current;

    static std::set<Thread *> threads;

    static std::mutex threadsMutex;

  private:

    std::thread thread;

    std::unique_ptr<EventLoop> eventLoop;

    const char * name;
  };


  #define THREAD_ENSURE(t, func, ...) \
  { \
    if (t != kad::Thread::Current()) \
    { \
      t->BeginInvoke([this, ##__VA_ARGS__](void *, void *) { this->func(__VA_ARGS__); }); \
      return; \
    } \
  }
}