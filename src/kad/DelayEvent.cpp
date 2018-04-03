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

#include "DelayEvent.h"


namespace kad
{
  DelayEvent::DelayEvent()
  {
    this->thread = std::thread(std::bind(&DelayEvent::ThreadProc, this));
  }


  DelayEvent::~DelayEvent()
  {
    this->quit = true;

    {
      std::unique_lock<std::mutex> lock(this->mutex);

      this->cond.notify_all();
    }

    if (this->thread.joinable())
    {
      this->thread.join();
    }
  }


  std::pair<void *, void *> DelayEvent::Reset(int msTime, EventHandler handler, void * sender, void * args, Thread * owner)
  {
    DelayEventEntry * e = new DelayEventEntry();
    e->time = std::chrono::steady_clock::now() + std::chrono::microseconds(msTime);
    e->handler = handler;
    e->sender = sender;
    e->args = args;
    e->owner = owner;

    auto result = std::make_pair<void *, void *>(nullptr, nullptr);

    {
      std::unique_lock<std::mutex> lock(this->mutex);

      if (this->entry)
      {
        result.first = this->entry->sender;
        result.second = this->entry->args;
      }

      this->entry.reset(e);

      this->cond.notify_one();
    }

    return result;
  }


  void DelayEvent::ThreadProc()
  {
    while (!this->quit)
    {
      auto now = std::chrono::steady_clock::now();

      {
        std::unique_lock<std::mutex> lock(this->mutex);

        if (this->entry)
        {
          if (this->entry->time <= now)
          {
            Call(this->entry->handler, this->entry->sender, this->entry->args, this->entry->owner);
            this->entry.reset();
          }
          else
          {
            this->cond.wait_for(lock, this->entry->time - now);
          }
        }
        else
        {
          this->cond.wait(lock);
        }
      }
    }
  }


  void DelayEvent::Call(EventHandler handler, void * sender, void * args, Thread * owner)
  {
    if (owner)
    {
      owner->BeginInvoke(handler, sender, args);
    }
    else
    {
      handler(sender, args);
    }
  }
}