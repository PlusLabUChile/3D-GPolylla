#!/usr/bin/env python3
"""Audit every primary confirmed-absence cell; preserve original outputs.

Any absence lacking a checked dual certificate becomes indeterminate in a
separate, explicitly versioned quality overlay consumed by analyze.py.
"""
import argparse
from collections import Counter
from concurrent.futures import ProcessPoolExecutor
import copy
import csv
import json
from pathlib import Path
import numpy as np
from evaluate import load_mesh,cell_boundary
from kernel_certificate import certify_absence
from gen_mesh import sha


def planes(vertices,tetras):
    ids=np.unique(tetras);local=vertices[ids]
    points=(local-local.mean(axis=0))/np.linalg.norm(np.ptp(local,axis=0))
    normals=[];offsets=[]
    for face,opposite in cell_boundary(np.searchsorted(ids,tetras)):
        p=points[list(face)];normal=np.cross(p[1]-p[0],p[2]-p[0])
        first=p[0]
        if normal@(points[opposite]-p[0])>0:normal=-normal;first=p[2]
        normal/=np.linalg.norm(normal)
        normals.append(normal);offsets.append(-normal@first)
    return np.asarray(normals),np.asarray(offsets)


def corrected_summary(original,details):
    q=copy.deepcopy(original);states=Counter(row['kernel_state'] for row in details)
    q['kernel_states']={state:states[state] for state in ('present','absent_confirmed','indeterminate','error')}
    q['kernel_fraction_lower']=states['present']/q['cells']
    q['kernel_fraction_upper']=(states['present']+states['indeterminate']+states['error'])/q['cells']
    fields=list(dict.fromkeys(k for row in details for k in row));q['metrics']={}
    for field in fields:
        values=[row[field] for row in details if isinstance(row.get(field),(int,float)) and np.isfinite(row[field])]
        if values:q['metrics'][field]={**dict(zip(['min','p5','median','p95','max'],map(float,np.quantile(values,[0,.05,.5,.95,1])))),'valid_count':len(values)}
    for suffix in ('triangles','merged'):
        terms=[row[f'rho1_{suffix}']*(row[f'rho2_{suffix}']+row[f'rho3_{suffix}'])/2 for row in details
               if all(row.get(f'rho{i}_{suffix}') is not None for i in (1,2,3))]
        q[f'rho_{suffix}']=float(np.sqrt(np.mean(terms))) if terms else None
        q[f'rho_{suffix}_valid_cells']=len(terms)
    return q


def audit_one(path):
    path=Path(path);run=json.loads(path.read_text());folder=path.parent
    checkpoint=folder/'kernel-audit.json'
    if checkpoint.exists():return json.loads(checkpoint.read_text())
    q=run.get('quality')
    report={'mesh':run['mesh']['name'],'variant':run['variant'],'status':'not_applicable' if not q else 'complete',
            'checked_absent_cells':0,'certified_absent_cells':0,'downgraded_cells':[],'certificates':[],
            'audit_version':'dual-box-v1','primary_result_sha256':sha(path)}
    if q and q['kernel_states']['absent_confirmed']:
        vertices,tetras,_=load_mesh(run['mesh']['node'],run['mesh']['ele'])
        membership=folder/'quality_0.membership';owner=np.loadtxt(membership,dtype=int,ndmin=1)
        order=np.argsort(owner,kind='stable');split=np.flatnonzero(np.diff(owner[order]))+1
        groups={int(owner[ids[0]]):ids for ids in np.split(order,split)}
        with (folder/'quality_0.details.csv').open() as f:raw=list(csv.DictReader(f))
        details=[]
        for row in raw:
            parsed={}
            for key,value in row.items():
                if value=='':parsed[key]=None
                else:
                    try:parsed[key]=float(value)
                    except ValueError:parsed[key]=value
            details.append(parsed)
        for row in details:
            if row['kernel_state']!='absent_confirmed':continue
            cell=int(row['cell']);normals,offsets=planes(vertices,tetras[groups[cell]])
            certificate=certify_absence(normals,offsets);certificate['cell']=cell
            report['certificates'].append(certificate);report['checked_absent_cells']+=1
            if certificate['certified']:report['certified_absent_cells']+=1
            else:
                report['downgraded_cells'].append(cell);row['kernel_state']='indeterminate'
                for field in ('kernel_volume','kernel_fraction','volume_ratio_legacy','rho1_triangles','rho1_merged'):row[field]=None
        if report['downgraded_cells']:
            audited=corrected_summary(q,details)
            report['corrected_quality']=audited
            (folder/'quality_audited.quality.json').write_text(json.dumps(audited,indent=2)+'\n')
            with (folder/'quality_audited.details.csv').open('w') as f:
                writer=csv.DictWriter(f,fieldnames=list(raw[0]));writer.writeheader();writer.writerows(details)
    checkpoint.write_text(json.dumps(report,indent=2)+'\n')
    return report


def audit(root,workers=4):
    root=Path(root);paths=sorted((root/'runs').glob('*/*/result.json'));rows=[]
    with ProcessPoolExecutor(max_workers=workers) as pool:
        for row in pool.map(audit_one,paths):
            rows.append({k:v for k,v in row.items() if k not in ('certificates','corrected_quality')})
            print(f'AUDIT {len(rows)}/{len(paths)} {row["mesh"]} {row["variant"]}: {len(row["downgraded_cells"])} downgraded',flush=True)
    report={'runs':len(rows),'checked_absent_cells':sum(r['checked_absent_cells'] for r in rows),
            'certified_absent_cells':sum(r['certified_absent_cells'] for r in rows),
            'downgraded_cells':sum(len(r['downgraded_cells']) for r in rows),'rows':rows}
    (root/'kernel-audit.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',required=True);p.add_argument('--workers',type=int,default=4)
    a=p.parse_args();r=audit(a.root,a.workers);print(json.dumps({k:v for k,v in r.items() if k!='rows'}))
