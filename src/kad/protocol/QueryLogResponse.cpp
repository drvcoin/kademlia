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

#include <stdio.h>
#include <string.h>
#include "protocol/QueryLogResponse.h"

namespace kad
{
  namespace protocol
  {
    QueryLogResponse::QueryLogResponse()
      : Instruction(OpCode::QUERY_LOG_RESPONSE)
    {
    }


    bool QueryLogResponse::Serialize(IOutputStream & output) const
    {
      if (!this->data)
      {
        return false;
      }

      if (!this->SerializeOpCode(output))
      {
        return false;
      }

      output.WriteUInt64(this->version);

      output.WriteUInt32(this->ttl);

      output.WriteUInt32((uint32_t)this->data->Size());

      if (this->data->Size() > 0 && this->data->Data())
      {
        output.Write(this->data->Data(), this->data->Size());
      }

      return true;
    }


    bool QueryLogResponse::Deserialize(IInputStream & input)
    {
      if (!this->DeserializeOpCode(input))
      {
        return false;
      }

      if (input.Remainder() < sizeof(uint64_t))
      {
        return false;
      }

      this->version = input.ReadUInt64();

      if (input.Remainder() < sizeof(uint32_t))
      {
        return false;
      }

      this->ttl = input.ReadUInt32();

      if (input.Remainder() < sizeof(uint32_t))
      {
        return false;
      }

      uint32_t len = input.ReadUInt32();
      if (input.Remainder() < len)
      {
        return false;
      }

      uint8_t * buffer = new uint8_t[len];

      input.Read(buffer, len);

      this->data = std::make_shared<Buffer>(buffer, len, false, true);

      return true;
    }


    void QueryLogResponse::Print() const
    {
      printf("[QUERY_LOG_RESPONSE] size=%llu version=%llu ttl=%lu\n",
        (long long unsigned)this->data->Size(),
        (long long unsigned)this->version,
        (long unsigned)this->ttl
      );
    }
  }
}
