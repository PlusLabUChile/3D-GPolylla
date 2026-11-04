#!/usr/bin/env python3
"""Partition invariance on independent synthetic meshes under affine units."""
import argparse
import itertools
import json
from pathlib import Path
import subprocess
from evaluate import load_mesh
from pilot import canonical


def validate(root,exe,out):
    root=Path(root);out=Path(out);out.mkdir(parents=True,exist_ok=True)
    rows=[]
    for family in ('random','poisson','grid','quality'):
        folder=root/'meshes'/f'{family}_500_s1'
        node,ele=folder/'input.1.node',folder/'input.1.ele'
        vertices,tetras,_=load_mesh(node,ele)
        for order in ('ascending','descending','index','random'):
            reference=out/f'{family}_{order}_reference'
            subprocess.run([exe,str(node),str(ele),'cavity-'+order,str(reference),'1'],check=True,capture_output=True)
            expected=canonical(reference.with_suffix('.membership'))
            for scale in (.001,1.,1000.):
                name=f'{family}_{order}_{scale}'
                scaled_node=out/f'{name}.node'
                with scaled_node.open('w') as f:
                    f.write(f'{len(vertices)} 3 0 0\n')
                    for i,p in enumerate(vertices*scale+[2,-3,4]):f.write(f'{i} '+' '.join(format(x,'.17g') for x in p)+'\n')
                prefix=out/name
                result=subprocess.run([exe,str(scaled_node),str(ele),'cavity-'+order,str(prefix),'1'],capture_output=True,text=True)
                rows.append({'family':family,'order':order,'scale':scale,'returncode':result.returncode,
                             'partition_equal':result.returncode==0 and canonical(Path(str(prefix)+'.membership'))==expected,'error':result.stderr})
    (out/'validation.json').write_text(json.dumps(rows,indent=2)+'\n')
    return rows


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for name in ('root','exe','out'):p.add_argument('--'+name,required=True)
    a=p.parse_args();rows=validate(a.root,a.exe,a.out)
    print(json.dumps({'cases':len(rows),'equal':sum(r['partition_equal'] for r in rows),'failures':[r for r in rows if not r['partition_equal']]}))
