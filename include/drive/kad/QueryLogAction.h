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
#include <drive/kad/Thread.h>
#include <drive/kad/Key.h>
#include <drive/kad/Contact.h>
#include <drive/kad/Buffer.h>
#include <drive/kad/Action.h>

#include <json/json.h>


namespace kad
{
  class QueryLogAction : public Action
  {
  public:

    explicit QueryLogAction(Thread * owner, PackageDispatcher * dispatcher);

    void Initialize(KeyPtr target, std::string query, const std::vector<std::pair<KeyPtr, ContactPtr>> & nodes);

    bool Start() override;

    BufferPtr GetResult() const;

    uint64_t Version() const;

    uint32_t TTL() const;

    bool GetMissedNode(std::pair<KeyPtr, ContactPtr> & result) const;

  private:

    void SendCandidate(std::pair<KeyPtr, ContactPtr> candidate);

    void OnResponse(KeyPtr key, PackagePtr request, PackagePtr response);

  private:

    KeyPtr target;

    std::string query;

    std::map<KeyPtr, std::pair<KeyPtr, ContactPtr>, KeyCompare> candidates;

    std::set<KeyPtr, KeyCompare> validating;

    std::map<KeyPtr, ContactPtr, KeyCompare> validated;

    std::map<KeyPtr, std::pair<KeyPtr, ContactPtr>, KeyCompare> missed;

    std::set<KeyPtr, KeyCompare> offline;


    BufferPtr result;

    uint64_t version = 0;

    uint32_t ttl = 0;

  public:

    Json::Value root;

    uint32_t limit;

  };
}
