/*-------------------------------------------------------------------------
 *
 * remote_dml.h
 *	  POSTGRES remote access method DML statements processing code.
 *
 *
 * Copyright Zhao Wei (david.zhao.cn@gmail.com)
 *
 * src/include/access/remote_dml.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef REMOTE_DML_H
#define REMOTE_DML_H
#include "utils/relcache.h"
#include "nodes/nodes.h"
#include "nodes/pg_list.h"
#include "nodes/primnodes.h"

extern void InitRemoteDMLContext();
extern void ClearRemoteDMLContext();
extern void set_remote_command(CmdType cmd0);
extern void add_remote_target_entry(const char *attname, Oid typid, Node *expr, List *rtable);
extern void add_remote_target_table(Relation rel);
extern void set_remote_returning_list(List *lret);
extern void set_remote_where_cond(Node *qual, List *rtable);
extern void set_remote_target_table(Relation rel);
extern void make_remote_dml_stmts();
extern bool is_remote_dml_stmt();
extern void exclude_unused_base_rel(Oid relid);
#endif // !REMOTE_DML_H
