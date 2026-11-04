#!/usr/bin/env python3
"""Reproduce a seeded stratified sample from Sorgente's published Thingi subset.

Route B from the plan: closed, genus-zero, nondegenerate surfaces, TetGen -d
before PLC tetrahedralization. Every attempted model receives a status record.
"""
import argparse
from concurrent.futures import ProcessPoolExecutor
import csv
import hashlib
import io
import json
from pathlib import Path
import subprocess
import resource
import zipfile
import numpy as np
import trimesh
from gen_mesh import sha


def mesh_limit():
    resource.setrlimit(resource.RLIMIT_AS, (2*1024**3, 2*1024**3))


def prepare_one(job):
    name, contents, metadata, root, tetgen = job
    folder = Path(root)/name
    folder.mkdir(parents=True, exist_ok=True)
    record = {'id': name, 'license': metadata.get('License'), 'thing_id': metadata.get('Thing ID'),
              'metadata_link': metadata.get('Link'), 'source_off_sha256': hashlib.sha256(contents).hexdigest()}
    checkpoint = folder/'preprocess.json'
    if checkpoint.exists():
        return json.loads(checkpoint.read_text())
    try:
        if not record['license'] or record['license'] in ('None','Unknown',''):
            raise ValueError('Model license unavailable')
        if len(contents) >= 1000000:
            raise ValueError('Surface is >= 1 MB')
        surface = trimesh.load(io.BytesIO(contents), file_type='off', process=False)
        if not surface.is_watertight or surface.euler_number != 2 or len(surface.split(only_watertight=False)) != 1:
            raise ValueError('Surface fails closed/single-component/genus-zero filter')
        if not np.isfinite(surface.vertices).all() or np.any(surface.area_faces <= 0):
            raise ValueError('Degenerate or non-finite surface')
        v = np.asarray(surface.vertices)
        v = (v-v.min(axis=0)) / np.linalg.norm(np.ptp(v, axis=0))
        faces = np.asarray(surface.faces)
        surface = trimesh.Trimesh(v, faces, process=False)
        volume = abs(float(surface.volume))
        if volume <= 0:
            raise ValueError('Zero surface volume')
        poly = folder/'surface.poly'
        with poly.open('w') as f:
            f.write(f'{len(v)} 3 0 0\n')
            for i, point in enumerate(v):
                f.write(f'{i} '+' '.join(format(x,'.17g') for x in point)+'\n')
            f.write(f'{len(faces)} 0\n')
            for face in faces:
                f.write('1 0\n3 '+' '.join(map(str, face))+'\n')
            f.write('0\n0\n')
        check = subprocess.run([str(tetgen),'-dz',str(poly)],capture_output=True,text=True,timeout=600, preexec_fn=mesh_limit)
        (folder/'intersection.log').write_text(check.stdout+check.stderr)
        if check.returncode != 0 or not any(message in check.stdout for message in ('No faces are intersecting', 'The input surface mesh is correct.')):
            raise ValueError('TetGen intersection check failed; inspect intersection.log')
        max_volume = volume/12000
        command = [str(tetgen),f'-pq1.414a{max_volume:.17g}S150000fennzQ',str(poly)]
        result = subprocess.run(command,capture_output=True,text=True,timeout=600, preexec_fn=mesh_limit)
        (folder/'tetgen.log').write_text(result.stdout+result.stderr)
        if result.returncode != 0:
            raise ValueError(f'TetGen returned {result.returncode}')
        node, ele = folder/'surface.1.node', folder/'surface.1.ele'
        record.update(vertices=int(node.read_text().split()[0]),tetrahedra=int(ele.read_text().split()[0]),
                      node_sha256=sha(node),ele_sha256=sha(ele),max_volume=max_volume,command=command)
        if not 10000 <= record['tetrahedra'] <= 1000000:
            raise ValueError('Tetrahedral count outside preregistered [10000,1000000]')
        record['status']='accepted'
    except Exception as e:
        record.update(status='excluded',cause=f'{type(e).__name__}: {e}')
    checkpoint.write_text(json.dumps(record,indent=2)+'\n')
    return record


def prepare(archive, metadata, root, tetgen, workers=4, seed=1):
    root = Path(root).resolve();root.mkdir(parents=True,exist_ok=True)
    with Path(metadata).open(newline='') as f:
        lookup={row['ID']:row for row in csv.DictReader(f)}
    jobs=[]
    with zipfile.ZipFile(archive) as z:
        for item in sorted(z.infolist(),key=lambda x:x.filename):
            if not item.filename.endswith('.off'):
                continue
            name=Path(item.filename).stem
            jobs.append((name,z.read(item),lookup.get(name,{}),str(root/'pool'),tetgen))
    # Candidate pool is sampled before meshing and before inspecting quality.
    # Stratification uses actual tetrahedral counts of accepted candidates.
    rng=np.random.default_rng(seed)
    indices=sorted(rng.choice(len(jobs), min(180,len(jobs)), replace=False))
    selected_jobs=[jobs[i] for i in indices]
    reference_jobs=[job for job in jobs if job[0] in ('60246','85580') and job not in selected_jobs]
    jobs=selected_jobs+reference_jobs
    records=[]
    with ProcessPoolExecutor(max_workers=workers) as pool:
        for record in pool.map(prepare_one,jobs):
            records.append(record)
            with (root/'preprocess.jsonl').open('a') as f:f.write(json.dumps(record)+'\n')
            print(f'{len(records)}/{len(jobs)} {record["id"]}: {record["status"]}',flush=True)
    accepted=sorted([r for r in records if r['status']=='accepted'],key=lambda r:(r['tetrahedra'],int(r['id'])))
    if len(accepted)<60:
        raise RuntimeError(f'Only {len(accepted)} accepted models; cannot form 3 strata of 20')
    rng=np.random.default_rng(seed)
    selected=[]
    for stratum, indices in enumerate(np.array_split(np.arange(len(accepted)),3),1):
        for i in sorted(rng.choice(indices,20,replace=False)):
            selected.append({**accepted[i],'stratum':stratum})
    for name in ('60246','85580'):
        reference=next((r for r in records if r['id']==name),None)
        if reference and reference['status']=='accepted' and name not in {r['id'] for r in selected}:
            selected.append({**reference,'stratum':'reference'})
    manifest={'archive_sha256':sha(archive),'metadata_sha256':sha(metadata),'tetgen_sha256':sha(tetgen),
              'seed':seed,'candidate_pool_seed':seed,'candidate_pool_rule':'180 sampled without replacement from published 1806 before preprocessing; plus references','preprocess_timeout_seconds':600,'preprocess_address_space_bytes':2*1024**3,'pool_count':len(records),'accepted_count':len(accepted),'selected':selected,'records':records,
              'source':'https://github.com/TommasoSorgente/polyhedron_kernel/tree/33a1ce1780cb58a9ddec9eac4325ef0c6f6a896c/datasets',
              'metadata_source':'https://huggingface.co/datasets/Thingi10K/Thingi10K/blob/main/metadata/input_summary.csv'}
    (root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return manifest


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for name in ('archive','metadata','root','tetgen'):p.add_argument('--'+name,required=True)
    p.add_argument('--workers',type=int,default=4)
    a=p.parse_args();prepare(a.archive,a.metadata,a.root,a.tetgen,a.workers)
