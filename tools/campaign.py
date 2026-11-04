#!/usr/bin/env python3
"""Resumable experiment runner with raw logs, independent verification and limits."""
import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
import csv
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import resource
import shutil
import signal
import subprocess
import sys
import time
from gen_mesh import sha
from pilot import canonical


ENV={**os.environ,'OPENBLAS_NUM_THREADS':'1','OMP_NUM_THREADS':'1','MKL_NUM_THREADS':'1'}


def bounded(command, log, timeout=1800, memory=3*1024**3):
    def limit():
        resource.setrlimit(resource.RLIMIT_AS,(memory,memory))
    start=time.monotonic()
    with Path(log).open('w') as stream:
        process=subprocess.Popen(list(map(str,command)),stdout=stream,stderr=subprocess.STDOUT,
                                 env=ENV,start_new_session=True,preexec_fn=limit)
        try:
            code=process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid,signal.SIGKILL);process.wait()
            raise TimeoutError(f'Execution exceeded {timeout} seconds; process group terminated')
    if code:
        raise RuntimeError(f'Exit {code}; see {log}')
    return time.monotonic()-start


def run_job(job):
    mesh,variant,root,exe,source,reference=job
    folder=Path(root)/'runs'/mesh['name']/variant
    folder.mkdir(parents=True,exist_ok=True)
    checkpoint=folder/'result.json'
    signature={'node':sha(mesh['node']),'ele':sha(mesh['ele']),'binary':sha(exe),'variant':variant}
    if checkpoint.exists():
        previous=json.loads(checkpoint.read_text())
        if previous.get('signature')!=signature:
            raise RuntimeError(f'Incompatible checkpoint {folder}')
        return previous
    row={'mesh':mesh,'variant':variant,'signature':signature,'started_utc':datetime.now(timezone.utc).isoformat()}
    started=time.monotonic()
    try:
        memberships=[]
        for repetition in range(2):
            prefix=folder/f'quality_{repetition}'
            membership=prefix.with_suffix('.membership')
            if variant=='edge' or 'legacy' in variant:
                command=[sys.executable,Path(__file__).with_name('run_baseline.py'),'--node',mesh['node'],'--ele',mesh['ele'],
                         '--variant',variant,'--out',membership,'--reference',reference,'--cache',folder/'oracle']
                bounded(['/usr/bin/time','-f','%M','-o',folder/f'quality_{repetition}.rss_kib',*command],folder/f'algorithm_{repetition}.log')
                # Native validator checks coverage, connectivity and boundary reconstruction.
                bounded([exe,mesh['node'],mesh['ele'],'membership',folder/f'validated_{repetition}',membership],folder/f'validation_{repetition}.log')
            else:
                command=[exe,mesh['node'],mesh['ele'],variant,prefix,str(mesh.get('seed',1))]
                bounded(['/usr/bin/time','-f','%M','-o',folder/f'quality_{repetition}.rss_kib',*command],folder/f'algorithm_{repetition}.log')
            memberships.append(membership)
        if canonical(memberships[0])!=canonical(memberships[1]):
            raise ValueError('Independent verification partition differs')
        row['membership_sha256']=sha(memberships[0])
        prefix=folder/'quality_0'
        bounded([sys.executable,Path(__file__).with_name('evaluate.py'),'--node',mesh['node'],'--ele',mesh['ele'],
                 '--membership',memberships[0],'--out',prefix],folder/'evaluation.log',timeout=max(1,int(1800-(time.monotonic()-started))))
        row['quality']=json.loads(prefix.with_suffix('.quality.json').read_text())
        row['rss_kib']=max(int((folder/f'quality_{r}.rss_kib').read_text().strip().splitlines()[-1]) for r in range(2))
        row['status']='complete' if not row['quality']['kernel_states']['error'] else 'complete_with_metric_errors'
    except Exception as e:
        row.update(status='failed',cause=f'{type(e).__name__}: {e}')
    row['wall_seconds']=time.monotonic()-started
    checkpoint.write_text(json.dumps(row,indent=2)+'\n')
    return row


def synthetic(root,tetgen):
    root=Path(root)
    meshes=[]
    for family in ('random','poisson','grid','quality'):
        for n in (500,1000,5000,10000,50000):
            for seed in (range(1,6) if family in ('random','poisson') else (1,)):
                name=f'{family}_{n}_s{seed}';folder=root/'meshes'/name
                if not (folder/'manifest.json').exists():
                    folder.mkdir(parents=True,exist_ok=True)
                    bounded([sys.executable,Path(__file__).with_name('gen_mesh.py'),'--family',family,'--n',n,'--seed',seed,
                             '--out',folder,'--tetgen',tetgen],folder/'generation.log')
                info=json.loads((folder/'manifest.json').read_text())
                for file,digest in info['hashes'].items():
                    if sha(folder/file)!=digest:raise ValueError(f'Mesh checksum mismatch {folder/file}')
                meshes.append({'name':name,'family':family,'n':n,'seed':seed,'node':str(folder/'input.1.node'),
                               'ele':str(folder/'input.1.ele'),'vertices':info['vertices'],'tetrahedra':info['tetrahedra']})
                print(f'INPUT {name}: {info["vertices"]} vertices, {info["tetrahedra"]} tetrahedra',flush=True)
    (root/'synthetic-inputs.json').write_text(json.dumps(meshes,indent=2)+'\n')
    return meshes


def execute_jobs(jobs,root,stage,workers):
    root=Path(root);rows=[]
    checkpoint=root/f'{stage}.json'
    with ProcessPoolExecutor(max_workers=workers) as pool:
        futures=[pool.submit(run_job,job) for job in jobs]
        for future in as_completed(futures):
            row=future.result();rows.append(row)
            checkpoint.write_text(json.dumps(rows,indent=2)+'\n')
            print(f'{stage} {len(rows)}/{len(jobs)} {row["mesh"]["name"]} {row["variant"]}: {row["status"]}',flush=True)
    return rows


def main(a):
    root=Path(a.root).resolve();root.mkdir(parents=True,exist_ok=True)
    source=Path(a.source).resolve();exe=str(Path(a.exe).resolve());tetgen=str(Path(a.tetgen).resolve())
    reference=str(source.parent/'3D-Polylla')
    manifest_path=root/'manifest.json'
    if not manifest_path.exists():
        manifest={'started_utc':datetime.now(timezone.utc).isoformat(),'platform':platform.platform(),
                  'python':sys.version,'binary_sha256':sha(exe),'tetgen_sha256':sha(tetgen),
                  'source_commit':subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip(),
                  'source_diff_sha256':hashlib.sha256(subprocess.check_output(['git','-C',str(source),'diff','HEAD'])).hexdigest(),
                  'cpuinfo':Path('/proc/cpuinfo').read_text(),'meminfo':Path('/proc/meminfo').read_text(),
                  'quality_workers':a.workers,'quality_timeout_seconds':1800,'quality_address_space_bytes':3*1024**3,
                  'timing_scope':'native read, algorithm including output reconstruction, validation, membership write; separate flat construction',
                  'deterministic_families':'Grid and Quality use one unique mesh per size. Repeated deterministic seed aliases are not independent samples.',
                  'tie_rule_face':'rounded score/max_score*1e12, then ascending canonical face ID',
                  'incircle_rule':'geometric inradius; legacy squared-side oracle recorded separately',
                  'scripts':{p.name:sha(p) for p in Path(__file__).parent.glob('*.py')}}
        manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
    else:
        manifest=json.loads(manifest_path.read_text())
        if manifest['binary_sha256']!=sha(exe) or manifest['tetgen_sha256']!=sha(tetgen):
            raise ValueError('Campaign binaries changed')
    meshes=synthetic(root,tetgen)
    variants=('cavity-ascending','face-area','face-incircle','face-area-repair','face-incircle-repair','edge','pairs')
    if a.stage in ('synthetic','all'):
        # E1 shares ascending results with E2; different orders have own files.
        jobs=[]
        for mesh in meshes:
            for variant in variants:jobs.append((mesh,variant,str(root),exe,str(source),reference))
            if mesh['n'] in (1000,10000):
                for order in ('descending','index','random'):jobs.append((mesh,'cavity-'+order,str(root),exe,str(source),reference))
        execute_jobs(jobs,root,'synthetic-results',a.workers)
    if a.stage in ('real','all'):
        real_manifest=root/'real'/'manifest.json'
        if not real_manifest.exists():raise RuntimeError('Real preprocessing manifest not available yet')
        real=json.loads(real_manifest.read_text())
        jobs=[]
        for model in real['selected']:
            mesh={'name':'thingi_'+model['id'],'family':'real','n':model['vertices'],'seed':1,'stratum':model['stratum'],
                  'node':str(root/'real'/'pool'/model['id']/'surface.1.node'),
                  'ele':str(root/'real'/'pool'/model['id']/'surface.1.ele')}
            for variant in variants:jobs.append((mesh,variant,str(root),exe,str(source),reference))
        execute_jobs(jobs,root,'real-results',a.workers)


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for name in ('root','source','exe','tetgen'):p.add_argument('--'+name,required=True)
    p.add_argument('--stage',choices=('synthetic','real','all'),default='all')
    p.add_argument('--workers',type=int,default=4)
    main(p.parse_args())
