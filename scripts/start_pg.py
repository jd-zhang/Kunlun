import sys
import os
#arg: port
if __name__ == "__main__":
    args = dict([arg.split('=') for arg in sys.argv[1:]])
    port = args["port"]

    install_path = os.getcwd()[:-8] # cut off the /scripts tail
    os.system("export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:" + install_path + "/lib")

    # open the instances_list to look for target instance's datadir
    etc_path = install_path + "/etc"
    conf_list_file = etc_path+"/instances_list.txt"
    if not os.path.exists(conf_list_file):
        raise Error("instance list file " + conf_list_file + " not exist!")

    # sequentially find target port's datadir from instances_list file
    fp_conf_list = open(conf_list_file, 'r')
    lines = fp_conf_list.readlines()
    datadir = ''
    for line in lines:
        if '==>' in line:
            cfg = line.split('==>')
            if cfg[0] == port:
                datadir = cfg[1][:-1]
                break

    pg_logfp = datadir + "/logfile-" + port
    startup_cmd = install_path + '/bin/postgres -D ' + datadir + " > " + pg_logfp + " 2>&1 &"
    os.system(startup_cmd)
