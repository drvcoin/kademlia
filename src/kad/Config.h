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

#include "Key.h"
#include "Contact.h"
#include "PlatformUtils.h"

namespace kad
{
  class Config
  {
  public:

    static void Initialize(TSTRING path);

    static void Initialize(const Key & key, const Contact & contact);

    static KeyPtr NodeId()                { return nodeId; }

    static const Contact & ContactInfo()  { return contactInfo; }

    static const size_t Parallelism()     { return parallelism; }

    static const TSTRING & RootPath()     { return rootPath; }

    static bool Verbose()                 { return verbose; }

    static void SetVerbose(bool value)    { verbose = value; }

    static int RefreshInterval()          { return refreshInterval; }

    static uint32_t MinCacheTTL()         { return minCacheTTL; }

    static uint32_t ReplicateTTL()        { return replicateTTL; }

    static int RefreshTimerInterval()     { return refreshTimerInterval; }

  private:

    static void InitKey();

  private:

    static KeyPtr nodeId;

    static Contact contactInfo;

    static const size_t parallelism;

    static TSTRING rootPath;

    static bool verbose;

    static int refreshInterval;

    static uint32_t minCacheTTL;

    static uint32_t replicateTTL;

    static int refreshTimerInterval;
  };
}