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

#include <utility>
#include <vector>
#include "Key.h"
#include "Contact.h"
#include "Instruction.h"

namespace kad
{
  namespace protocol
  {
    class FindNodeResponse : public Instruction
    {
    public:

      using KeyContactPair = std::pair<KeyPtr, ContactPtr>;

    public:

      FindNodeResponse();

      ~FindNodeResponse() override = default;

      bool Serialize(IOutputStream & output) const override;

      bool Deserialize(IInputStream & input) override;

      bool AddNode(KeyPtr key, ContactPtr contact);

      void Print() const override;

      const std::vector<KeyContactPair> & Nodes() const
      {
        return this->nodes;
      }

    private:

      std::vector<KeyContactPair> nodes;
    };
  }
}