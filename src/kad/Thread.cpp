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

#include <string.h>
#include <drive/kad/Thread.h>

namespace kad
{
  thread_local Thread * Thread::current = nullptr;

  std::set<Thread *> Thread::threads;

  std::mutex Thread::threadsMutex;

  
  Thread * Thread::Current()
  {
    return current;
  }


  Thread::Thread(const char * name)
  {
    if (name)
    {
      size_t len = strlen(name);
      char * buffer = new char[len + 1];
      memcpy(buffer, name, len);
      buffer[len] = 0;
      this->name = buffer;
    }

    this->eventLoop = std::unique_ptr<EventLoop>(new EventLoop());

    this->thread = std::thread(& Thread::ThreadProc, this, this->eventLoop.get());

    std::unique_lock<std::mutex> lock(Thread::threadsMutex);

    Thread::threads.emplace(this);
  }


  Thread::~Thread()
  {
    if (this->eventLoop->IsRunning())
    {
      this->eventLoop->Quit();
    }

    if (this->thread.joinable())
    {
      this->thread.join();
    }

    std::unique_lock<std::mutex> lock(Thread::threadsMutex);

    Thread::threads.erase(this);

    if (this->name)
    {
      delete[] this->name;
      this->name = nullptr;
    }
  }


  bool Thread::IsRunning() const
  {
    return this->eventLoop->IsRunning();
  }


  void Thread::BeginInvoke(EventHandler handler, void * sender, void * args)
  {
    this->eventLoop->BeginInvoke(handler, sender, args);
  }


  void Thread::Invoke(EventHandler handler, void * sender, void * args)
  {
    this->eventLoop->Invoke(handler, sender, args);
  }


  void Thread::ThreadProc(Thread * _this, EventLoop * eventLoop)
  {
    Thread::current = _this;

    eventLoop->Run();
  }

}
