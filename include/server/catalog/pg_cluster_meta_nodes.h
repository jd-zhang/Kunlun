/*-------------------------------------------------------------------------
 *
 * pg_cluster_meta_nodes.h
 *	  definition of the "cluster_meta_nodes" system catalog (pg_cluster_meta_nodes)
 *
 *
 * Copyright (c) Zhao Wei (david.zhao.cn@gmail.com)
 *
 * src/include/catalog/pg_cluster_meta_nodes.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */

#ifndef PG_CLUSTER_META_NODES_H
#define PG_CLUSTER_META_NODES_H

#include "catalog/genbki.h"
#include "catalog/pg_cluster_meta_nodes_d.h"


/*
 * This meta table contains connect information about the meta data cluster nodes.
 * */
CATALOG(pg_cluster_meta_nodes,12350,ClusterMetaNodesRelationId) BKI_SHARED_RELATION BKI_WITHOUT_OIDS
{
  Oid server_id; /* mysql server_id variable value. used as PK to identify the row */
  Oid cluster_id;/* owner cluster's id, it's the same for all rows of one such table. */
  /*
   * Whether this node is master. There can be only one master in this table.
   * FormData_pg_cluster_meta::cluster_master_id is the master's server_id.
   * In case of master switch in meta cluster, the update of 'is_master'
   * fields in old and new master rows and the update of
   * FormData_pg_cluster_meta::cluster_master_id must always be in the same
   * transaction in order for the system to see consistent master info.
   * */
  bool is_master;
  int32 port;
  NameData ip; /* ip address, ipv6 or ipv4. */
  NameData user_name;
#ifdef CATALOG_VARLEN
  text passwd BKI_FORCE_NOT_NULL;
#endif
} FormData_pg_cluster_meta_nodes;

typedef FormData_pg_cluster_meta_nodes*Form_pg_cluster_meta_nodes;
#endif /* !PG_CLUSTER_META_NODES_H */
