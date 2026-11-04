#!/usr/bin/env python3
"""Supplemental E5 timing of the unchanged common quality evaluator.

This wrapper changes no geometry, states or aggregation rules. It records read
and per-cell statistics separately, plus full evaluator wall time. Primary
quality runs retain their original frozen evaluator and outputs.
"""
import argparse
import json
from pathlib import Path
import time
try:
    import evaluate_strict as evaluator
except ModuleNotFoundError:
    import evaluate as evaluator


def timed(node,ele,membership,out):
    totals={'read_ms':0.,'cell_statistics_ms':0.}
    load=evaluator.load_mesh;quality=evaluator.quality
    def measured_load(*args,**kwargs):
        start=time.perf_counter()
        try:return load(*args,**kwargs)
        finally:totals['read_ms']+=(time.perf_counter()-start)*1000
    def measured_quality(*args,**kwargs):
        start=time.perf_counter()
        try:return quality(*args,**kwargs)
        finally:totals['cell_statistics_ms']+=(time.perf_counter()-start)*1000
    evaluator.load_mesh=measured_load;evaluator.quality=measured_quality
    start=time.perf_counter()
    try:report=evaluator.evaluate(node,ele,membership,out)
    finally:evaluator.load_mesh=load;evaluator.quality=quality
    totals['evaluator_wall_ms']=(time.perf_counter()-start)*1000
    totals['membership_aggregation_export_ms']=max(0.,totals['evaluator_wall_ms']-totals['read_ms']-totals['cell_statistics_ms'])
    totals['repetitions']=1
    totals['scope']='Independent Python quality process, no native VISF export; per-cell statistics accumulated separately from input reading.'
    Path(str(out)+'.statistics-timing.json').write_text(json.dumps(totals,indent=2)+'\n')
    return totals


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for name in ('node','ele','membership','out'):p.add_argument('--'+name,required=True)
    a=p.parse_args();print(json.dumps(timed(a.node,a.ele,a.membership,a.out)))
