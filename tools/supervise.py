#!/usr/bin/env python3
"""Continue quality, real, timing and analysis stages after the first runner exits."""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess
import sys
import time


def supervise(a):
    root=Path(a.root).resolve();logs=Path(a.logs).resolve();logs.mkdir(parents=True,exist_ok=True)
    state=root/'supervisor.json';marker=logs/'RUNNING'
    marker.write_text('Active experiment campaign\n')
    def record(stage,status,**kwargs):
        state.write_text(json.dumps({'stage':stage,'status':status,'updated_utc':datetime.now(timezone.utc).isoformat(),**kwargs},indent=2)+'\n')
    try:
        record('synthetic','waiting_for_running_process',pid=a.wait_pid)
        while True:
            try:os.kill(a.wait_pid,0)
            except ProcessLookupError:break
            time.sleep(15)
        inputs=json.loads((root/'synthetic-inputs.json').read_text())
        expected=7*len(inputs)+3*sum(mesh['n'] in (1000,10000) for mesh in inputs)
        results=json.loads((root/'synthetic-results.json').read_text())
        if len(results)!=expected:raise RuntimeError(f'Incomplete synthetic stage: {len(results)}/{expected}')
        stages=[('real',[sys.executable,root/'frozen/tools/campaign.py','--root',root,'--source',a.source,
                         '--exe',root/'frozen/GPolyllaExperiment','--tetgen',root/'frozen/tetgen','--stage','real','--workers','4']),
                ('audit',[sys.executable,root/'frozen/tools/audit_kernels.py','--root',root,'--workers','4']),
                ('timing',[sys.executable,root/'frozen/tools/timing.py','--root',root,'--exe',root/'frozen/GPolyllaExperiment',
                           '--tetgen',root/'frozen/tetgen','--cooldown','120','--max-n','4000000']),
                ('analysis',[sys.executable,root/'frozen/tools/analyze.py','--root',root,'--out',a.out])]
        for stage,command in stages:
            record(stage,'running')
            with (logs/f'campaign-{stage}.log').open('a') as stream:
                process=subprocess.run(list(map(str,command)),stdout=stream,stderr=subprocess.STDOUT,
                    env={**os.environ,'OPENBLAS_NUM_THREADS':'1','OMP_NUM_THREADS':'1','MKL_NUM_THREADS':'1'})
            if process.returncode:raise RuntimeError(f'{stage} exited {process.returncode}; inspect logs')
        record('review','ready_for_agent_review')
    except Exception as e:
        record('interrupted','needs_agent_attention',cause=f'{type(e).__name__}: {e}')
        raise
    finally:
        marker.unlink(missing_ok=True)


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for name in ('root','source','logs','out'):p.add_argument('--'+name,required=True)
    p.add_argument('--wait-pid',type=int,required=True)
    supervise(p.parse_args())
