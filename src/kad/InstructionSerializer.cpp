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

#include "protocol/Ping.h"
#include "protocol/Pong.h"
#include "protocol/Store.h"
#include "protocol/StoreResponse.h"
#include "protocol/FindNode.h"
#include "protocol/FindNodeResponse.h"
#include "protocol/FindValue.h"
#include "protocol/FindValueResponse.h"

#include "IOutputStream.h"
#include "IInputStream.h"
#include "InstructionSerializer.h"

namespace kad
{
  bool InstructionSerializer::Serialize(IOutputStream & output, const Instruction * instr)
  {
    if (!instr)
    {
      return false;
    }

    return instr->Serialize(output);
  }


  Instruction * InstructionSerializer::Deserialize(IInputStream & input)
  {
    if (input.Remainder() < sizeof(uint16_t))
    {
      return nullptr;
    }

    Instruction * instr = CreateInstruction(static_cast<OpCode>(input.PeekUInt16()));
    if (!instr)
    {
      return nullptr;
    }

    if (!instr->Deserialize(input))
    {
      delete instr;
      return nullptr;
    }

    return instr;
  }


  Instruction * InstructionSerializer::CreateInstruction(OpCode code)
  {
    switch (code)
    {
      case OpCode::PING:                return new protocol::Ping();
      case OpCode::PONG:                return new protocol::Pong();
      case OpCode::STORE:               return new protocol::Store();
      case OpCode::STORE_RESPONSE:      return new protocol::StoreResponse();
      case OpCode::FIND_NODE:           return new protocol::FindNode();
      case OpCode::FIND_NODE_RESPONSE:  return new protocol::FindNodeResponse();
      case OpCode::FIND_VALUE:          return new protocol::FindValue();
      case OpCode::FIND_VALUE_RESPONSE: return new protocol::FindValueResponse();

      default:                          return nullptr;
    }
  }
}