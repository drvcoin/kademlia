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
#include "Config.h"
#include "Storage.h"

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
      for (const auto & name : names)
      {
        KeyPtr key = std::make_shared<Key>();

        if (key->FromString(name.c_str()))
        {
          this->Update(key, 0);
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


  bool Storage::Save(KeyPtr key, BufferPtr content, int64_t ttl)
  {
    if (!key || !content || !content->Data() || content->Size() == 0)
    {
      return false;
    }

    this->Update(key, ttl);

    TCHAR path[PATH_MAX];
    _stprintf(path, _T("%s%s%s"),
      this->folder.c_str(),
      PATH_SEPERATOR_STR,
      _TS(key->ToString()).c_str()
    );

    FILE * file = _tfopen(path, _T("wb"));

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



  BufferPtr Storage::Load(KeyPtr key) const
  {
    if (!key || this->index.find(key) == this->index.end())
    {
      return nullptr;
    }

    TCHAR path[PATH_MAX];
    _stprintf(path, _T("%s%s%s"), this->folder.c_str(), PATH_SEPERATOR_STR, _TS(key->ToString()).c_str());

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
    int64_t now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
    
    auto start = this->rindex.begin();
    auto end = start;

    for (; end != this->rindex.end() && end->first <= now; ++end)
    {
      TCHAR path[PATH_MAX];
      _stprintf(path, _T("%s%s%s"), this->folder.c_str(), PATH_SEPERATOR_STR, _TS(end->second->ToString()).c_str());

      _tunlink(path);

      this->index.erase(end->second);
    }

    if (start != end)
    {
      this->rindex.erase(start, end);
    }
  }


  void Storage::Update(KeyPtr key, int64_t ttl)
  {
    auto now = std::chrono::steady_clock::now();

    int64_t timestamp = std::chrono::time_point_cast<std::chrono::seconds>(now).time_since_epoch().count() + ttl;

    auto idx = this->index.find(key);

    if (idx != this->index.end())
    {
      auto range = this->rindex.equal_range(idx->second);

      for (auto itr = range.first; itr != range.second; ++itr)
      {
        if ((*key) == (*itr->second))
        {
          this->rindex.erase(itr);
          break;
        }
      }

      this->index.erase(idx);
    }

    this->rindex.emplace(std::make_pair(timestamp, key));
    this->index[key] = timestamp;
  }


  void Storage::GetExpiredKeys(std::vector<KeyPtr> & result)
  {
    int64_t now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now()).time_since_epoch().count();

    for (const auto & pair : this->rindex)
    {
      if (pair.first > now)
      {
        break;
      }

      result.emplace_back(pair.second);
    }
  }
}