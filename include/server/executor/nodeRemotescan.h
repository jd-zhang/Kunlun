/*-------------------------------------------------------------------------
 *
 * nodeRemotescan.h
 *
 *
 * Copyright Zhao Wei (david.zhao.cn@gmail.com)
 *
 *
 * src/include/executor/nodeRemotescan.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEREMOTESCAN_H
#define NODEREMOTESCAN_H

#include "access/parallel.h"
#include "nodes/execnodes.h"

extern RemoteScanState *ExecInitRemoteScan(RemoteScan *node, EState *estate, int eflags);
extern void ExecEndRemoteScan(RemoteScanState *node);
extern void ExecReScanRemoteScan(RemoteScanState *node);

/* parallel scan support */
extern void ExecRemoteScanEstimate(RemoteScanState *node, ParallelContext *pcxt);
extern void ExecRemoteScanInitializeDSM(RemoteScanState *node, ParallelContext *pcxt);
extern void ExecRemoteScanReInitializeDSM(RemoteScanState *node, ParallelContext *pcxt);
extern void ExecRemoteScanInitializeWorker(RemoteScanState *node,
							ParallelWorkerContext *pwcxt);

#endif							/* NODEREMOTESCAN_H */
