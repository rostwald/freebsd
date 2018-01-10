//===-- ThreadGDBRemote.h ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ThreadGDBRemote_h_
#define liblldb_ThreadGDBRemote_h_

// C Includes
// C++ Includes
#include <string>

// Other libraries and framework includes
// Project includes
#include "lldb/Target/Process.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/StructuredData.h"

class StringExtractor;

namespace lldb_private {
namespace process_gdb_remote {

class ProcessGDBRemote;

class ThreadGDBRemote : public Thread {
public:
  ThreadGDBRemote(Process &process, lldb::tid_t tid);

  ~ThreadGDBRemote() override;

  void WillResume(lldb::StateType resume_state) override;

  void RefreshStateAfterStop() override;

  const char *GetName() override;

  const char *GetQueueName() override;

  lldb::QueueKind GetQueueKind() override;

  lldb::queue_id_t GetQueueID() override;

  lldb::QueueSP GetQueue() override;

  lldb::addr_t GetQueueLibdispatchQueueAddress() override;

  void SetQueueLibdispatchQueueAddress(lldb::addr_t dispatch_queue_t) override;

  bool ThreadHasQueueInformation() const override;

  lldb::RegisterContextSP GetRegisterContext() override;

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(StackFrame *frame) override;

  void Dump(Log *log, uint32_t index);

  static bool ThreadIDIsValid(lldb::tid_t thread);

  bool ShouldStop(bool &step_more);

  const char *GetBasicInfoAsString();

  void SetName(const char *name) override {
    if (name && name[0])
      m_thread_name.assign(name);
    else
      m_thread_name.clear();
  }

  lldb::addr_t GetThreadDispatchQAddr() { return m_thread_dispatch_qaddr; }

  void SetThreadDispatchQAddr(lldb::addr_t thread_dispatch_qaddr) {
    m_thread_dispatch_qaddr = thread_dispatch_qaddr;
  }

  void ClearQueueInfo();

  void SetQueueInfo(std::string &&queue_name, lldb::QueueKind queue_kind,
                    uint64_t queue_serial, lldb::addr_t dispatch_queue_t,
                    lldb_private::LazyBool associated_with_libdispatch_queue);

  lldb_private::LazyBool GetAssociatedWithLibdispatchQueue() override;

  void SetAssociatedWithLibdispatchQueue(
      lldb_private::LazyBool associated_with_libdispatch_queue) override;

  StructuredData::ObjectSP FetchThreadExtendedInfo() override;

protected:
  friend class ProcessGDBRemote;

  std::string m_thread_name;
  std::string m_dispatch_queue_name;
  lldb::addr_t m_thread_dispatch_qaddr;
  lldb::addr_t m_dispatch_queue_t;
  lldb::QueueKind
      m_queue_kind; // Queue info from stop reply/stop info for thread
  uint64_t
      m_queue_serial_number; // Queue info from stop reply/stop info for thread
  lldb_private::LazyBool m_associated_with_libdispatch_queue;

  bool PrivateSetRegisterValue(uint32_t reg, llvm::ArrayRef<uint8_t> data);

  bool PrivateSetRegisterValue(uint32_t reg, uint64_t regval);

  bool CachedQueueInfoIsValid() const {
    return m_queue_kind != lldb::eQueueKindUnknown;
  }
  void SetStopInfoFromPacket(StringExtractor &stop_packet, uint32_t stop_id);

  bool CalculateStopInfo() override;
};

} // namespace process_gdb_remote
} // namespace lldb_private

#endif // liblldb_ThreadGDBRemote_h_
