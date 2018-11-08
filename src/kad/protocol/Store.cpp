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

#include <string.h>
#include "protocol/Store.h"

namespace kad
{
  namespace protocol
  {
    Store::Store()
      : Instruction(OpCode::STORE)
    {
    }


    bool Store::Serialize(bdfs::IOutputStream & output) const
    {
      if (!this->key || !this->data)
      {
        return false;
      }

      if (!this->SerializeOpCode(output))
      {
        return false;
      }

      output.WriteUInt64(this->version);

      output.WriteUInt32(this->ttl);

      output.WriteUInt8(this->original ? 1 : 0);

      this->key->Serialize(output);

      output.WriteUInt32(this->data->Size());
      
      if (this->data->Size() > 0)
      {
        output.Write(this->data->Data(), this->data->Size());
      }

      return true;
    }


    bool Store::Deserialize(bdfs::IInputStream & input)
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

      if (input.Remainder() < sizeof(uint8_t))
      {
        return false;
      }

      this->original = (input.ReadUInt8() != 0);

      this->key = std::make_shared<Key>();
      if (!this->key->Deserialize(input))
      {
        return false;
      }

      if (input.Remainder() < sizeof(uint32_t))
      {
        return false;
      }

      size_t size = input.ReadUInt32();

      if (input.Remainder() < size)
      {
        return false;
      }

      uint8_t * ptr = new uint8_t[size];

      bool result = input.Read(ptr, size) == size;

      this->data = std::make_shared<Buffer>(ptr, size, false, true);

      return result;
    }


    void Store::Print() const
    {
      printf("[STORE] key=%s size=%llu version=%llu ttl=%lu\n",
        this->key->ToString().c_str(),
        (unsigned long long)this->data->Size(),
        (unsigned long long)this->version,
        (unsigned long)this->ttl
      );
    }
  }
}
