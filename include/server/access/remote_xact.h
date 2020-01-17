/*-------------------------------------------------------------------------
 *
 * remote_xact.h
 *	  POSTGRES global transaction mgmt code
 *
 *
 * Copyright Zhao Wei (david.zhao.cn@gmail.com)
 *
 * src/include/access/remote_xact.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef REMOTE_XACT_H
#define REMOTE_XACT_H

#include "access/xact.h"

typedef enum GTxnDD_Victim_Policy
{
    KILL_OLDEST,
    KILL_YOUNGEST,
    KILL_MOST_ROWS_CHANGED,
    KILL_LEAST_ROWS_CHANGED,
    KILL_MOST_ROWS_LOCKED
} GTxnDD_Victim_Policy;

extern GTxnDD_Victim_Policy g_glob_txnmgr_deadlock_detector_victim_policy;
extern void perform_deadlock_detect();
extern void gdd_init();

extern void StartSubTxnRemote(const char *name);
extern void SendReleaseSavepointToRemote(const char *name);
extern void SendRollbackRemote(const char *txnid, bool xa_end);
extern void SendRollbackSubToRemote(const char *name);
extern bool Send1stPhaseRemote(const char *txnid);
extern void Send2ndPhaseRemote(const char *txnid);
extern void StartTxnRemote(StringInfo cmd);
extern char *MakeTopTxnName(TransactionId txnid, time_t now);

#endif // !REMOTE_XACT_H
