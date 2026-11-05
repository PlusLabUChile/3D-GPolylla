#!/usr/bin/env python3
"""Sequential E5 repetitions and actual E6 flat-container construction."""
import argparse
import csv
import json
import resource
from pathlib import Path
import subprocess
import sys
import time
from campaign import bounded
from gen_mesh import sha


def one(exe,node,ele,variant,prefix,memory,flat=False):
    command=[exe,node,ele,variant,prefix,'1']+(['--flat'] if flat else [])
    wall=bounded(['/usr/bin/time','-f','%M','-o',str(prefix)+'.rss_kib',*command],str(prefix)+'.log',memory=memory)
    with Path(str(prefix)+'.timing.csv').open() as f:record=next(csv.DictReader(f))
    record.update(wall_seconds=wall,rss_kib=int(Path(str(prefix)+'.rss_kib').read_text().strip().splitlines()[-1]),
                  membership_sha256=sha(str(prefix)+'.membership'),variant=variant)
    return record


def timing(root,exe,tetgen,cooldown=120,max_n=4000000,reuse_flat=False,reuse_generation_failures=False):
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    root=Path(root).resolve()
    memory=int(next(line.split()[1] for line in Path('/proc/meminfo').read_text().splitlines() if line.startswith('MemTotal:')))*1024*3//4
    limits_file=root/'timing-limits.json'
    previous_generation_failures={}
    if reuse_generation_failures and limits_file.exists():
        previous_generation_failures={
            (row['family'],row['n']):row
            for row in json.loads(limits_file.read_text())
            if row['phase']=='generation' and row['status']=='failed'
        }
    rows=[];limits=[]
    for family in ('random','quality'):
        active=['cavity-ascending','face-area','face-incircle','face-area-repair','face-incircle-repair']
        for n in (500,1000,5000,10000,50000,100000,250000,500000,1000000,2000000,4000000,8000000,16000000):
            if n>max_n or not active:break
            name=f'{family}_{n}_s1';folder=root/'meshes'/name
            folder.mkdir(parents=True,exist_ok=True)
            if not (folder/'manifest.json').exists():
                previous_failure=previous_generation_failures.get((family,n))
                if previous_failure is not None:
                    limits.append(previous_failure)
                    limits_file.write_text(json.dumps(limits,indent=2)+'\n')
                    break
                try:
                    bounded([sys.executable,Path(__file__).with_name('gen_mesh.py'),'--family',family,'--n',n,'--seed','1',
                             '--out',folder,'--tetgen',tetgen],folder/'generation.log',memory=memory)
                except Exception as e:
                    limits.append({'family':family,'n':n,'phase':'generation','status':'failed','cause':str(e)})
                    (root/'timing-limits.json').write_text(json.dumps(limits,indent=2)+'\n')
                    break
            node,ele=folder/'input.1.node',folder/'input.1.ele'
            out=root/'timing'/name;out.mkdir(parents=True,exist_ok=True)
            measured_new=False
            for variant in active.copy():
                prefix=out/f'{variant}_warmup'
                try:
                    if not Path(str(prefix)+'.timing.csv').exists():one(exe,node,ele,variant,prefix,memory)
                except Exception as e:
                    limits.append({'family':family,'n':n,'variant':variant,'phase':'warmup','status':'failed','cause':str(e)})
                    active.remove(variant)
            # Rotate order in every repetition; all runs are independent processes.
            for repetition in range(10):
                order=active[repetition%len(active):]+active[:repetition%len(active)] if active else []
                for variant in order:
                    prefix=out/f'{variant}_r{repetition:02d}'
                    result_file=Path(str(prefix)+'.result.json')
                    if result_file.exists():
                        record=json.loads(result_file.read_text())
                        if record['status']=='failed' and variant in active:
                            active.remove(variant)
                            limits.append({**record,'phase':'measurement'})
                    else:
                        measured_new=True
                        try:
                            record=one(exe,node,ele,variant,prefix,memory)
                            record.update(family=family,n=n,repetition=repetition,status='complete')
                        except Exception as e:
                            record={'family':family,'n':n,'variant':variant,'repetition':repetition,'status':'failed','cause':str(e)}
                            if variant in active:active.remove(variant)
                            limits.append({**record,'phase':'measurement'})
                        result_file.write_text(json.dumps(record,indent=2)+'\n')
                    rows.append(record)
                    (root/'timing-results.json').write_text(json.dumps(rows,indent=2)+'\n')
            if n <= 50000:
                # One separately timed statistics process for each completed
                # core input/variant. It is not mixed with native repetitions.
                for variant in active:
                    statistics_file=out/f'{variant}_statistics.statistics-timing.json'
                    if statistics_file.exists():continue
                    try:
                        prefix=out/f'{variant}_statistics'
                        bounded(['/usr/bin/time','-f','%M','-o',str(prefix)+'.rss_kib',sys.executable,
                                 Path(__file__).with_name('evaluate_timed.py'),'--node',node,'--ele',ele,
                                 '--membership',out/f'{variant}_warmup.membership','--out',prefix],str(prefix)+'.log',memory=memory)
                    except Exception as e:
                        statistics_file.write_text(json.dumps({'status':'failed','family':family,'n':n,'variant':variant,'cause':str(e)},indent=2)+'\n')
            limits.append({'family':family,'n':n,'phase':'block','status':'complete','remaining_variants':active.copy()})
            (root/'timing-limits.json').write_text(json.dumps(limits,indent=2)+'\n')
            print(f'E5 {name}: completed; variants {active}',flush=True)
            if cooldown and measured_new:time.sleep(cooldown)
    if reuse_flat:
        return rows,limits,json.loads((root/'flat-results.json').read_text())
    # E6 measures only cell containers, with the common tetrahedral mesh excluded
    # on both sides. Memory is allocated, not merely estimated.
    flat=[]
    for family in ('random','poisson','grid','quality'):
        folder=root/'meshes'/f'{family}_50000_s1'
        if not (folder/'manifest.json').exists():continue
        out=root/'flat'/family;out.mkdir(parents=True,exist_ok=True)
        try:
            record=one(exe,folder/'input.1.node',folder/'input.1.ele','cavity-ascending',out/'flat',memory,True)
            inventory_file=out/'inventory.csv'
            bounded([root/'frozen/GPolyllaMemory',folder/'input.1.node',folder/'input.1.ele',inventory_file],out/'inventory.log',memory=memory)
            with inventory_file.open() as f:inventory=next(csv.DictReader(f))
            if int(record['nested_capacity_bytes'])!=int(inventory['output_cell_capacity_bytes']):raise ValueError('Independent per-cell capacity counters disagree')
            record['flat_cycle_ms']=float(record['flat_build_ms'])
            record['flat_time_scope']='allocate, fill and release flat auxiliary containers; one sample, not a speed claim'
            record['inventory']=inventory
            record['flat_retained_sum_bytes']=int(inventory['retained_sum_bytes'])-int(inventory['output_cell_capacity_bytes'])+int(record['flat_actual_bytes'])
            record.update(family=family,status='complete')
        except Exception as e:record={'family':family,'status':'failed','cause':str(e)}
        flat.append(record)
        (root/'flat-results.json').write_text(json.dumps(flat,indent=2)+'\n')
    return rows,limits,flat


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for name in ('root','exe','tetgen'):p.add_argument('--'+name,required=True)
    p.add_argument('--cooldown',type=int,default=120)
    p.add_argument('--max-n',type=int,default=4000000)
    p.add_argument('--reuse-flat',action='store_true',help='Keep the existing E6 results when extending E5 for the same campaign')
    p.add_argument('--reuse-generation-failures',action='store_true',
                   help='Keep prior generation limits for sizes with no valid mesh')
    a=p.parse_args()
    timing(a.root,a.exe,a.tetgen,a.cooldown,a.max_n,a.reuse_flat,a.reuse_generation_failures)
