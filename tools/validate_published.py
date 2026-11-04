#!/usr/bin/env python3
"""Validate evaluator on coarsest meshes in the published P035 dataset."""
import argparse
from collections import Counter,defaultdict
import json
from pathlib import Path
import subprocess
import zipfile
import numpy as np
from evaluate import evaluate,load_mesh
from gen_mesh import sha


def data_rows(text):
    for line in text.splitlines():
        line=line.split('#',1)[0].strip()
        if line:yield line.split()


def regn(text):
    rows=data_rows(text);count=int(next(rows)[0]);regions=[]
    for _ in range(count):
        header=next(rows);faces=[]
        for _ in range(int(header[1])):
            row=next(rows);n=int(row[1]);faces.append(tuple(map(int,row[2:2+n])))
        regions.append(faces)
    return regions


def validate(root,exe):
    root=Path(root);tet=zipfile.ZipFile(root/'tet-poisson1.zip');poly=zipfile.ZipFile(root/'poly-poisson1.zip')
    counts=[]
    for i in range(4):
        a=regn(tet.read(f'tet-poisson1/{i}.ele').decode());b=regn(poly.read(f'poly-poisson1/{i}.ele').decode())
        counts.append({'level':i,'tetrahedra':len(a),'polyhedra':len(b),'reduction':1-len(b)/len(a)})
    tetra_regions=regn(tet.read('tet-poisson1/0.ele').decode());poly_regions=regn(poly.read('poly-poisson1/0.ele').decode())
    tetra_vertices=[tuple(sorted({v for face in region for v in face})) for region in tetra_regions]
    if not all(len(v)==4 for v in tetra_vertices):raise ValueError('Dataset tetrahedral mesh contains non-tetrahedral cells')
    folder=root/'converted';folder.mkdir(exist_ok=True)
    node=folder/'reference.node';node.write_bytes(tet.read('tet-poisson1/0.node'))
    ele=folder/'reference.ele'
    with ele.open('w') as f:
        f.write(f'{len(tetra_vertices)} 4 0\n')
        for i,vertices in enumerate(tetra_vertices):f.write(f'{i} '+' '.join(map(str,vertices))+'\n')
    face_tetras=defaultdict(list)
    for ti,faces in enumerate(tetra_regions):
        for face in faces:face_tetras[tuple(sorted(face))].append(ti)
    owner=np.full(len(tetra_vertices),-1,dtype=int)
    for pi,region in enumerate(poly_regions):
        facets=[set(face) for face in region];vertices=set().union(*facets)
        boundary={face for face in face_tetras if any(set(face)<=facet for facet in facets)}
        candidates={ti for ti,v in enumerate(tetra_vertices) if set(v)<=vertices}
        seen=set();matching=[]
        for seed in sorted(candidates):
            if seed in seen:continue
            group=[];pending=[seed];seen.add(seed)
            while pending:
                ti=pending.pop();group.append(ti)
                for face in tetra_regions[ti]:
                    key=tuple(sorted(face))
                    if key in boundary:continue
                    for nxt in face_tetras[key]:
                        if nxt in candidates and nxt not in seen:seen.add(nxt);pending.append(nxt)
            counts_boundary=Counter(tuple(sorted(face)) for ti in group for face in tetra_regions[ti])
            if {face for face,count in counts_boundary.items() if count==1}==boundary:matching.append(group)
        if len(matching)!=1:raise ValueError(f'Ambiguous source polyhedron {pi}: {len(matching)} matches')
        for ti in matching[0]:
            if owner[ti]>=0:raise ValueError('Published cell overlap')
            owner[ti]=pi
    if np.any(owner<0):raise ValueError('Published cells leave uncovered tetrahedra')
    membership=folder/'published.membership';np.savetxt(membership,owner,fmt='%d')
    subprocess.run([exe,str(node),str(ele),'membership',str(folder/'published'),str(membership)],check=True,capture_output=True)
    identity=folder/'identity.membership';np.savetxt(identity,np.arange(len(owner)),fmt='%d')
    tq=evaluate(node,ele,identity,folder/'tetrahedral');pq=evaluate(node,ele,membership,folder/'polyhedral')
    _,_,volumes=load_mesh(node,ele)
    report={'dataset_manifest':json.loads((root/'manifest.json').read_text()),'source_counts':counts,
            'converted_node_sha256':sha(node),'converted_ele_sha256':sha(ele),'source_partition_sha256':sha(membership),
            'tetrahedral_quality':tq,'published_polyhedral_quality':pq,'total_volume':float(volumes.sum()),
            'expected':{'tetrahedra':175,'polyhedra':140,'tetrahedron_kernel_fraction':1,'pair_kernel_fraction':1}}
    report['pass']=bool(abs(volumes.sum()-1)<1e-9 and tq['cells']==175 and pq['cells']==140
                        and tq['kernel_states']['present']==175 and pq['kernel_states']['present']==140
                        and tq['metrics']['rho3_triangles']['min']==1 and pq['metrics']['tetrahedra']['max']==2)
    (root/'validation.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',required=True);p.add_argument('--exe',required=True)
    a=p.parse_args();r=validate(a.root,a.exe);print(json.dumps({'pass':r['pass'],'counts':r['source_counts'],'volume':r['total_volume']}))
