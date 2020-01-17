/*-------------------------------------------------------------------------
 *
 * remotetup.h
 *	  POSTGRES remote tuple accumulation and send.
 *
 *
 * Copyright Zhao Wei (david.zhao.cn@gmail.com)
 *
 * src/include/access/remotetup.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef REMOTE_TUP_H
#define REMOTE_TUP_H

#include "postgres.h"
#include "catalog/pg_type.h"
#include "utils/algos.h"
#include "executor/tuptable.h"
#include "nodes/execnodes.h"
#include "lib/stringinfo.h"

extern StringInfo CreateRemotetupCacheState(Relation rel);
extern bool cache_remotetup(TupleTableSlot *slot, StringInfo str);
extern void end_remote_insert_stmt(StringInfo str, bool eos);
extern size_t GetInsertedNumRows(StringInfo str);

inline static bool is_date_time_type(Oid typid)
{
    const static Oid dttypes[] =
        {TIMETZOID, TIMESTAMPTZOID, TIMESTAMPOID, DATEOID, TIMEOID, INTERVALOID,
         TIMESTAMPARRAYOID, DATEARRAYOID, TIMEARRAYOID, TIMESTAMPTZARRAYOID,
         INTERVALARRAYOID, TIMETZARRAYOID };

    for (int i = 0; i < sizeof(dttypes) / sizeof(Oid); i++)
        if (dttypes[i] == typid)
            return true;
    return false;
}

/*
 * See if 'typid' is of types that can produce interval values by substracting.
 * */
inline static bool is_interval_opr_type(Oid typid)
{
    const static Oid dttypes[] =
        {TIMESTAMPTZOID, TIMESTAMPOID, DATEOID};

    for (int i = 0; i < sizeof(dttypes) / sizeof(Oid); i++)
        if (dttypes[i] == typid)
            return true;
    return false;
}
#endif // !REMOTE_TUP_H
