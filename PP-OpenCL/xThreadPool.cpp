#include "xThreadPool.h"

//===============================================================================================================================================================================================================

void xThreadPool::create(int32 NumThreads, int32 QueueSize)
{
  assert(NumThreads>0);
  assert(QueueSize >0);

  m_NumThreads = NumThreads;
  m_WaitingTasks  .setSize(QueueSize);
  m_CompletedTasks.setSize(QueueSize);

  for(int32 i=0; i<m_NumThreads; i++)
  {
    std::packaged_task<uint32(xThreadPool*)> PackagedTask(xThreadStarter);
    m_Future.push_back(PackagedTask.get_future());
    std::thread Thread = std::thread(std::move(PackagedTask), this);
    m_ThreadId.push_back(Thread.get_id());
    m_Thread  .push_back(std::move(Thread));      
  }

  for(int32 TaskIdx = 0; TaskIdx < QueueSize * 2; TaskIdx++) { m_UnusedTasks.push(new xPoolTask); }

  m_EventCreated.set();
}
void xThreadPool::destroy()
{
  assert(isWaitingQueueEmpty());

  for(int32 i=0; i<m_NumThreads; i++)
  {
    xPoolTask* Terminator = new xPoolTask;
    Terminator->setTerminator();
    m_WaitingTasks.EnqueueWait(Terminator);
  }

  bool AnyActive = false;
  for(int32 i=0; i<m_NumThreads; i++)
  {   
    std::future_status Status = m_Future[i].wait_for(std::chrono::milliseconds(500));
    if(Status == std::future_status::ready && m_Thread[i].joinable()) { m_Thread[i].join(); }
    else                                                              { AnyActive = true;   }
  }

  if(AnyActive)
  {
    for(int32 i=0; i<m_NumThreads; i++)
    {
      std::future_status Status = m_Future[i].wait_for(std::chrono::seconds(5));
      if(Status == std::future_status::ready && m_Thread[i].joinable()) { m_Thread[i].join();    }
      else                                                              { m_Thread[i].~thread(); }
    }
  }

  //cleanup queues
  while(!m_WaitingTasks  .isEmpty()) { xPoolTask* Task; m_CompletedTasks.DequeueWait(Task); delete Task; }
  while(!m_CompletedTasks.isEmpty()) { xPoolTask* Task; m_CompletedTasks.DequeueWait(Task); delete Task; }
  while(!m_UnusedTasks.empty()) { xPoolTask* Task = m_UnusedTasks.top(); m_UnusedTasks.pop(); delete Task; }
}
void xThreadPool::addWaitingTask(std::function<void(int32)> Function)
{
  xPoolTask* Task = m_UnusedTasks.top(); 
  m_UnusedTasks.pop();
  Task->setFunction(Function);
  m_WaitingTasks.EnqueueWait(Task);
}
void xThreadPool::waitUntilTasksFinished(int32 NumTasksToWaitFor)
{
  for(int32 TaskId = 0; TaskId < NumTasksToWaitFor; TaskId++)
  {
    xPoolTask* Task;
    m_CompletedTasks.DequeueWait(Task);
    m_UnusedTasks.push(Task);
  }
}
uint32 xThreadPool::xThreadFunc()
{
  m_EventCreated.wait();
  std::thread::id ThreadId = std::this_thread::get_id();
  int32 ThreadIdx = (int32)(std::find(m_ThreadId.begin(), m_ThreadId.end(), ThreadId) - m_ThreadId.begin());
  while(1)
  {    
    xPoolTask* Task;
    m_WaitingTasks.DequeueWait(Task);
    if(Task->getStatus() == xPoolTask::eStatus::Terminate) { delete Task; break; }
    xPoolTask::StarterFunction(Task, ThreadIdx);
    m_CompletedTasks.EnqueueWait(Task);
  }
  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
