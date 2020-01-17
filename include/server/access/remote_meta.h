/*-------------------------------------------------------------------------
 *
 * remote_meta.h
 *	  POSTGRES remote access method definitions.
 *
 *
 * Copyright Zhao Wei (david.zhao.cn@gmail.com)
 *
 * src/include/access/remote_meta.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef REMOTE_META_H
#define REMOTE_META_H

#include "postgres.h"
#include "access/tupdesc.h"
#include "utils/relcache.h"
#include "utils/rel.h"
#include "utils/algos.h"

extern bool replaying_ddl_log;
extern bool enable_remote_relations;
extern char *remote_stmt_ptr;

extern void make_remote_create_table_stmt1(Relation heaprel, const TupleDesc tupDesc);
extern void make_remote_create_table_stmt2(
    Relation indexrel, Relation heaprel, const TupleDesc tupDesc, bool is_primary,
    bool is_unique, int16*coloptions);

extern void RemoteDDLCxt_SetOrigSQL(const char *sql);
extern const char *show_remote_sql(void);
extern void SetRemoteContextShardId(Oid shardid);
extern Oid GetRemoteContextShardId();
extern void ResetRemoteDDLStmt();
extern void InitRemoteDDLContext();
extern void TrackRemoteDropTable(Oid relid, bool is_cascade);
extern void TrackRemoteDropTableStorage(Relation rel);
extern void TrackRemoteCreatePartitionedTable(Relation rel);
extern void TrackRemoteCreateIndex(Relation heaprel, const char *idxname, Oid amid,
    bool is_unique, bool is_partitioned);
extern void TrackRemoteDropIndex(Oid relid, bool is_cascade);
extern void TrackRemoteDropIndexStorage(Relation rel);
extern void end_metadata_txn(bool commit_it);
extern void set_dropping_tree(bool b);
extern void end_remote_ddl_stmt();
extern void RemoteDDLSetSkipStorageIndexing(bool b);
extern void RemoteCreateDatabase(const char *dbname);
extern void RemoteCreateSchema(const char *schemaName);
extern void RemoteDropDatabase(const char *db);
extern void RemoteDropSchema(const char *schema);
extern bool is_metadata_txn(uint64_t*p_opid);
#endif /* !REMOTE_META_H */
