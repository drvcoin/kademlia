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

#include "KBuckets.h"

namespace kad
{
  const size_t KBuckets::SizeK;


  KBuckets::KBuckets(KeyPtr selfKey)
    : selfKey(selfKey)
  {
  }


  int KBuckets::FindBucket(const Key & key) const
  {
    return this->selfKey->GetDistance(key).GetHighestBit();
  }


  ContactPtr KBuckets::FindContact(KeyPtr key)
  {
    int idx = FindBucket(* key);

    if (idx < 0)
    {
      return nullptr;
    }
    else
    {
      return this->buckets[idx].FindContact(key);
    }
  }


  void KBuckets::FindClosestContacts(KeyPtr key, std::vector<std::pair<KeyPtr, ContactPtr>> & result, bool restrictBucket)
  {
    int idx = FindBucket(* key);

    if (idx < 0)
    {
      // Do not include self in the result;
      if (!restrictBucket)
      {
        idx = 0;
      }
      else
      {
        return;
      }
    }

    auto collect = [this, & result](int i)
    {
      for (size_t j = 0; j < this->buckets[i].Size() && result.size() < SizeK; ++j)
      {
        auto id = this->buckets[i].GetKeyFromIndx(j);

        if (id == nullptr)
        {
          break;
        }

        result.emplace_back(std::make_pair(id, this->buckets[i].FindContact(id)));
      }
    };

    if (!restrictBucket)
    {
      for (int i = idx; i >= 0 && result.size() < SizeK; --i)
      {
        collect(i);
      }

      if (result.size() < SizeK)
      {
        for (size_t i = idx + 1; i < Key::KEY_LEN_BITS && result.size() < SizeK; ++i)
        {
          collect(i);
        }
      }
    }
    else
    {
      collect(idx);
    }
  }


  ContactPtr KBuckets::UpdateContact(KeyPtr key)
  {
    int idx = FindBucket(* key);

    if (idx < 0)
    {
      return nullptr;
    }

    return this->buckets[idx].UpdateContact(key);
  }


  void KBuckets::GetOldContacts(KeyPtr key, size_t count, std::vector<std::pair<KeyPtr, ContactPtr>> & result)
  {
    int idx = FindBucket(* key);

    if (idx < 0)
    {
      return;
    }

    size_t added = 0;

    for (size_t i = 0; i < this->buckets[idx].Size() && added < count; ++i)
    {
      auto id = this->buckets[idx].GetKeyFromIndx(i);

      result.emplace_back(std::make_pair(id, this->buckets[idx].FindContact(id)));
    }
  }

  ContactPtr KBuckets::EraseContact(KeyPtr key)
  {
    int idx = FindBucket(* key);

    if (idx >= 0)
    {
      ContactPtr contact = this->buckets[idx].EraseContact(key);
      if (contact)
      {
        -- this->size;
        return contact;
      }
    }

    return nullptr;
  }

  
  bool KBuckets::AddContact(KeyPtr key, ContactPtr contact)
  {
    int idx = FindBucket(* key);

    if (idx >= 0 && this->buckets[idx].AddContact(key, contact))
    {
      ++ this->size;
      return true;
    }

    return false;
  }


  void KBuckets::GetAllContacts(std::vector<std::pair<KeyPtr, ContactPtr>> & result) const
  {
    for (size_t i = 0; i < Key::KEY_LEN_BITS; ++i)
    {
      this->buckets[i].GetAllContacts(result);
    }
  }


  size_t KBuckets::GetBucketSize(KeyPtr key) const
  {
    int idx = FindBucket(* key);

    if (idx >= 0)
    {
      return this->buckets[idx].Size();
    }

    return 0;
  }


  void KBuckets::UpdateLookupTime(KeyPtr key)
  {
    int idx = FindBucket(*key);

    if (idx < 0)
    {
      return;
    }

    this->buckets[idx].UpdateLookupTime();
  }


  void KBuckets::GetRefreshTargets(const Key & base, std::chrono::steady_clock::duration expiration, std::vector<KeyPtr> & result) const
  {
    auto now = std::chrono::steady_clock::now();

    for (size_t i = 0; i < Key::KEY_LEN_BITS; ++i)
    {
      if (now - this->buckets[i].LastLookupTime() > expiration)
      {
        Key mask = {};

        mask.SetBit(i, true);

        // a ^ b = c => a ^ c = b

        result.emplace_back(std::make_shared<Key>(base.GetDistance(mask)));
      }
    }
  }


  size_t KBuckets::GetCloserContactCount(const Key & key) const
  {
    size_t result = 0;

    int idx = FindBucket(key);

    if (idx < 0)
    {
      return result;
    }

    for (size_t i = 0; i < idx; ++i)
    {
      result += this->buckets[i].Size();
    }

    std::vector<std::pair<KeyPtr, ContactPtr>> contacts;
    this->buckets[idx].GetAllContacts(contacts);

    Key distance = this->selfKey->GetDistance(key);

    for (const auto & contact : contacts)
    {
      if (this->selfKey->GetDistance(*contact.first) < distance)
      {
        ++ result;
      }
    }

    return result;
  }
}