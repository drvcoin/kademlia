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

#include <assert.h>
#include <stdio.h>
#include "protocol/FindNodeResponse.h"

namespace kad
{
  namespace protocol
  {
    FindNodeResponse::FindNodeResponse()
      : Instruction(OpCode::FIND_NODE_RESPONSE)
    {
    }


    bool FindNodeResponse::Serialize(bdfs::IOutputStream & output) const
    {
      if (!this->SerializeOpCode(output))
      {
        return false;
      }

      output.WriteUInt8((uint8_t)this->nodes.size());

      for (const auto & pair : this->nodes)
      {
        pair.first->Serialize(output);

        pair.second->Serialize(output);
      }

      return true;
    }


    bool FindNodeResponse::Deserialize(bdfs::IInputStream & input)
    {
      if (!this->DeserializeOpCode(input))
      {
        return false;
      }

      if (input.Remainder() < sizeof(uint8_t))
      {
        return false;
      }

      this->nodes.clear();

      size_t len = input.ReadUInt8();

      for (size_t i = 0; i < len; ++i)
      {
        KeyPtr key = std::make_shared<Key>();
        ContactPtr contact = std::make_shared<Contact>();

        if (!key->Deserialize(input) || !contact->Deserialize(input))
        {
          return false;
        }

        this->nodes.emplace_back(std::make_pair(key, contact));
      }

      return true;
    }


    bool FindNodeResponse::AddNode(KeyPtr key, ContactPtr contact)
    {
      if (!key || !contact)
      {
        return false;
      }

      this->nodes.emplace_back(std::make_pair(key, contact));

      return true;
    }


    void FindNodeResponse::Print() const
    {
      printf("[FIND_NODE_REPONSE] nodes=[");

      bool first = true;
      for (const auto & node : this->nodes)
      {
        if (first)
          first = false;
        else
          printf(" ,");

        printf("{key:\"%s\", contact:\"%s\"}", node.first->ToString().c_str(), node.second->ToString().c_str());
      }

      printf("]\n");
    }
  }
}