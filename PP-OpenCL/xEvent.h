#pragma once

/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2019, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * int32_tERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

 // Original author: Jakub Stankowski, jakub.stankowski@put.poznan.pl,
 //                  Poznan University of Technology, Poznañ, Poland

#include "CommonDef.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

//===============================================================================================================================================================================================================
// xEvent - thread safe binary event
//===============================================================================================================================================================================================================
class xEvent
{
protected:
  bool                    m_State;
  std::mutex              m_Mutex;
  std::condition_variable m_ConditionVariable;

public:
  xEvent(bool InitialState) : m_State(InitialState) {}
  xEvent(const xEvent&) = delete;
  xEvent& operator=(const xEvent&) = delete;

  inline void set  ()
  {
    m_Mutex.lock();
    m_State = true;
    m_Mutex.unlock();
    m_ConditionVariable.notify_all();
  }
  inline void reset()
  {
    m_Mutex.lock();
    m_State = false;
    m_Mutex.unlock();
  }
  inline void wait ()
  {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while(m_State == false) { m_ConditionVariable.wait(LockManager, [&] { return m_State; }); }
  }
};

//===============================================================================================================================================================================================================

