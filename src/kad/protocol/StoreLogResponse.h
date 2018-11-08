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

#include <drive/kad/Instruction.h>

namespace kad
{
  namespace protocol
  {
    class StoreLogResponse : public Instruction
    {
    public:

      enum class ErrorCode
      {
        SUCCESS = 0,
        FAILED = 1,
        OUT_OF_DATE = 2,
        __MAX__
      };

      StoreLogResponse();
      ~StoreLogResponse() override = default;

      bool Serialize(bdfs::IOutputStream & output) const override;

      bool Deserialize(bdfs::IInputStream & input) override;

      void Print() const override;

      ErrorCode Result() const            { return this->result; }

      void SetResult(ErrorCode value)     { this->result = value; }

    private:

      ErrorCode result = ErrorCode::FAILED;
    };
  }
}
