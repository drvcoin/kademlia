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

#include <drive/kad/Bucket.h>

namespace kad
{
  ContactPtr Bucket::FindContact(KeyPtr key)
  {
    auto itr = this->values.find(key);
    return itr != this->values.end() ? itr->second.contact : nullptr;
  }


  ContactPtr Bucket::UpdateContact(KeyPtr key)
  {
    auto itr = this->values.find(key);

    if (itr == this->values.end())
    {
      return nullptr;
    }

    this->keys.splice(this->keys.end(), this->keys, itr->second.iter);

    itr->second.iter = (-- this->keys.end());

    return itr->second.contact;
  }


  KeyPtr Bucket::GetKeyFromIndx(size_t idx)
  {
    if (idx >= this->keys.size())
    {
      return nullptr;
    }

    auto itr = this->keys.begin();

    for (size_t i = 0; i < idx; ++i) {
      ++ itr;
    }

    return * itr;
  }


  ContactPtr Bucket::EraseContact(KeyPtr key)
  {
    auto itr = this->values.find(key);

    if (itr == this->values.end())
    {
      return nullptr;
    }

    this->keys.erase(itr->second.iter);

    auto result = itr->second.contact;

    this->values.erase(itr);

    return result;
  }


  bool Bucket::AddContact(KeyPtr key, ContactPtr contact)
  {
    if (this->values.find(key) != this->values.end())
    {
      return false;
    }

    this->keys.push_back(key);

    Entry entry;
    entry.iter = (-- this->keys.end());
    entry.contact = contact;

    this->values.emplace(key, std::move(entry));

    return true;
  }


  void Bucket::GetAllContacts(std::vector<std::pair<KeyPtr, ContactPtr>> & result) const
  {
    for (auto value : this->values)
    {
      result.emplace_back(std::make_pair(value.first, value.second.contact));
    }
  }


  void Bucket::UpdateLookupTime()
  {
    this->lastLookupTime = std::chrono::steady_clock::now();
  }
}
