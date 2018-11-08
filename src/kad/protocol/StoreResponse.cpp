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

#include "protocol/StoreResponse.h"

namespace kad
{
  namespace protocol
  {
    StoreResponse::StoreResponse()
      : Instruction(OpCode::STORE_RESPONSE)
    {
    }


    bool StoreResponse::Serialize(bdfs::IOutputStream & output) const
    {
      if (!this->SerializeOpCode(output))
      {
        return false;
      }

      return output.WriteUInt16(static_cast<uint16_t>(this->result));
    }


    bool StoreResponse::Deserialize(bdfs::IInputStream & input)
    {
      if (!this->DeserializeOpCode(input))
      {
        return false;
      }

      if (input.Remainder() < sizeof(uint16_t))
      {
        return false;
      }

      uint16_t value = input.ReadUInt16();

      if (value < static_cast<uint16_t>(ErrorCode::__MAX__))
      {
        this->result = static_cast<ErrorCode>(value);
      }
      else
      {
        this->result = ErrorCode::FAILED;
      }

      return true;
    }


    void StoreResponse::Print() const
    {
      printf("[STORE_RESPONSE] %s\n", this->result == ErrorCode::SUCCESS ? "success" : "failed");
    }
  }
}