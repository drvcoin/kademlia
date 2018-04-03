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

#include <chrono>
#include "AsyncResult.h"


namespace kad
{
  IAsyncResult::~IAsyncResult()
  {
  }


  void IAsyncResult::Complete()
  {
    this->completed = true;

    std::unique_lock<std::mutex> lock(this->mutex);

    this->cond.notify_all();
  }


  bool IAsyncResult::Wait(int msTimeout)
  {
    if (this->completed)
    {
      return true;
    }

    std::unique_lock<std::mutex> lock(this->mutex);

    if (this->completed)
    {
      return true;
    }

    if (msTimeout > 0)
    {
      return this->cond.wait_for(lock, std::chrono::milliseconds(msTimeout)) == std::cv_status::no_timeout;
    }
    else
    {
      this->cond.wait(lock);
      return true;
    }
  }
}