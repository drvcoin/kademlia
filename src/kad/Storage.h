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

#include <map>
#include <vector>
#include "Key.h"
#include "Buffer.h"
#include "PlatformUtils.h"

namespace kad
{
  class Storage
  {
  private:

    struct Entry
    {
      uint64_t version;
      int64_t timestamp;
      int64_t expiration;
    };

  public:

    static Storage * Persist();

    static Storage * Cache();

  public:

    void Initialize(bool load);

    bool Save(KeyPtr key, uint64_t version, BufferPtr content, int64_t ttl);

    void UpdateVersion(KeyPtr key, uint64_t version);

    void UpdateTTL(KeyPtr key, int64_t ttl);

    void UpdateTimestamp(KeyPtr key, int64_t timestamp = -1);

    BufferPtr MatchQuery(std::string query) const;

    BufferPtr Load(KeyPtr key) const;

    bool GetVersion(KeyPtr key, uint64_t * result) const;

    bool GetTTL(KeyPtr key, int64_t * result) const;

    bool GetExpiration(KeyPtr key, int64_t * result) const;

    bool GetTimestamp(KeyPtr key, int64_t * result) const;

    void Invalidate();

    void GetIdleKeys(std::vector<KeyPtr> & result, uint32_t period);

  private:

    Storage(TSTRING folder);

  private:

    TSTRING GetFileName(KeyPtr key, uint64_t version, int64_t expiration) const;

    static TSTRING Mkdir(const TCHAR * name);

  private:

    static Storage * persist;

    static Storage * cache;

  private:

    TSTRING folder;

    std::map<KeyPtr, Entry, KeyCompare> index;

    std::multimap<int64_t, KeyPtr> timestamps;

    std::multimap<int64_t, KeyPtr> expirations;
  };
}
