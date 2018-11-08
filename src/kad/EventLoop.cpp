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
#include <chrono>
#include <drive/kad/EventLoop.h>

namespace kad
{
  EventLoop::~EventLoop()
  {
    this->Quit();
  }


  void EventLoop::Run()
  {
    this->alive = true;

    while (!this->quit)
    {
      EventHandlerEntry * entry = nullptr;

      if (this->events.Consume(entry))
      {
        entry->handler(entry->sender, entry->args);
        if (entry->completed && entry->mutex && entry->cond)
        {
          std::unique_lock<std::mutex> lock(* entry->mutex);
          * entry->completed = true;
          entry->cond->notify_all();
        }

        delete entry;
      }
      else
      {
        std::unique_lock<std::mutex> lock(this->mutex);

        this->cond.wait(lock);
      }
    }

    this->alive = false;
  }


  void EventLoop::Quit()
  {
    this->quit = true;

    {
      std::unique_lock<std::mutex> lock(this->mutex);
      this->cond.notify_all();
    }

    while (this->alive)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }


  void EventLoop::BeginInvoke(EventHandler handler, void * sender, void * args)
  {
    assert(handler != nullptr);

    auto entry = new EventHandlerEntry();
    entry->handler = std::move(handler);
    entry->sender = sender;
    entry->args = args;

    this->events.Produce(entry);

    std::unique_lock<std::mutex> lock(this->mutex);
    this->cond.notify_one();
  }


  void EventLoop::Invoke(EventHandler handler, void * sender, void * args)
  {
    assert(handler != nullptr);

    auto entry = new EventHandlerEntry();
    entry->handler = std::move(handler);
    entry->sender = sender;
    entry->args = args;

    // Make sure we do not access the same instance of shared_ptr on different threads by making copies on each thread
    auto completed = entry->completed = std::make_shared<std::atomic<bool>>(false);
    auto m = entry->mutex = std::make_shared<std::mutex>();
    auto c = entry->cond = std::make_shared<std::condition_variable>();

    this->events.Produce(entry);

    {
      std::unique_lock<std::mutex> lock(this->mutex);
      this->cond.notify_one();
    }

    std::unique_lock<std::mutex> lock(* m);
    if (! (* completed))
    {
      c->wait(lock);
    }
  }
}
