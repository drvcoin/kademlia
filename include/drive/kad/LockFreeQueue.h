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

namespace kad
{
  template<typename T>
  class LockFreeQueue
  {
  private:

    struct Node
    {
      explicit Node(T val)
        : next(nullptr)
        , value(std::move(val))
      {
      }

      std::atomic<Node *> next;
      T value;
    };

  public:

    LockFreeQueue()
      : size(0)
    {
      Node * val = new Node(T());
      this->first = val;
      this->divider = val;
      this->last = val;
    }


    ~LockFreeQueue()
    {
      while (this->first != nullptr)
      {
        Node * tmp = this->first;
        this->first = tmp->next.load();
        delete tmp;
      }
    }


    size_t Size()
    {
      return this->size;
    }


    bool Produce(T t)
    {
      Node * n = new Node(std::move(t));

      while (true)
      {
        assert(this->first && this->last && this->divider);

        Node * nullNode = nullptr;
      
        if (std::atomic_compare_exchange_weak(&this->last.load()->next, &nullNode, n))
        {
          this->last = n;

          this->size += 1;

          Node * tmp;

          while (true)
          {
            tmp = this->first;
            if (tmp == this->divider)
            {
              break;
            }

            if (std::atomic_compare_exchange_weak(& this->first, & tmp, tmp->next.load()))
            {
              delete tmp;
            }
            else
            {
              break;
            }
          }

          break;
        }
      }

      return true;
    }


    bool Consume(T & result)
    {
      while (true)
      {
        assert(this->first && this->last && this->divider);

        Node * tmp = this->divider;

        if (tmp != this->last)
        {
          result = std::move(tmp->next.load()->value);
          if (std::atomic_compare_exchange_weak(& this->divider, & tmp, tmp->next.load()))
          {
            this->size -= 1;

            return true;
          }
        }
        else
        {
          return false;
        }
      }
    }


  private:

    std::atomic<Node *> first;

    std::atomic<Node *> divider;

    std::atomic<Node *> last;

    std::atomic<size_t> size;
  };
}