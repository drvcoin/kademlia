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

#include <atomic>
#include <memory>
#include <drive/common/IOutputStream.h>
#include <drive/kad/Contact.h>
#include <drive/kad/Key.h>
#include <drive/kad/Instruction.h>

namespace kad
{
  class Package
  {
  public:

    enum class PackageType
    {
      Request,
      Response,
      __MAX__
    };

  public:

    explicit Package(PackageType type, KeyPtr from, ContactPtr tgt, std::unique_ptr<Instruction> && instr);

    explicit Package(PackageType type, KeyPtr from, uint16_t id, ContactPtr tgt, std::unique_ptr<Instruction> && instr);

    KeyPtr From() const                   { return this->from; }

    ContactPtr Target() const             { return this->target; }

    Instruction * GetInstruction() const  { return this->instruction.get(); }

    uint16_t Id() const                   { return this->id; }

    PackageType Type() const              { return this->type; }

    bool Serialize(bdfs::IOutputStream & output) const;

    static std::unique_ptr<Package> Deserialize(ContactPtr sender, bdfs::IInputStream & input);

  private:

    static std::atomic<uint16_t> gid;

  private:

    KeyPtr from;

    ContactPtr target;

    std::unique_ptr<Instruction> instruction;

    uint16_t id;

    PackageType type;
  };

  using PackagePtr = std::shared_ptr<Package>;
}
