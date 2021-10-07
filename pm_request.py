import requests as re
import json
import time
import subprocess
import os


def Get_Data():
    _url = "http://"
    #Query to get usage
    usage_query = "sum(container_fs_usage_bytes{container!='POD', id!='/'}) by (name, pod)"
    #Query to get limited disk capacity
    limit_query = "sum(kube_pod_container_resource_limits{resource='ephemeral_storage'}) by (name,pod)"
    ip = "192.168.0.9:"
    port = "80"
    end_point = "/api/v1/query"
    url = _url+ip+port+end_point
    data = re.get(url,params={"query":usage_query}).json()
    data2 = re.get(url,params={"query":limit_query}).json()
    
    return data['data'], data2['data']

def check_disk_usage(name, use, limit):
    #docker shell script option 
    res = subprocess.check_output('docker inspect '+ name +' | grep -i upperdir', shell=True)
    
    con_path = str(res).split('"')[3]
    qsh_mt = "/.qsh_mt"

    
    f = open(con_path+qsh_mt, 'w')
    #container disk capacity check (85% or more)
    if 85 < (use/limit)*100:
        #f.write("00\0")
        print("85% up")
    else:
        #f.write("10\0")
        print("85% down : " + con_path+qsh_mt)
    
    f.close()
    

def change_byte(use_val, limit_val, con_name):
    
    byte = 1024 
    kbyte = byte*byte
    mbyte = kbyte*byte
    gbyte = mbyte*byte
    
    print("#####################################") 
    print("container_name : ",con_name)

    if( use_val < byte):
        print("usage : ",round(use_val,2),'B')
    elif( byte <= use_val and use_val < kbyte):
        print("usage : ",round(use_val/byte,2),'KB')
    elif( kbyte <= use_val and use_val < mbyte):
        print("usage : ",round(use_val/kbyte,2),'MB')
    elif( mbyte <= use_val and use_val < gbyte):
        print("usage : ",round(use_val/mbyte,2),'GB')
    else:
        print("usage : ",round(use_val/gbyte,2),'TB') 
    
    if( limit_val < byte):
        print("limit_disk : ",round(limit_val,2),'B')
    elif( byte <= limit_val and limit_val < kbyte):
        print("limit_disk : ",round(limit_val/byte,2),'KB')
    elif( kbyte <= limit_val and limit_val < mbyte):
        print("limit_disk : ",round(limit_val/kbyte,2),'MB')
    elif( mbyte <= limit_val and limit_val < gbyte):
        print("limit_disk : ",round(limit_val/mbyte,2),'GB')
    else:
        print("limit_disk : ",round(limit_val/gbyte,2),'TB')
    
    print("use(%) : ",round((use_val/limit_val)*100,2),"%")



def script_init(spare_disk,default_size, count):
    shell_path = "/root/init_sh_qsh.sh"
   
    script = "#!/bin/bash\n","path=$1\n","(echo n; echo e; echo \"\"; echo \"\"; echo \"\"; echo w;) | fdisk $path\n","size=$2\n","size='+'$size\n","for((i=0; i<$3; i++))\n","do\n","    echo -n temp\\ >> /root/qsh_backup_disk/qsh_path\n","    dir=\"/root/qsh_backup_disk/qsh_host_\"\n","    (echo n; echo l; echo \"\"; echo $size; echo w;) | fdisk $path\n","    partprobe\n","    path_tmp=`fdisk -l $path | tail -n 1 | cut -d ' ' -f1`\n","    path_sl=${path_tmp:5}\n","    dir=$dir$path_sl\n","    mkdir $dir\n","    chmod 755 $dir\n","    (echo y;) | mkfs.xfs -f $path_tmp\n","    mount $path_tmp $dir\n","    echo -n $dir\\ >> /root/qsh_backup_disk/qsh_path\n","done\n","mkdir /root/qsh_backup_disk/temp\n"


    #shell script create
    f = open(shell_path, 'w')
    for sc in script:
        f.write(sc)
        #print(sc)
    f.close()
    
    #shell script permissions
    os.system('chmod 0755 '+shell_path)

    #disk partition size
    size = str(default_size/count)+'G'

    #shell script exec
    print(shell_path+' '+spare_disk+' '+size+' '+str(count))
    #os.system(shell_path+' '+spare_disk+' '+size+' '+count)

    #shell script delte
    #os.system('rm -rf '+shell_path) 

if __name__ == "__main__":
    spare_disk = "/dev/sdb"
    default_size=900
    count =5

    script_init(spare_disk,default_size, count)
    time.sleep(1)
    while True:
        os.system('clear')
        d = Get_Data()
      
        for limit in d[1]['result']:
            for use in d[0]['result']:
                if use['metric']['pod'] == limit['metric']['pod']:   
                    use_val = float(use['value'][1])
                    limit_val = float(limit['value'][1])
                    con_name = use['metric']['name']
                    change_byte(use_val, limit_val, con_name)
                    check_disk_usage(con_name, use_val, limit_val) 
        time.sleep(1)
