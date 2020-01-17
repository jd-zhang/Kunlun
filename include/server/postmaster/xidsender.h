/*-------------------------------------------------------------------------
 *
 * xidsender.h
 *	  definitions of types, and declarations of functions, that are used in
 *	  xidsender background process.
 *
 *
 * Copyright (c) Zhao Wei (david.zhao.cn@gmail.com)
 *
 * src/include/postmaster/xidsender.h
 *
 *-------------------------------------------------------------------------
 */
typedef uint64_t GlobalTrxId;
extern void CreateSharedBackendXidSlots(void);
extern Size BackendXidSenderShmemSize(void);
extern void xidsender_initialize(void);
extern void XidSenderMain(int argc, char **argv);
extern int  xidsender_start(void);
extern bool WaitForXidCommitLogWrite(Oid comp_nodeid, GlobalTrxId xid, time_t deadline, bool commit_it);
extern void wait_latch(int millisecs);

extern bool XidSyncDone();
extern bool skip_tidsync;
