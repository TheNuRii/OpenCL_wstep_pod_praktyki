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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

//===============================================================================================================================================================================================================
// xQueue - thread safe std::queue (FIFO) for any type of data
//===============================================================================================================================================================================================================
template <class XXX> class xQueue
{
protected:
  std::queue<XXX> m_Queue;
  uint32          m_QueueSize;

  //threading utils
  std::mutex              m_Mutex;
  std::condition_variable m_EnqueueConditionVariable;
  std::condition_variable m_DequeueConditionVariable;
  
public:
  xQueue(int32 QueueSize = 1) { setSize(QueueSize); }

  int32    getSize  (          ) const { return m_QueueSize; }
  void     setSize  (int32 Size)       { assert(Size>0); std::lock_guard<std::mutex> LockManager(m_Mutex); m_QueueSize = Size; m_EnqueueConditionVariable.notify_all(); }
  bool     isEmpty  (          ) const { return m_Queue.empty(); }
  bool     isFull   (          ) const { return (m_Queue.size() == m_QueueSize); }
  uintSize getLoad  (          ) const { return m_Queue.size(); }

  void EnqueueResize(XXX  Data);
  bool DequeueTry   (XXX& Data);

  void EnqueueWait  (XXX  Data);
  void DequeueWait  (XXX& Data);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class XXX> void xQueue<XXX>::EnqueueResize(XXX Data)
{
  m_Mutex.lock();
  if(isFull()) { m_QueueSize++; }
  m_Queue.push(Data);
  m_Mutex.unlock();
  m_DequeueConditionVariable.notify_one();
}
template<class XXX> bool xQueue<XXX>::DequeueTry(XXX& Data)
{
  m_Mutex.lock(); 
  if(!m_Queue.empty())
  {
    Data = m_Queue.front();
    m_Queue.pop();
    m_Mutex.unlock();
    m_EnqueueConditionVariable.notify_one();
    return true;
  }
  else
  {
    m_Mutex.unlock();
    return false;
  }
}
template<class XXX> void xQueue<XXX>::EnqueueWait(XXX Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.size()>=m_QueueSize) { m_EnqueueConditionVariable.wait(LockManager, [&]{ return m_Queue.size()<m_QueueSize; } ); }
  m_Queue.push(Data);
  LockManager.unlock();
  m_DequeueConditionVariable.notify_one();
  //release lock - std::unique_lock destructor... 
}
template<class XXX> void xQueue<XXX>::DequeueWait(XXX& Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.empty()) { m_DequeueConditionVariable.wait(LockManager, [&]{ return !m_Queue.empty(); } ); }
  Data = m_Queue.front();
  m_Queue.pop();
  LockManager.unlock();
  m_EnqueueConditionVariable.notify_one();
}

//===============================================================================================================================================================================================================

