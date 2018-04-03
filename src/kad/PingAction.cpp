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
#include "Thread.h"
#include "PackageDispatcher.h"
#include "Config.h"
#include "PingAction.h"


namespace kad
{
  PingAction::PingAction(Thread * owner, PackageDispatcher * dispatcher)
    : Action(owner, dispatcher)
  {
  }

  
  void PingAction::Initialize(ContactPtr contact)
  {
    this->contact = contact;
  }


  bool PingAction::Start()
  {
    if (!this->contact)
    {
      return false;
    }

    this->Send();
    return true;
  }


  bool PingAction::GetResult()
  {
    return this->IsCompleted() && this->result;
  }


  void PingAction::Send()
  {
    THREAD_ENSURE(this->owner, Send);

    auto package = std::make_shared<Package>(Package::PackageType::Request, Config::NodeId(), this->contact, std::unique_ptr<protocol::Ping>(new protocol::Ping()));

    using namespace std::placeholders;

    this->dispatcher->Send(package, std::bind(&PingAction::OnResponse, this, _1, _2));
  }


  void PingAction::OnResponse(PackagePtr request, PackagePtr response)
  {
    if (response)
    {
      Instruction * instr = response->GetInstruction();
      if (instr && instr->Code() == OpCode::PONG)
      {
        this->result = true;
      }
    }

    this->Complete();

    if (this->onComplete)
    {
      this->onComplete(this->onCompleteSender, this);
    }

    delete this;
  }
}