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
#include <set>
#include <map>
#include "Thread.h"
#include "Key.h"
#include "Contact.h"
#include "Buffer.h"
#include "Action.h"

namespace kad
{
  class FindValueAction : public Action
  {
  public:

    explicit FindValueAction(Thread * owner, PackageDispatcher * dispatcher);

    void Initialize(KeyPtr target, const std::vector<std::pair<KeyPtr, ContactPtr>> & nodes);

    bool Start() override;

    BufferPtr GetResult() const;

  private:

    void SendCandidate(std::pair<KeyPtr, ContactPtr> candidate);

    void OnResponse(KeyPtr key, PackagePtr request, PackagePtr response);

  private:

    KeyPtr target;

    std::map<KeyPtr, std::pair<KeyPtr, ContactPtr>, KeyCompare> candidates;

    std::set<KeyPtr, KeyCompare> validating;

    std::map<KeyPtr, ContactPtr, KeyCompare> validated;

    std::set<KeyPtr, KeyCompare> offline;

    BufferPtr result;
  };
}