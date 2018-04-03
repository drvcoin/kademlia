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
#include <atomic>
#include <map>
#include <chrono>
#include "Thread.h"

namespace kad
{
  class Timer
  {
  private:

    struct TimerEntry
    {
      std::chrono::steady_clock::time_point time;
      int interval;
      EventHandler handler;
      void * sender;
      void * args;
      Thread * owner;
    };

  public:

    Timer();

    ~Timer();

    std::pair<void *, void *> Reset(int msTime, bool repeat, EventHandler handler, void * sender, void * args, Thread * owner);

    std::pair<void *, void *> Reset();

  private:

    void ThreadProc();

    static void Call(EventHandler handler, void * sender, void * args, Thread * owner);

  private:

    std::atomic<bool> quit{false};

    std::unique_ptr<TimerEntry> entry;

    std::thread thread;

    std::mutex mutex;

    std::condition_variable cond;
  };
}