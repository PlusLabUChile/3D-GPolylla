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
from kernel_certificate import certify_absence, exact_tetrahedron_orientation
from gen_mesh import sha
try:
    from evaluate_strict import quality as strict_quality
except ModuleNotFoundError:
    from evaluate import quality as strict_quality


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


AUDIT_VERSION='dual-box-v2-analytic-tetrahedra'


def audit_one(path):
    path=Path(path);run=json.loads(path.read_text());folder=path.parent
    checkpoint=folder/'kernel-audit.json'
    if checkpoint.exists():
        previous=json.loads(checkpoint.read_text())
        if previous.get('audit_version')==AUDIT_VERSION:return previous
        (folder/'kernel-audit.previous.json').write_text(json.dumps(previous,indent=2)+'\n')
    q=run.get('quality')
    report={'mesh':run['mesh']['name'],'variant':run['variant'],'status':'not_applicable' if not q else 'complete',
            'checked_absent_cells':0,'certified_absent_cells':0,'downgraded_cells':[],'certificates':[],
            'analytic_cells':[],'analytically_resolved_absent_cells':0,'merged_indeterminate_cells':[],
            'audit_version':AUDIT_VERSION,'primary_result_sha256':sha(path)}
    if q:
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
        mesh_cache=None
        def geometry():
            nonlocal mesh_cache
            if mesh_cache is None:
                vertices,tetras,volumes=load_mesh(run['mesh']['node'],run['mesh']['ele'])
                owner=np.loadtxt(folder/'quality_0.membership',dtype=int,ndmin=1)
                order=np.argsort(owner,kind='stable');split=np.flatnonzero(np.diff(owner[order]))+1
                groups={int(owner[ids[0]]):ids for ids in np.split(order,split)}
                mesh_cache=(vertices,tetras,volumes,groups)
            return mesh_cache
        changed=False
        for row in details:
            cell=int(row['cell']);old_state=row['kernel_state'];is_tet=int(row['tetrahedra'])==1
            fraction=row.get('kernel_fraction')
            analytic=is_tet and (old_state!='present' or fraction is None or abs(fraction-1)>1e-7
                      or row.get('faces_merged')!=4 or row.get('rho3_merged')!=1)
            if analytic:
                vertices,tetras,volumes,groups=geometry();ids=groups[cell]
                if exact_tetrahedron_orientation(vertices[tetras[ids[0]]])==0:
                    row['kernel_state']='error';row['error']='Degenerate tetrahedron confirmed by exact orientation'
                    for field in ('kernel_volume','kernel_fraction','volume_ratio_legacy','rho1_triangles','rho1_merged'):row[field]=None
                    report.setdefault('exact_degenerate_cells',[]).append(cell);changed=True
                    if old_state=='absent_confirmed':
                        report['checked_absent_cells']+=1;report['downgraded_cells'].append(cell)
                    continue
                original_error=row.get('error')
                if row.get('volume') is None or row.get('rho2_triangles') is None:
                    vertices,tetras,volumes,groups=geometry();ids=groups[cell]
                    replacement=strict_quality(vertices,tetras[ids],volumes[ids]);row.update(replacement)
                row.update(kernel_state='present',kernel_volume=row['volume'],kernel_fraction=1.,volume_ratio_legacy=1.,
                           surface_ratio=1.,rho1_triangles=1.,rho1_merged=1.,faces_merged=4,edges_merged=6,
                           edge_ratio_merged=row['edge_ratio_triangles'],rho2_merged=row['rho2_triangles'],rho3_merged=1.,
                           min_face_to_cell_diameter_merged=row['min_face_to_cell_diameter'],
                           min_edge_to_face_diameter_merged=row['min_edge_to_face_diameter'])
                if 'error' in row:row['error']=None
                report['analytic_cells'].append({'cell':cell,'previous_state':old_state,'previous_error':original_error,
                                                 'proof':'nondegenerate tetrahedron: kernel and hull equal the cell'})
                if old_state=='absent_confirmed':
                    report['checked_absent_cells']+=1;report['analytically_resolved_absent_cells']+=1
                changed=True
            elif old_state=='absent_confirmed':
                vertices,tetras,volumes,groups=geometry()
                normals,offsets=planes(vertices,tetras[groups[cell]])
                certificate=certify_absence(normals,offsets);certificate['cell']=cell
                report['certificates'].append(certificate);report['checked_absent_cells']+=1
                if certificate['certified']:report['certified_absent_cells']+=1
                else:
                    report['downgraded_cells'].append(cell);row['kernel_state']='indeterminate'
                    for field in ('kernel_volume','kernel_fraction','volume_ratio_legacy','rho1_triangles','rho1_merged'):row[field]=None
                    changed=True
            if (row.get('faces_merged') is not None and row['faces_merged']<4) or (row.get('rho3_merged') is not None and row['rho3_merged']>1+1e-7):
                for field in list(row):
                    if field.endswith('_merged'):row[field]=None
                report['merged_indeterminate_cells'].append(cell);changed=True
        report['unresolved_metric_errors']=[{'cell':int(row['cell']),'cause':row.get('error') or 'Primary kernel construction or volume validation did not provide a usable value'}
                    for row in details if row['kernel_state']=='error']
        if changed:
            audited=corrected_summary(q,details)
            resolved={row['cell'] for row in report['analytic_cells']}
            audited['primary_failures']=q.get('failures',[])
            audited['failures']=[failure for failure in q.get('failures',[]) if failure['cell'] not in resolved]
            report['corrected_quality']=audited
            (folder/'quality_audited.quality.json').write_text(json.dumps(audited,indent=2)+'\n')
            fields=list(dict.fromkeys([*raw[0],*(key for row in details for key in row)]))
            with (folder/'quality_audited.details.csv').open('w') as f:
                writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader();writer.writerows(details)
    checkpoint.write_text(json.dumps(report,indent=2)+'\n')
    return report


def audit(root,workers=4):
    root=Path(root);paths=sorted((root/'runs').glob('*/*/result.json'));rows=[]
    with ProcessPoolExecutor(max_workers=workers) as pool:
        for row in pool.map(audit_one,paths):
            rows.append({k:v for k,v in row.items() if k not in ('certificates','corrected_quality')})
            print(f'AUDIT {len(rows)}/{len(paths)} {row["mesh"]} {row["variant"]}: {len(row["downgraded_cells"])} downgraded',flush=True)
    report={'audit_version':AUDIT_VERSION,'runs':len(rows),'checked_absent_cells':sum(r['checked_absent_cells'] for r in rows),
            'certified_absent_cells':sum(r['certified_absent_cells'] for r in rows),
            'analytically_resolved_cells':sum(len(r['analytic_cells']) for r in rows),
            'analytically_resolved_absent_cells':sum(r['analytically_resolved_absent_cells'] for r in rows),
            'downgraded_cells':sum(len(r['downgraded_cells']) for r in rows),'rows':rows}
    (root/'kernel-audit.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',required=True);p.add_argument('--workers',type=int,default=4)
    a=p.parse_args();r=audit(a.root,a.workers);print(json.dumps({k:v for k,v in r.items() if k!='rows'}))
