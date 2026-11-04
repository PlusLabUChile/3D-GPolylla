#!/usr/bin/env python3
"""Validate native Face partitions against the parametrized Python oracle."""
import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
import json
from pathlib import Path
import subprocess
import sys
from gen_mesh import generate
from pilot import canonical


def compare(job):
    name,node,ele,variant,out,exe,reference,cache=job
    out=Path(out);out.mkdir(parents=True,exist_ok=True)
    prefix=out/variant;oracle=out/f'{variant}.oracle.membership'
    native=subprocess.run([exe,str(node),str(ele),variant,str(prefix)],capture_output=True,text=True,timeout=1800)
    python=subprocess.run([sys.executable,str(Path(__file__).with_name('run_baseline.py')),
               '--node',str(node),'--ele',str(ele),'--variant',variant,'--out',str(oracle),
               '--reference',str(reference),'--cache',str(cache)],capture_output=True,text=True,timeout=1800)
    (out/f'{variant}.native.log').write_text(native.stdout+native.stderr)
    (out/f'{variant}.oracle.log').write_text(python.stdout+python.stderr)
    equal=native.returncode==0 and python.returncode==0 and canonical(prefix.with_suffix('.membership'))==canonical(oracle)
    return {'mesh':name,'variant':variant,'native_returncode':native.returncode,'oracle_returncode':python.returncode,
            'equal':equal,'both_rejected':native.returncode!=0 and python.returncode!=0}


def parity(root,source,exe,tetgen,workers=4):
    root=Path(root).resolve();source=Path(source).resolve();root.mkdir(parents=True,exist_ok=True)
    inputs=[(name,source/'data'/f'{name}.node',source/'data'/f'{name}.ele') for name in
            ('basic','minimal','3D_100','socket','1000points','1000points07','mage','angel')]
    for family in ('random','poisson','grid','quality'):
        for n in (500,1000):
            for seed in (range(1,6) if family in ('random','poisson') else (1,)):
                name=f'{family}_{n}_s{seed}';folder=root/'meshes'/name
                generate(family,n,seed,folder,tetgen)
                inputs.append((name,folder/'input.1.node',folder/'input.1.ele'))
    jobs=[]
    for name,node,ele in inputs:
        for variant in ('face-area','face-incircle','face-area-repair','face-incircle-repair'):
            # Per-job copies avoid races while importing reference modules.
            jobs.append((name,node,ele,variant,root/'runs'/name,exe,source.parent/'3D-Polylla',root/'oracle'/name/variant))
    report=[]
    with ProcessPoolExecutor(max_workers=workers) as pool:
        for row in pool.map(compare,jobs):
            report.append(row);print(json.dumps(row),flush=True)
            (root/'parity.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for name in ('root','source','exe','tetgen'):p.add_argument('--'+name,required=True)
    p.add_argument('--workers',type=int,default=4)
    a=p.parse_args();r=parity(a.root,a.source,a.exe,a.tetgen,a.workers)
    sys.exit(0 if all(row['equal'] or row['both_rejected'] for row in r) else 1)
