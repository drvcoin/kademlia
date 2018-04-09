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

#include <vector>
#include <chrono>
#include "Bucket.h"

namespace kad
{
  class KBuckets
  {
  public:

    static const size_t SizeK = 20;

  public:

    explicit KBuckets(KeyPtr selfKey);

    ContactPtr FindContact(KeyPtr key);

    void FindClosestContacts(KeyPtr key, std::vector<std::pair<KeyPtr, ContactPtr>> & result, bool restrictBucket = false);

    ContactPtr UpdateContact(KeyPtr key);

    void GetOldContacts(KeyPtr key, size_t count, std::vector<std::pair<KeyPtr, ContactPtr>> & result);

    ContactPtr EraseContact(KeyPtr key);

    bool AddContact(KeyPtr key, ContactPtr contact);

    void GetAllContacts(std::vector<std::pair<KeyPtr, ContactPtr>> & result) const;

    void UpdateLookupTime(KeyPtr key);

    void GetRefreshTargets(const Key & base, std::chrono::steady_clock::duration expiration, std::vector<KeyPtr> & result) const;

    size_t GetCloserContactCount(const Key & key) const;

    size_t GetBucketSize(KeyPtr key) const;

    size_t Size() const       { return this->size; }

    KeyPtr SelfKey() const    { return this->selfKey; }

  private:

    int FindBucket(const Key & key) const;

  private:

    Bucket buckets[Key::KEY_LEN_BITS] = {};

    size_t size = 0;

    KeyPtr selfKey;
  };
}