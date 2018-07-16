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
#if defined(WIN32) || defined(_WIN32)
#include <stdio.h>
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

#include <string>
#include <vector>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits>
#include "Config.h"
#include "Storage.h"

#include <json/json.h>


namespace kad
{
  Storage * Storage::persist = nullptr;

  Storage * Storage::cache = nullptr;


  Storage * Storage::Persist()
  {
    if (unlikely(persist == nullptr))
    {
      persist = new Storage(Storage::Mkdir(_T("data")));
    }

    return persist;
  }


  Storage * Storage::Cache()
  {
    if (unlikely(cache == nullptr))
    {
      cache = new Storage(Storage::Mkdir(_T("cache")));
    }

    return cache;
  }


  TSTRING Storage::Mkdir(const TCHAR * name)
  {
    TCHAR buffer[PATH_MAX];

    _stprintf(buffer, _T("%s%s%s"),
      Config::RootPath().c_str(),
      PATH_SEPERATOR_STR,
      _T("storage")
    );

    _tmkdir(buffer);

    _stprintf(buffer, _T("%s%s%s%s%s"),
      Config::RootPath().c_str(),
      PATH_SEPERATOR_STR,
      _T("storage"),
      PATH_SEPERATOR_STR,
      name
    );

    _tmkdir(buffer);

    return buffer;
  }


  Storage::Storage(TSTRING folder)
    : folder(folder)
  {
  }


  static inline int64_t get_now()
  {
    return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
  }


  void Storage::Initialize(bool load)
  {
    std::vector<std::string> names;

#if defined(WIN32) || defined(_WIN32)
    static_assert(false, "Not Implemented");
#else

    DIR * dir = opendir(this->folder.c_str());

    if (!dir)
    {
      return;
    }

    struct dirent * entry = nullptr;

    while ((entry = readdir(dir)) != nullptr)
    {
      names.emplace_back(entry->d_name);
    }

    closedir(dir);
#endif

    if (load)
    {
      int64_t now = get_now();

      for (const auto & name : names)
      {
        char keyname[PATH_MAX];
        long long expiration;
        unsigned long long version;

        if (sscanf(name.c_str(), "%lld-%llu-%s", &expiration, &version, keyname) == 3)
        {
          if (expiration < now)
          {
            TCHAR path[PATH_MAX];
            _stprintf(path, _T("%s%s%s"), this->folder.c_str(), PATH_SEPERATOR_STR, _TS(name).c_str());
            _tunlink(path);
          }
          else
          {
            KeyPtr key = std::make_shared<Key>();

            if (key->FromString(keyname))
            {
              this->index[key] = {
                .version = version,
                .timestamp = 0,
                .expiration = expiration
              };
              this->timestamps.emplace(0, key);
              this->expirations.emplace(expiration, key);
            }
          }
        }
      }
    }
    else
    {
      for (const auto & name : names)
      {
        TCHAR path[PATH_MAX];
        _stprintf(path, _T("%s%s%s"), this->folder.c_str(), PATH_SEPERATOR_STR, _TS(name).c_str());
        _tunlink(path);
      }
    }
  }


  TSTRING Storage::GetFileName(KeyPtr key, uint64_t version, int64_t expiration) const
  {
    TCHAR path[PATH_MAX];
    _stprintf(path, _T("%s%s%lld-%llu-%s"),
      this->folder.c_str(),
      PATH_SEPERATOR_STR,
      static_cast<long long>(expiration),
      static_cast<unsigned long long>(version),
      _TS(key->ToString()).c_str()
    );

    return path;
  }


  bool Storage::GetVersion(KeyPtr key, uint64_t * result) const
  {
    if (!key)
    {
      return false;
    }

    auto itr = this->index.find(key);
    if (itr == this->index.end())
    {
      return false;
    }

    if (result)
    {
      *result = itr->second.version;
    }

    return true;
  }


  bool Storage::GetTTL(KeyPtr key, int64_t * result) const
  {
    if (!this->GetExpiration(key, result))
    {
      return false;
    }

    if (result)
    {
      *result = std::max<int64_t>(0, *result - get_now());
    }

    return true;
  }


  bool Storage::GetExpiration(KeyPtr key, int64_t * result) const
  {
    if (!key)
    {
      return false;
    }

    auto itr = this->index.find(key);
    if (itr == this->index.end())
    {
      return false;
    }

    if (result)
    {
      *result = itr->second.expiration;
    }

    return true;
  }


  bool Storage::Save(KeyPtr key, uint64_t version, BufferPtr content, int64_t ttl)
  {
    if (!key || !content || !content->Data() || content->Size() == 0)
    {
      return false;
    }

    uint64_t oldVersion;
    if (this->GetVersion(key, &oldVersion) && oldVersion > version)
    {
      return false;
    }

    this->UpdateVersion(key, version);
    this->UpdateTTL(key, ttl);
    this->UpdateTimestamp(key, get_now());

    int64_t expiration;
    if (!this->GetExpiration(key, &expiration))
    {
      return false;
    }

    FILE * file = _tfopen(this->GetFileName(key, version, expiration).c_str(), _T("wb"));

    if (!file)
    {
      return false;
    }

    bool result = false;

    if (content->Size() > 0)
    {
      result = fwrite(content->Data(), 1, content->Size(), file) == content->Size();
    }

    fclose(file);

    return result;
  }

  bool Storage::Compare(const char * data, const char * query) const
  {
    bool res;

    res = false;

    int s;
    int r;

    sscanf( query, "(storage:%d+reputation:%d", &s, &r );
    printf("storage = %d reputation = %d\n", s, r);

    printf("# COMPARE(data,query): %s >< %s\n", data, query);


    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(data, strlen(data), root, false))
    {
      printf("ERROR parsing json\n");
      return false;
    }


    std::string node = root["node"].asString();
    int storage = root["storage"].asInt();
    int reputation = root["reputation"].asInt();
    printf("node: %s %d %d\n",node.c_str(), storage, reputation);

    if (storage >= s && reputation >= r)
    {
      res = true;
    }
    return res;
  }

  char * rtrim(char *s)
  {
    char * e = s + strlen(s) - 1;
    while (*e == ' ' || *e == '\r' || *e == '\n' || *e == '\t')
    {
      *e = '\0';
      e--;
    }
    return s;
  }

  BufferPtr Storage::MatchQuery(std::string query) const
  {
printf("Storage::MatchQuery: query %s\n", query.c_str());

    DIR * dir = opendir(this->folder.c_str());

    if (!dir)
    {
      return nullptr;
    }

    struct dirent * entry = nullptr;

    while ((entry = readdir(dir)) != nullptr)
    {
      std::string name = entry->d_name;
      char keyname[PATH_MAX];
      long long expiration;
      unsigned long long version;

      if (sscanf(name.c_str(), "%lld-%llu-%s", &expiration, &version, keyname) == 3)
      {
        printf("%s\n", name.c_str());

        TCHAR path[PATH_MAX];
        _stprintf(path, _T("%s%s%s"), this->folder.c_str(), PATH_SEPERATOR_STR, _TS(name).c_str());


        struct stat stat_buf;

        if (_tstat(path, &stat_buf) != 0)
        {
          return nullptr;
        }

        size_t size = stat_buf.st_size;

        uint8_t * buffer = new uint8_t[size];

        bool result = false;

        FILE * file = _tfopen(path, _T("rb"));

        if (file)
        {
          result = fread(buffer, 1, size, file) == size;

          if (result)
          {
            rtrim((char*)buffer);

            printf("%lu '%s'\n", size, (char*) buffer);

            bool compres = this->Compare((char*)buffer, query.c_str());
            if (compres)
            {
              printf("MATCH\n");

              return std::make_shared<Buffer>(buffer, size, false, true);

            }
            else
            {
              delete[] buffer;
            }
          }

          fclose(file);
        }

      }
    }

    closedir(dir);

    return nullptr;
  }


  BufferPtr Storage::Load(KeyPtr key) const
  {
    if (!key)
    {
      return nullptr;
    }

    auto itr = this->index.find(key);
    if (itr == this->index.end())
    {
      return nullptr;
    }

    auto path = this->GetFileName(key, itr->second.version, itr->second.expiration);

    struct stat stat_buf;

    if (_tstat(path.c_str(), &stat_buf) != 0)
    {
      return nullptr;
    }

    size_t size = stat_buf.st_size;

    uint8_t * buffer = new uint8_t[size];

    bool result = false;

    FILE * file = _tfopen(path.c_str(), _T("rb"));

    if (file)
    {
      result = fread(buffer, 1, size, file) == size;

      fclose(file);
    }

    if (result)
    {
      return std::make_shared<Buffer>(buffer, size, false, true);
    }
    else
    {
      delete[] buffer;
      return nullptr;
    }
  }


  void Storage::Invalidate()
  {
    int64_t now = get_now();

    auto start = this->expirations.begin();
    auto end = start;

    for (; end != this->expirations.end() && end->first <= now; ++end)
    {
      auto itr = this->index.find(end->second);

      if (itr != this->index.end())
      {
        _tunlink(this->GetFileName(itr->first, itr->second.version, itr->second.expiration).c_str());

        auto range = this->timestamps.equal_range(itr->second.timestamp);
        for (auto ts = range.first; ts != range.second; ++ts)
        {
          if ((*ts->second) == (*itr->first))
          {
            this->timestamps.erase(ts);
            break;
          }
        }

        this->index.erase(itr);
      }
    }

    if (start != end)
    {
      this->expirations.erase(start, end);
    }
  }


  void Storage::UpdateVersion(KeyPtr key, uint64_t version)
  {
    auto idx = this->index.find(key);
    if (idx != this->index.end())
    {
      if (idx->second.version != version)
      {
        _trename(
          this->GetFileName(key, idx->second.version, idx->second.expiration).c_str(),
          this->GetFileName(key, version, idx->second.expiration).c_str()
        );

        idx->second.version = version;
      }
    }
    else
    {
      this->index[key] = {
        .version = version,
        .timestamp = 0,
        .expiration = 0
      };
      this->timestamps.emplace(0, key);
      this->expirations.emplace(0, key);
    }
  }


  void Storage::UpdateTTL(KeyPtr key, int64_t ttl)
  {
    int64_t now = get_now();

    ttl = std::min<int64_t>(ttl, std::numeric_limits<int64_t>::max() - now);

    int64_t expiration = now + ttl;

    auto idx = this->index.find(key);

    if (idx != this->index.end())
    {
      if (idx->second.expiration != expiration)
      {
        auto range = this->expirations.equal_range(idx->second.expiration);
        for (auto itr = range.first; itr != range.second; ++itr)
        {
          if ((*key) == (*itr->second))
          {
            this->expirations.erase(itr);
            break;
          }
        }

        _trename(
          this->GetFileName(key, idx->second.version, idx->second.expiration).c_str(),
          this->GetFileName(key, idx->second.version, expiration).c_str()
        );

        idx->second.expiration = expiration;
        this->expirations.emplace(expiration, key);
      }
    }
    else
    {
      this->index[key] = {
        .version = 0,
        .timestamp = 0,
        .expiration = expiration
      };
      this->timestamps.emplace(0, key);
      this->expirations.emplace(expiration, key);
    }
  }


  void Storage::UpdateTimestamp(KeyPtr key, int64_t timestamp)
  {
    if (timestamp < 0)
    {
      timestamp = get_now();
    }

    auto idx = this->index.find(key);
    if (idx != this->index.end())
    {
      if (idx->second.timestamp != timestamp)
      {
        auto range = this->timestamps.equal_range(idx->second.timestamp);
        for (auto itr = range.first; itr != range.second; ++itr)
        {
          if ((*key) == (*itr->second))
          {
            this->timestamps.erase(itr);
            break;
          }
        }
      }

      idx->second.timestamp = timestamp;
      this->timestamps.emplace(timestamp, key);
    }
    else
    {
      this->index[key] = {
        .version = 0,
        .timestamp = timestamp,
        .expiration = 0
      };
      this->timestamps.emplace(timestamp, key);
      this->expirations.emplace(0, key);
    }
  }


  void Storage::GetIdleKeys(std::vector<KeyPtr> & result, uint32_t period)
  {
    int64_t criteria = std::max<int64_t>(0, get_now() - period);

    for (const auto & pair : this->timestamps)
    {
      if (pair.first > criteria)
      {
        break;
      }

      result.emplace_back(pair.second);
    }
  }
}
