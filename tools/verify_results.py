#!/usr/bin/env python3
"""Verify frozen binaries, input hashes, audit coverage and stored CSV/JSON data.

A failed experiment is a recorded outcome. Missing or inconsistent evidence is
an error and prevents closure.
"""
import argparse
from collections import Counter
import csv
import hashlib
import json
import math
from pathlib import Path
from pilot import canonical


def digest(path):
    with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()


def finite(value):
    if isinstance(value,float) and not math.isfinite(value):raise ValueError('Non-finite JSON number')
    if isinstance(value,dict):
        for v in value.values():finite(v)
    elif isinstance(value,list):
        for v in value:finite(v)


def verify(root):
    root=Path(root);manifest=json.loads((root/'manifest.json').read_text())
    assert digest(root/'frozen/GPolyllaExperiment')==manifest['binary_sha256']
    assert digest(root/'frozen/tetgen')==manifest['tetgen_sha256']
    synthetic=json.loads((root/'synthetic-inputs.json').read_text())
    real=json.loads((root/'real/manifest.json').read_text())
    variants=('cavity-ascending','face-area','face-incircle','face-area-repair','face-incircle-repair','edge','pairs')
    expected={(mesh['name'],variant) for mesh in synthetic for variant in variants}
    expected.update((mesh['name'],'cavity-'+order) for mesh in synthetic if mesh['n'] in (1000,10000) for order in ('descending','index','random'))
    expected.update(('thingi_'+model['id'],variant) for model in real['selected'] for variant in variants)
    actual=set();hashes={};counts=Counter();audited_counts=Counter();cells=0;metric_errors=0
    for path in sorted((root/'runs').glob('*/*/result.json')):
        run=json.loads(path.read_text());key=(run['mesh']['name'],run['variant']);assert key not in actual
        actual.add(key);counts[run['status']]+=1
        for name in ('node','ele'):
            source=run['mesh'][name]
            if source not in hashes:hashes[source]=digest(source)
            assert hashes[source]==run['signature'][name],f'Changed input {source}'
        assert run['signature']['binary']==manifest['binary_sha256']
        if run['status']=='failed':
            audited_counts['failed']+=1
            assert run.get('cause'),f'Failure without cause {path}'
            continue
        assert run['status'] in ('complete','complete_with_metric_errors')
        folder=path.parent;audit=json.loads((folder/'kernel-audit.json').read_text())
        assert audit['primary_result_sha256']==digest(path)
        assert audit['checked_absent_cells']==run['quality']['kernel_states']['absent_confirmed']
        assert audit['certified_absent_cells']+len(audit['downgraded_cells'])+audit.get('analytically_resolved_absent_cells',0)==audit['checked_absent_cells']
        q=audit.get('corrected_quality',run['quality']);finite(q)
        audited_counts['complete' if not q['kernel_states']['error'] else 'complete_with_metric_errors']+=1
        assert sum(q['kernel_states'].values())==q['cells']
        assert 0<=q['reduction']<1 and q['cells']<=q['tetrahedra']
        assert abs(q['reduction']-(1-q['cells']/q['tetrahedra']))<1e-12
        assert -1e-12<=q['kernel_fraction_lower']<=q['kernel_fraction_upper']+1e-12<=1+1e-12
        first=folder/'quality_0.membership';second=folder/'quality_1.membership'
        assert digest(first)==run['membership_sha256']
        if digest(first)!=digest(second):assert canonical(first)==canonical(second)
        timing=folder/'quality_0.timing.csv'
        if not timing.exists():timing=folder/'validated_0.timing.csv'
        with timing.open() as f:record=next(csv.DictReader(f))
        for field,key_q in [('vertices','vertices'),('tetrahedra','tetrahedra'),('cells','cells')]:assert int(record[field])==q[key_q]
        detailed=folder/('quality_audited.details.csv' if audit.get('corrected_quality') else 'quality_0.details.csv')
        states=Counter();tetrahedra=0;observations=Counter();extrema={}
        with detailed.open() as f:
            reader=csv.DictReader(f);assert '' not in reader.fieldnames and len(reader.fieldnames)==len(set(reader.fieldnames))
            for row in reader:
                assert None not in row,f'CSV field mismatch {detailed}'
                states[row['kernel_state']]+=1;tetrahedra+=int(float(row['tetrahedra']))
                for field,value in row.items():
                    if not value or field in ('kernel_state','error'):continue
                    number=float(value);assert math.isfinite(number),f'Non-finite CSV field {detailed}:{field}'
                    observations[field]+=1
                    lo,hi=extrema.get(field,(number,number));extrema[field]=(min(lo,number),max(hi,number))
        assert sum(states.values())==q['cells'] and tetrahedra==q['tetrahedra']
        for state,count in q['kernel_states'].items():assert states[state]==count
        for field,statistics in q['metrics'].items():
            assert observations[field]==statistics['valid_count']
            for observed,recorded in zip(extrema[field],(statistics['min'],statistics['max'])):
                assert math.isclose(observed,recorded,rel_tol=1e-12,abs_tol=1e-14),f'CSV/JSON extrema differ {detailed}:{field}'
        cells+=q['cells'];metric_errors+=q['kernel_states']['error']
    assert actual==expected,f'Job coverage mismatch: missing {expected-actual}, extra {actual-expected}'
    report={'pass':True,'jobs':len(actual),'primary_status_counts':dict(counts),'audited_status_counts':dict(audited_counts),'successful_cells_checked':cells,
            'metric_error_cells':metric_errors,'unique_input_files_hashed':len(hashes),
            'checks':'Binary hashes; input hashes; all planned jobs; membership reproducibility; audit source hash and coverage; native counts; CSV schema, finite numbers, kernel states, tetrahedral coverage, metric counts and extrema.'}
    (root/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',required=True);a=p.parse_args();print(json.dumps(verify(a.root)))
