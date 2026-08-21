#pragma once
#include "CommonDef.h"
#include "xQueue.h"
#include "xEvent.h"
#include <vector>
#include <future>
#include <stack>

//===============================================================================================================================================================================================================

class xThreadPool
{
public:
  class xPoolTask
  {
  public:
    enum class eStatus
    {
      INVALID = NOT_VALID,
      UNKNOWN = 0,
      Waiting,
      Processed,
      Completed,
      Terminate,
    };

  protected:
    eStatus                    m_Status;
    std::function<void(int32)> m_Function; //void Function(int32 ThreadIdx)

  protected:
    void WorkingFunction(int32 ThreadIdx) { m_Function(ThreadIdx); }

  public:
    xPoolTask() { m_Status = eStatus::UNKNOWN; }

    inline void    setFunction  (std::function<void(int32)> Function) { m_Function = Function; m_Status = eStatus::Waiting;   }
    inline void    setTerminator(                                   ) { m_Function = nullptr ; m_Status = eStatus::Terminate; }
    inline void    setStatus(eStatus Status) { m_Status = Status; }
    inline eStatus getStatus() const         { return m_Status; }

    static void StarterFunction(xPoolTask* WorkerTask, int32 ThreadIdx)
    {
      assert(WorkerTask->m_Status == eStatus::Waiting);
      WorkerTask->m_Status = eStatus::Processed;
      WorkerTask->WorkingFunction(ThreadIdx);
      WorkerTask->m_Status = eStatus::Completed;
    }
  };

protected:
  //threads data
  int32                            m_NumThreads;
  xEvent                           m_EventCreated;
  std::vector<std::future<uint32>> m_Future;
  std::vector<std::thread>         m_Thread;
  std::vector<std::thread::id>     m_ThreadId;

  //input & output queques
  xQueue    <xPoolTask*> m_WaitingTasks;
  xQueue    <xPoolTask*> m_CompletedTasks;
  std::stack<xPoolTask*> m_UnusedTasks;
  

protected:  
  uint32        xThreadFunc();
  static uint32 xThreadStarter(xThreadPool* ThreadPool) { return ThreadPool->xThreadFunc(); }

public:
  xThreadPool() : m_EventCreated(false) { m_NumThreads = 0; }

  void   create   (int32 NumThreads, int32 QueueSize);
  void   destroy  ();

  void   addWaitingTask        (std::function<void(int32)> Function);
  void   waitUntilTasksFinished(int32 NumTasksToWaitFor);

  int32  getWaitingQueueSize  () const { return m_WaitingTasks.getSize(); }
  bool   isWaitingQueueEmpty  () const { return m_WaitingTasks.isEmpty(); }
  int32  getCompletedQueueSize() const { return m_CompletedTasks.getSize(); }
  bool   isCompletedQueueEmpty() const { return m_CompletedTasks.isEmpty(); }
  int32  getNumThreads        () const { return m_NumThreads; }
};

//===============================================================================================================================================================================================================
