import sys
import subprocess
import os
# run this ./client/mutated_memcache 192.168.1.11:11211 190000 -w 5 -W 10000
SERVER = '192.168.1.11:11211'

CLIENTS = int(sys.argv[1])
TOTAL_RPS = int(sys.argv[2])
# Add any other arugment to enable dumping

# The absolute path to the directory this file is in
DIR = os.path.dirname(os.path.abspath(__file__))

# Remove any files with the format dump_*.txt
for f in os.listdir(DIR):
    if f.startswith('dump_') and f.endswith('.txt'):
        os.remove(os.path.join(DIR, f))

print('Starting', CLIENTS, 'clients with', TOTAL_RPS, 'total rps')
clients = []
for i in range(CLIENTS):
    cmd = list(map(str, [os.path.join(DIR, 'client/mutated_memcache'), SERVER, TOTAL_RPS / CLIENTS, '-w', '5', '-W', '10000', '-x', i]))
    
    if len(sys.argv) > 3:
        cmd.append('-r')
        with(open(os.path.join(DIR, f'dump_{i}.txt'), 'w')) as f:
            p = subprocess.Popen(cmd, stdout=f)
    else:
        p = subprocess.Popen(cmd)
    clients.append(p)
    
for p in clients:
    p.wait()
    
 