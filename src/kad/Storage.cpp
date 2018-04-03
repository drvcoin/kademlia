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

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "PlatformUtils.h"
#include "Config.h"
#include "Storage.h"

namespace kad
{
  static const TCHAR * STORAGE_ROOT = _T("storage");


  bool Storage::Save() const
  {
    TSTRING root = Config::RootPath() + PATH_SEPERATOR_STR + STORAGE_ROOT;

    _tmkdir(root.c_str());

    if (!this->key || !this->data || !this->data->Data() || this->data->Size() == 0)
    {
      return false;
    }

    TSTRING path = root + PATH_SEPERATOR_STR + _TS(this->key->ToString());

    FILE * file = _tfopen(path.c_str(), _T("wb"));

    if (!file)
    {
      return false;
    }

    bool result = false;

    if (this->data->Size() > 0)
    {
      result = fwrite(this->data->Data(), 1, this->data->Size(), file) == this->data->Size();
    }

    fclose(file);

    return result;
  }


  bool Storage::Load()
  {
    TSTRING root = Config::RootPath() + PATH_SEPERATOR_STR + STORAGE_ROOT;

    _tmkdir(root.c_str());

    if (!this->key)
    {
      return false;
    }

    TSTRING path = root + PATH_SEPERATOR_STR + _TS(this->key->ToString());

    struct stat stat_buf;

    if (_tstat(path.c_str(), &stat_buf) != 0)
    {
      return false;
    }

    size_t size = stat_buf.st_size;

    bool result = false;

    uint8_t * buffer = new uint8_t[size];

    FILE * file = _tfopen(path.c_str(), _T("rb"));

    if (file)
    {
      result = fread(buffer, 1, size, file) == size;
      
      fclose(file);
    }

    if (result)
    {
      this->data = std::make_shared<Buffer>(buffer, size, false, true);
    }
    else
    {
      delete[] buffer;
    }

    return result;
  }
}