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

#include <list>
#include <map>
#include <vector>
#include <chrono>
#include "Key.h"
#include "Contact.h"

namespace kad
{
  class Bucket
  {
  private:

    struct Entry
    {
      std::list<KeyPtr>::const_iterator iter;
      ContactPtr contact;
    };

  public:

    ContactPtr FindContact(KeyPtr key);

    ContactPtr UpdateContact(KeyPtr key);

    KeyPtr GetKeyFromIndx(size_t idx);

    ContactPtr EraseContact(KeyPtr key);

    bool AddContact(KeyPtr key, ContactPtr contact);

    void GetAllContacts(std::vector<std::pair<KeyPtr, ContactPtr>> & result) const;

    void UpdateLookupTime();

    size_t Size() const     { return this->keys.size(); }

    const std::chrono::steady_clock::time_point & LastLookupTime() const
    {
      return this->lastLookupTime;
    }

  private:

    std::list<KeyPtr> keys;

    std::map<KeyPtr, Entry, KeyCompare> values;

    std::chrono::steady_clock::time_point lastLookupTime;
  };
}