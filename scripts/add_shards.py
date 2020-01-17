import psycopg2
import mysql.connector
import argparse
import json
import time


# shards config file format:
# [
#   {
#   "shard_name": "shard-name",
#   "shard_nodes":
#   {
#       "master":
#       {
#          "ip": "127.0.0.1",
#          "port": 5432,
#          "user": "zw",
#          "password":"zw"
#       },
#   
#       "slave":
#       {
#          "ip": "127.0.0.1",
#          "port": 5432,
#          "user": "zw",
#          "password":"zw"
#       },
#       { more slaves as needed }
#   }
# ,
# { more shard configs like above}
# ]

def add_shards_to_cluster(mysql_conn_params, args, config_path):
    meta_conn = mysql.connector.connect(**mysql_conn_params)


    jsconf = open(config_path);
    jstr = jsconf.read()
    jscfg = json.loads(jstr);
    meta_cursor = meta_conn.cursor(prepared=True)
    get_cluster_id_stmt = "select id from db_clusters where name=%s"
    meta_cursor.execute(get_cluster_id_stmt, (args.cluster_name,))
    row = meta_cursor.fetchone();
    cluster_id = row[0]

    master_node_id = 0
    num_nodes = 0

    add_shard_stmt = "insert into shards(name, when_created, master_node_id, num_nodes, db_cluster_id) values(%s, now(), 0, 0, %s)"
    add_shard_node_stmt = "insert into shard_nodes(ip, port, user_name, passwd, shard_id, db_cluster_id, svr_node_id, master_priority) values(%s, %s, %s, %s, %s, %s, 0,0)"

    # add shards info to metadata-cluster tables.
    masters = []
    meta_cursor0 = meta_conn.cursor(buffered=True, dictionary=True)
    meta_cursor0.execute("start transaction")
    for shardcfg in jscfg:
        meta_cursor.execute(add_shard_stmt, (shardcfg['shard_name'], cluster_id))
        meta_cursor0.execute("select last_insert_id() as id");
        row = meta_cursor0.fetchone();
        shard_id = row['id'];
        shardcfg['shard_id'] = shard_id

        for key, val in shardcfg['shard_nodes'].iteritems():
            meta_cursor.execute(add_shard_node_stmt, (val['ip'], val['port'], val['user'], val['password'], shard_id, cluster_id))
            meta_cursor0.execute("select last_insert_id() as id")
            row = meta_cursor0.fetchone();
            val['shard_node_id'] = row['id']
            num_nodes=num_nodes+1
            if key == 'master' :
                master_node_id = row['id']
                masters.append(shardcfg['shard_nodes']['master'])

        shardcfg['master_node_id'] = master_node_id
        meta_cursor.execute("update shards set master_node_id=?, num_nodes=? where id=?", (master_node_id, num_nodes, shard_id))

    meta_cursor0.execute("commit")
    meta_cursor.close()
    meta_cursor0.close()

    # create the default database in each new shard.
    for master in masters:
        mysql_conn_params2 = {
            'host':master['ip'],
            'port':master['port'],
            'user':master['user'],
            'password':master['password']
        }
        master_conn = mysql.connector.connect(**mysql_conn_params2)
        master_cursor = master_conn.cursor()
        master_cursor.execute("create database postgres_$$_public CHARACTER set=utf8")
        master_cursor.close()
        master_cursor.close()

    add_new_shards_to_all_computing_nodes(cluster_id, num_nodes, meta_conn, jscfg)
    meta_conn.close()

def add_new_shards_to_all_computing_nodes(cluster_id, num_nodes, meta_conn, jscfg):

    meta_cursor0 = meta_conn.cursor(buffered=True, dictionary=True)
    meta_cursor0.execute("select * from comp_nodes")
    time.sleep(5)

    for row in meta_cursor0:
        conn = psycopg2.connect(host=row['ip'], port=row['port'], user=row['user_name'], database='postgres', password=row['passwd'])
        cur = conn.cursor()
        nretries = 0
        while nretries < 10:
            try:
                cur.execute("start transaction")
                for shardcfg in jscfg:
                    cur.execute("insert into pg_shard (name, id, num_nodes, master_node_id, space_volumn, num_tablets, db_cluster_id, when_created) values(%s, %s, %s, %s,%s,%s,%s, now())",
                            (shardcfg['shard_name'], shardcfg['shard_id'], num_nodes, shardcfg['master_node_id'], 0, 0, cluster_id))
                    for k, v in shardcfg['shard_nodes'].iteritems():
                        cur.execute("insert into pg_shard_node values(%s, %s, %s, %s, %s, %s, %s, %s, now())",
                                (v['shard_node_id'], v['port'], shardcfg['shard_id'], 0, 0, v['ip'], v['user'], v['password']))
                cur.execute("commit")
                break;
            except psycopg2.Error as pgerr:
                cur.execute("rollback")
                nretries = nretries + 1
                print "Got error: " + str(pgerr) + ". Will retry in 2 seconds, " + str(nretries) +" of 10 retries."
                time.sleep(2)

        cur.close()
        conn.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Add one or more shard(s) to the cluster.')
    parser.add_argument('--config', help="shard config file path");
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
        'password':args.meta_pwd,
    }
    add_shards_to_cluster(mysql_conn_params, args, args.config)
    print "Shard nodes successfully added to cluster " + args.cluster_name
