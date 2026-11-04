#!/usr/bin/env python3
import argparse
import json
import subprocess
import sys
import time
from pathlib import Path
import numpy as np
from gen_mesh import generate
from evaluate import evaluate


def canonical(path):
    owner = np.loadtxt(path, dtype=int, ndmin=1)
    ids = {}; result = []
    for value in owner:
        if int(value) not in ids: ids[int(value)] = len(ids)
        result.append(ids[int(value)])
    return result


def pilot(root, exe, tetgen):
    root = Path(root).resolve(); root.mkdir(parents=True, exist_ok=True)
    source = Path(__file__).resolve().parents[1]
    reference = source.parent / '3D-Polylla'
    report = []
    inputs = [(name, source/'data'/f'{name}.node', source/'data'/f'{name}.ele') for name in
              ('basic','minimal','3D_100','socket','1000points','1000points07','mage','angel')]
    for family in ('random','poisson','grid','quality'):
        folder = root/'meshes'/f'{family}_500_s1'
        info = generate(family,500,1,folder,tetgen)
        inputs.append((f'{family}_500_s1', folder/'input.1.node',folder/'input.1.ele'))
        print(f'generated {family}: {info["vertices"]} vertices', flush=True)
    for name, node, ele in inputs:
        out = root/'runs'/name; out.mkdir(parents=True,exist_ok=True)
        variants = ('cavity-ascending','face-area','face-incircle','pairs')
        for variant in variants:
            prefix = out/variant
            command = [str(exe), str(node),str(ele),variant,str(prefix)]
            start=time.monotonic()
            result=subprocess.run(command,capture_output=True,text=True,timeout=1800)
            row={'mesh':name,'variant':variant,'returncode':result.returncode,'wall_seconds':time.monotonic()-start}
            (out/f'{variant}.log').write_text(result.stdout+result.stderr)
            if result.returncode==0:
                # E0 pilot evaluates all cells of the four synthetic families
                # and small analytical legacy meshes before the campaign.
                if name.endswith('_s1') or name in ('basic','minimal'):
                    q=evaluate(node,ele,prefix.with_suffix('.membership'),prefix)
                    row['kernel_states']=q['kernel_states']
                if variant.startswith('face'):
                    oracle=out/f'{variant}.oracle.membership'
                    cmd=[sys.executable,str(source/'tools'/'run_baseline.py'),'--node',str(node),'--ele',str(ele),
                         '--variant',variant,'--out',str(oracle),'--reference',str(reference),'--cache',str(root/'oracle')]
                    oracle_result=subprocess.run(cmd,capture_output=True,text=True,timeout=1800)
                    (out/f'{variant}.oracle.log').write_text(oracle_result.stdout+oracle_result.stderr)
                    row['oracle_returncode']=oracle_result.returncode
                    row['oracle_equal']=oracle_result.returncode==0 and canonical(oracle)==canonical(prefix.with_suffix('.membership'))
            report.append(row)
            (root/'pilot.json').write_text(json.dumps(report,indent=2)+'\n')
            print(json.dumps(row),flush=True)
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--root',required=True);p.add_argument('--exe',required=True);p.add_argument('--tetgen',required=True)
    a=p.parse_args();pilot(a.root,a.exe,a.tetgen)
