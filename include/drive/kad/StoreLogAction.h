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
#include <map>
#include <drive/kad/Key.h>
#include <drive/kad/Contact.h>
#include <drive/kad/Buffer.h>
#include <drive/kad/Action.h>

namespace kad
{
  class StoreLogAction : public Action
  {
  public:

    explicit StoreLogAction(Thread * owner, PackageDispatcher * dispatcher);

    void Initialize(const std::vector<std::pair<KeyPtr, ContactPtr>> & nodes, KeyPtr key, uint64_t version, BufferPtr data, uint32_t ttl, bool original);

    bool Start() override;

    bool GetResult() const;

    bool IsOutOfDate() const;

  private:

    void Send(std::pair<KeyPtr, ContactPtr> node);

    void OnResponse(KeyPtr key, PackagePtr request, PackagePtr response);

  private:

    KeyPtr key;

    uint64_t version = 0;

    BufferPtr data;

    uint32_t ttl = 0;

    bool original = false;

    std::map<KeyPtr, ContactPtr, KeyCompare> targets;

    std::set<KeyPtr, KeyCompare> processing;

    bool result = false;

    bool outOfDate = false;
  };
}
