# add one or more computing nodes

import psycopg2
import mysql.connector
import argparse
import json



# config file format:
#[
#   {
#      "id":2,  this ID must be identical to the 'comp_node_id' variable in this computing node's postgresql.conf config file.
#      "name":"comp3",
#      "ip":"127.0.0.1",
#      "port":5433,
#      "user":"zw",
#      "password":"zw"
#   },    
#   {
#      "id":3,
#      "name":"comp4",
#      "ip":"127.0.0.1",
#      "port":5434,
#      "user":"zw",
#      "password":"zw"
#   }
#]


def add_computing_nodes(mysql_conn_params, args, config_path) :
    meta_conn = mysql.connector.connect(**mysql_conn_params)
    jsconf = open(config_path);
    jstr = jsconf.read()
    jscfg = json.loads(jstr);

    meta_cursor = meta_conn.cursor(prepared=True)
    get_cluster_id_stmt = "select id, @@server_id as svrid from db_clusters where name=%s"
    meta_cursor.execute(get_cluster_id_stmt, (args.cluster_name,))
    row = meta_cursor.fetchone();
    cluster_id = row[0]
    cluster_master_svrid = row[1]

    meta_cursor0 = meta_conn.cursor(buffered=True, dictionary=True)

    # insert computing nodes info into meta-tables.
    stmt = "insert into comp_nodes(id, name, ip, port, db_cluster_id,user_name,passwd) values(%s, %s, %s, %s, %s, %s, %s)"
    meta_cursor0.execute("start transaction")

    for compcfg in jscfg:
        meta_cursor.execute(stmt, (compcfg['id'], compcfg['name'],compcfg['ip'], compcfg['port'], cluster_id, compcfg['user'], compcfg['password']))
    meta_cursor.close()
    meta_cursor0.execute("select*from meta_db_nodes")
    meta_dbnodes = meta_cursor0.fetchall()
    meta_master_id = 0

    meta_cursor0.execute("select * from shards where db_cluster_id=" + str(cluster_id))
    shard_rows = meta_cursor0.fetchall()

    meta_cursor0.execute("select * from shard_nodes where db_cluster_id= " + str(cluster_id))
    shard_node_rows = meta_cursor0.fetchall()

    meta_cursor0.execute("commit")

    # create a partition for each computing node in commit log table. DDLs can't be prepared so we have to connect strings.
    for compcfg in jscfg:
        meta_cursor0.execute("alter table commit_log_" + args.cluster_name + " add partition(partition p" + str(compcfg['id']) + " values in (" + str(compcfg['id']) + "))");

    # insert meta data into each computing node's catalog tables.
    for compcfg in jscfg:
        conn = psycopg2.connect(host=compcfg['ip'], port=compcfg['port'], user=compcfg['user'], database='postgres', password=compcfg['password'])
        cur = conn.cursor()
        cur.execute("set skip_tidsync = true; start transaction")
        cur.execute("insert into pg_cluster_meta values(%s, %s, %s, %s, %s)",
                (compcfg['id'], cluster_id, 0, args.cluster_name, compcfg['name']))
        for meta_node in meta_dbnodes:
            is_master = False
            
            if meta_node['ip'] == args.meta_host and meta_node['port'] == args.meta_port:
                is_master = True
                meta_master_id = meta_node['id']

            cur.execute("insert into pg_cluster_meta_nodes values(%s, %s, %s, %s, %s, %s, %s)",
                    (meta_node['id'], cluster_id, is_master, meta_node['port'], meta_node['ip'], meta_node['user_name'], meta_node['passwd']))
        # if this is from a backup then it may already have some or all shard info, proceed anyway.
        cur1 = conn.cursor()
        cur1.execute("select id from pg_shard");
        shardids = cur1.fetchall();
        for shard_row in shard_rows:
            if shard_row['id'] in shardids:
                continue
            cur.execute("insert into pg_shard (name, id, master_node_id, num_nodes, space_volumn, num_tablets, db_cluster_id, when_created) values(%s, %s, %s, %s, %s,%s,%s,%s)",
                    (shard_row['name'], shard_row['id'], shard_row['master_node_id'], len(shard_node_rows), shard_row['space_volumn'],
                     shard_row['num_tablets'], shard_row['db_cluster_id'], shard_row['when_created']))

        cur1.execute("select id from pg_shard_node");
        shardnodeids = cur1.fetchall();
        for shard_node_row in shard_node_rows:
            if shard_node_row['id'] in shardnodeids:
                continue
            cur.execute("insert into pg_shard_node values(%s, %s, %s, %s, %s, %s, %s, %s, %s)",
                    (shard_node_row['id'], shard_node_row['port'], shard_node_row['shard_id'], 0, 0,
                     shard_node_row['ip'], shard_node_row['user_name'], shard_node_row['passwd'], shard_node_row['when_created']))

        cur.execute("update pg_cluster_meta set cluster_master_id=%s where cluster_name=%s",(meta_master_id, args.cluster_name))
        cur.execute("commit")
        cur.close()
        cur1.close()
        conn.close()

    meta_cursor.close()
    meta_cursor0.close()
    meta_conn.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Add one or more shard(s) to the cluster.')
    parser.add_argument('--config', type=str, help="shard config file path");
    parser.add_argument('--meta_host', type=str);
    parser.add_argument('--meta_port', type=int);
    parser.add_argument('--meta_user', type=str);
    parser.add_argument('--meta_pwd', type=str);
    parser.add_argument('--cluster_name', type=str);

    args = parser.parse_args();
    mysql_conn_params = {
        'host':args.meta_host,
        'port':args.meta_port,
        'database':'pxm',
        'user':args.meta_user,
        'password':args.meta_pwd
    }
    add_computing_nodes(mysql_conn_params, args, args.config)
    print "Computing nodes successfully added to cluster " + args.cluster_name
