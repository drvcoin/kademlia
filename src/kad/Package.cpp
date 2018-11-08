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

#include <drive/kad/InstructionSerializer.h>
#include <drive/kad/Package.h>

namespace kad
{
  std::atomic<uint16_t> Package::gid{0};


  Package::Package(Package::PackageType type, KeyPtr from, ContactPtr tgt, std::unique_ptr<Instruction> && instr)
    : from(from)
    , target(tgt)
    , instruction(std::move(instr))
    , id(Package::gid++)
    , type(type)
  {
  }


  Package::Package(Package::PackageType type, KeyPtr from, uint16_t id, ContactPtr tgt, std::unique_ptr<Instruction> && instr)
    : from(from)
    , target(tgt)
    , instruction(std::move(instr))
    , id(id)
    , type(type)
  {
  }


  bool Package::Serialize(bdfs::IOutputStream & output) const
  {
    if (!this->instruction)
    {
      return false;
    }

    output.WriteUInt8(0); // version, always 0

    output.WriteUInt8(static_cast<uint8_t>(this->type));

    output.WriteUInt16(this->id);

    this->from->Serialize(output);

    return InstructionSerializer::Serialize(output, this->instruction.get());
  }


  std::unique_ptr<Package> Package::Deserialize(ContactPtr sender, bdfs::IInputStream & input)
  {
    if (!sender || input.Remainder() < sizeof(uint8_t) * 2 + sizeof(uint16_t) || input.ReadUInt8() != 0)
    {
      return nullptr;
    }

    uint8_t typeVal = input.ReadUInt8();
    if (typeVal >= static_cast<uint8_t>(PackageType::__MAX__))
    {
      return nullptr;
    }

    PackageType type = static_cast<PackageType>(typeVal);

    uint16_t id = input.ReadUInt16();

    auto from = std::make_shared<Key>();

    if (!from->Deserialize(input))
    {
      return nullptr;
    }

    Instruction * instr = InstructionSerializer::Deserialize(input);

    if (!instr)
    {
      return nullptr;
    }

    return std::unique_ptr<Package>(new Package(type, from, id, sender, std::unique_ptr<Instruction>(instr)));
  }
}
