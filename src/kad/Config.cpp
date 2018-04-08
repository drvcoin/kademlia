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

#include <ctime>
#include <cstdlib>
#include "Config.h"

namespace kad
{
  KeyPtr Config::nodeId = std::make_shared<Key>();

  Contact Config::contactInfo = {};

  const size_t Config::parallelism = 3;

  TSTRING Config::rootPath = _T(".");

  bool Config::verbose = false;

  int Config::refreshInterval = 3600000;

  uint32_t Config::minCacheTTL = 3600 * 24;

  uint32_t Config::replicateTTL = 3600;

  int Config::refreshTimerInterval = 5 * 60 * 1000;


  void Config::Initialize(TSTRING path)
  {
    Config::rootPath = path;

    Config::InitKey();
  }


  void Config::Initialize(const Key & key, const Contact & contact)
  {
    Config::nodeId = std::make_shared<Key>(key);
    Config::contactInfo = contact;
  }



  void Config::InitKey()
  {
    TSTRING keyFilePath = Config::rootPath + _T(PATH_SEPERATOR_STR) + _T("key");

    uint8_t data[Key::KEY_LEN];

    bool hasValue = false;

    FILE * file = _tfopen(keyFilePath.c_str(), _T("r"));

    if (file)
    {
      if (fread(data, 1, Key::KEY_LEN, file) == Key::KEY_LEN)
      {
        hasValue = true;
      }
      fclose(file);
    }
    
    if (!hasValue)
    {
      // TODO: initialize key with something that may relate to the physical location of the node
      std::srand(static_cast<unsigned int>(std::time(nullptr)));
      for (size_t i = 0; i < Key::KEY_LEN; ++i)
      {
        data[i] = static_cast<uint8_t>(std::rand());
      }
    }

    Config::nodeId = std::make_shared<Key>(data);
  }
}