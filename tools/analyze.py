#!/usr/bin/env python3
"""Mesh-level paired inference, explicit failures and exportable figures."""
import argparse
from collections import Counter, defaultdict
import csv
import json
from pathlib import Path
import numpy as np
from scipy.stats import wilcoxon


BOOTSTRAPS=10000
SEED=20261103


def interval(values,seed=SEED):
    values=np.asarray(values,dtype=float)
    if not len(values):return None
    rng=np.random.default_rng(seed)
    samples=np.median(values[rng.integers(0,len(values),(BOOTSTRAPS,len(values)))],axis=1)
    return {'n':len(values),'median':float(np.median(values)),
            'ci95_low':float(np.quantile(samples,.025)),'ci95_high':float(np.quantile(samples,.975))}


def grouped_interval(values, labels, mode, seed=SEED):
    values=np.asarray(values,dtype=float)
    if not len(values):return None
    labels=np.asarray(labels)
    groups=[values[labels==key] for key in sorted(set(labels.tolist()))]
    rng=np.random.default_rng(seed)
    if mode=='stratified':
        samples=np.concatenate([group[rng.integers(0,len(group),(BOOTSTRAPS,len(group)))] for group in groups],axis=1)
        medians=np.median(samples,axis=1)
    else:
        chosen=rng.integers(0,len(groups),(BOOTSTRAPS,len(groups)))
        medians=np.array([np.median(np.concatenate([groups[i] for i in draw])) for draw in chosen])
    return {'n':len(values),'groups':len(groups),'method':mode,'median':float(np.median(values)),
            'ci95_low':float(np.quantile(medians,.025)),'ci95_high':float(np.quantile(medians,.975))}


def holm(pvalues):
    order=sorted(range(len(pvalues)),key=lambda i:pvalues[i])
    result=[1.]*len(pvalues);maximum=0
    for rank,index in enumerate(order):
        maximum=max(maximum,min(1,pvalues[index]*(len(order)-rank)))
        result[index]=maximum
    return result


def analyze(root,out):
    root=Path(root);out=Path(out);out.mkdir(parents=True,exist_ok=True)
    rows=[]
    for path in sorted((root/'runs').glob('*/*/result.json')):
        row=json.loads(path.read_text())
        audit_file=path.parent/'kernel-audit.json'
        if row.get('quality') and not audit_file.exists():raise RuntimeError(f'Missing kernel audit: {path.parent}')
        if audit_file.exists():
            audit=json.loads(audit_file.read_text())
            if audit.get('corrected_quality'):
                row['quality']=audit['corrected_quality']
                row['primary_status']=row['status']
                row['status']='complete' if not row['quality']['kernel_states']['error'] else 'complete_with_metric_errors'
            row['kernel_audit_downgraded']=len(audit['downgraded_cells'])
        if row.get('status')=='failed':
            cause=row.get('cause','')
            diagnostic=''
            if 'see ' in cause:
                logfile=Path(cause.rsplit('see ',1)[1].strip())
                if logfile.exists():
                    lines=[line for line in logfile.read_text(errors='replace').splitlines() if line.strip()]
                    diagnostic=lines[-1] if lines else ''
            row['failure_detail']=diagnostic or cause
            lowered=row['failure_detail'].lower()
            row['failure_category']=('timeout' if 'timeout' in lowered or 'exceeded' in lowered else
              'memory' if 'memoryerror' in lowered or 'bad_alloc' in lowered else
              'disconnected_partition' if 'disconnected' in lowered else
              'incomplete_partition' if 'uncovered' in lowered or 'incomplete partition' in lowered else
              'duplicate_partition' if 'duplicates' in lowered or 'exclusivity' in lowered else 'other')
        rows.append(row)
    index={(r['mesh']['name'],r['variant']):r for r in rows}
    complete=[r for r in rows if r['status'] in ('complete','complete_with_metric_errors')]
    # The evaluator's rho is conditional on cells with all components defined.
    # Recover whole-mesh bounds by allowing undefined cell contributions in [0,1].
    for row in complete:
        q=row['quality']
        for suffix in ('triangles','merged'):
            resolved=q.get(f'rho_{suffix}_valid_cells',0)
            conditional=q.get(f'rho_{suffix}')
            known_sum=(conditional**2*resolved) if conditional is not None else 0.
            q[f'rho_{suffix}_whole_lower']=float(np.sqrt(max(0,known_sum)/q['cells']))
            q[f'rho_{suffix}_whole_upper']=float(np.sqrt(min(q['cells'],known_sum+q['cells']-resolved)/q['cells']))
    summary=[]
    for row in rows:
        mesh=row['mesh'];q=row.get('quality',{})
        summary.append({'mesh':mesh['name'],'family':mesh['family'],'target_vertices':mesh.get('n'),
                        'seed':mesh.get('seed'),'stratum':mesh.get('stratum'),'variant':row['variant'],
                        'status':row['status'],'primary_status':row.get('primary_status',row['status']),'vertices':q.get('vertices'),'tetrahedra':q.get('tetrahedra'),
                        'cells':q.get('cells'),'reduction':q.get('reduction'),'isolated':q.get('isolated'),
                        'kernel_fraction_lower':q.get('kernel_fraction_lower'),'kernel_fraction_upper':q.get('kernel_fraction_upper'),
                        **{f'kernel_{state}':q.get('kernel_states',{}).get(state) for state in ('present','absent_confirmed','indeterminate','error')},
                        'rho_triangles_lower':q.get('rho_triangles_whole_lower'),'rho_triangles_upper':q.get('rho_triangles_whole_upper'),
                        'rho_merged_lower':q.get('rho_merged_whole_lower'),'rho_merged_upper':q.get('rho_merged_whole_upper'),
                        'rss_kib':row.get('rss_kib'),'cause':row.get('cause',''),'failure_detail':row.get('failure_detail',''),'failure_category':row.get('failure_category','')})
    if summary:
        with (out/'mesh-summary.csv').open('w') as f:
            writer=csv.DictWriter(f,fieldnames=list(summary[0]));writer.writeheader();writer.writerows(summary)
    distributions=[]
    for row in complete:
        for metric,statistics in row['quality'].get('metrics',{}).items():
            if metric=='cell':continue
            distributions.append({'mesh':row['mesh']['name'],'family':row['mesh']['family'],'variant':row['variant'],
                 'metric':metric,'cells_total':row['quality']['cells'],**statistics})
    if distributions:
        with (out/'cell-metric-distributions.csv').open('w') as f:
            writer=csv.DictWriter(f,fieldnames=['mesh','family','variant','metric','cells_total','min','p5','median','p95','max','valid_count'])
            writer.writeheader();writer.writerows(distributions)
    baselines=('face-area','face-incircle','face-area-repair','face-incircle-repair','edge','pairs')
    comparisons=[]
    for baseline in baselines:
        baseline_rows=[]
        for family in ('random','poisson','grid','quality','real'):
            expected_pairs=sum(r['variant']=='cavity-ascending' and r['mesh']['family']==family and r['mesh'].get('stratum')!='reference' for r in rows)
            pairs=[]
            for row in complete:
                if row['variant']!='cavity-ascending' or row['mesh']['family']!=family or row['mesh'].get('stratum')=='reference':continue
                other=index.get((row['mesh']['name'],baseline))
                if not other or 'quality' not in other:continue
                pairs.append((row['quality'],other['quality'],row['mesh']))
            if not pairs:
                baseline_rows.append({'baseline':baseline,'family':family,'n':0,'planned_pairs':expected_pairs,'pair_coverage':0.,'p_raw':1.})
                continue
            lower=[a['kernel_fraction_lower']-b['kernel_fraction_upper'] for a,b,mesh in pairs]
            upper=[a['kernel_fraction_upper']-b['kernel_fraction_lower'] for a,b,mesh in pairs]
            exact=[a['kernel_fraction_lower']-b['kernel_fraction_lower'] for a,b,mesh in pairs
                   if all(q['kernel_states']['indeterminate']==0 and q['kernel_states']['error']==0 for q in (a,b))]
            reduction=[a['reduction']-b['reduction'] for a,b,mesh in pairs]
            primary_interval=(lambda values: grouped_interval(values,[mesh.get('stratum') for a,b,mesh in pairs],'stratified')) if family=='real' else interval
            cluster_lower=grouped_interval(lower,[mesh['seed'] for a,b,mesh in pairs],'cluster') if family in ('random','poisson') else None
            cluster_reduction=grouped_interval(reduction,[mesh['seed'] for a,b,mesh in pairs],'cluster') if family in ('random','poisson') else None
            pvalue=float(wilcoxon(np.round(exact,12),alternative='two-sided').pvalue) if exact and np.any(np.round(exact,12)!=0) else 1.
            baseline_rows.append({'baseline':baseline,'family':family,'n':len(pairs),'planned_pairs':expected_pairs,'pair_coverage':len(pairs)/expected_pairs if expected_pairs else 0.,'kernel_difference_lower':primary_interval(lower),
                'kernel_difference_upper':primary_interval(upper),'kernel_difference_fully_resolved':interval(exact),
                'reduction_difference':primary_interval(reduction),'cluster_sensitivity_kernel_lower':cluster_lower,'cluster_sensitivity_reduction':cluster_reduction,'noninferiority_fraction_per_mesh':float(np.mean(np.asarray(reduction)>=-.1)),
                'p_raw':pvalue,'wilcoxon_resolved_pairs':len(exact)})
        corrected=holm([row['p_raw'] for row in baseline_rows])
        for row,pvalue in zip(baseline_rows,corrected):
            row['p_holm_5_families']=pvalue
            effect=row.get('kernel_difference_lower')
            row['h3a_effect_pass_on_observed_pairs']=bool(effect and effect['median']>=.05 and effect['ci95_low']>0)
            row['h3a_evaluable_as_planned']=row.get('pair_coverage',0)==1.
        comparisons.extend(baseline_rows)
    seed_results=[]
    for family in ('random','poisson','grid','quality'):
        for variant in ('cavity-descending','cavity-index','cavity-random'):
            lower=[];upper=[];reduction=[]
            for row in complete:
                if row['variant']!='cavity-ascending' or row['mesh']['family']!=family:continue
                other=index.get((row['mesh']['name'],variant))
                if not other or 'quality' not in other:continue
                a,b=row['quality'],other['quality']
                lower.append(b['kernel_fraction_lower']-a['kernel_fraction_upper'])
                upper.append(b['kernel_fraction_upper']-a['kernel_fraction_lower'])
                reduction.append(b['reduction']-a['reduction'])
            seed_results.append({'family':family,'variant':variant,'kernel_difference_lower':interval(lower),
                                 'kernel_difference_upper':interval(upper),'reduction_difference':interval(reduction)})
    timing_file=root/'timing-results.json'
    timing_rows=json.loads(timing_file.read_text()) if timing_file.exists() else []
    groups=defaultdict(list)
    for row in timing_rows:
        if row['status']=='complete':groups[(row['family'],row['n'],row['variant'])].append(row)
    timing_summary=[];time_ratios=[]
    for (family,n,variant),group in sorted(groups.items()):
        record={'family':family,'n':n,'variant':variant,'repetitions':len(group),
                'vertices':int(group[0]['vertices']),'tetrahedra':int(group[0]['tetrahedra'])}
        hashes={row['membership_sha256'] for row in group}
        record['partition_hash_count']=len(hashes)
        record['stable_partition']=len(hashes)==1
        quality_reference=index.get((f'{family}_{n}_s1',variant))
        expected_hash=quality_reference.get('membership_sha256') if quality_reference else None
        record['matches_quality_partition']=next(iter(hashes))==expected_hash if expected_hash else None
        for field in ('read_ms','algorithm_ms','validation_ms','write_ms','total_ms','rss_kib'):
            values=np.asarray([float(row[field]) for row in group])
            record[field]={'median':float(np.median(values)),'q25':float(np.quantile(values,.25)),'q75':float(np.quantile(values,.75)),'max':float(np.max(values))}
        timing_summary.append(record)
        if variant=='cavity-ascending':continue
        cavity=groups.get((family,n,'cavity-ascending'))
        if not cavity or len(group)!=10 or len(cavity)!=10 or len(hashes)!=1:continue
        if len({row['membership_sha256'] for row in cavity})!=1 or record['matches_quality_partition'] is False:continue
        a=np.array([float(row['algorithm_ms']) for row in cavity]);b=np.array([float(row['algorithm_ms']) for row in group])
        rng=np.random.default_rng(SEED)
        samples=np.median(a[rng.integers(0,len(a),(BOOTSTRAPS,len(a)))],axis=1)/np.median(b[rng.integers(0,len(b),(BOOTSTRAPS,len(b)))],axis=1)
        time_ratios.append({'family':family,'n':n,'baseline':variant,'ratio_cavity_to_face':float(np.median(a)/np.median(b)),
                            'ci95_low':float(np.quantile(samples,.025)),'ci95_high':float(np.quantile(samples,.975))})
    statistics_timing=[]
    for path in (root/'timing').glob('*/*_statistics.statistics-timing.json'):
        record=json.loads(path.read_text());record['input']=path.parent.name
        record['variant']=path.name.removesuffix('_statistics.statistics-timing.json')
        statistics_timing.append(record)
    failures=[row for row in summary if row['status']=='failed']
    flat_file=root/'flat-results.json'
    flat=json.loads(flat_file.read_text()) if flat_file.exists() else []
    report={'bootstrap_resamples':BOOTSTRAPS,'bootstrap_seed':SEED,'mesh_unit_of_analysis':True,'real_bootstrap':'within observed strata','synthetic_sensitivity':'resample seed clusters across sizes',
            'kernel_audit':json.loads((root/'kernel-audit.json').read_text()) if (root/'kernel-audit.json').exists() else None,
            'planned_jobs_completed_or_failed':len(rows),'status_counts':dict(Counter(row['status'] for row in rows)),'primary_status_counts':dict(Counter(row.get('primary_status',row['status']) for row in rows)),
            'failure_categories':dict(Counter(row['failure_category'] for row in summary if row['status']=='failed')),
            'failures_by_family_variant':{str(key):value for key,value in Counter((row['family'],row['variant']) for row in failures).items()},
            'comparisons':comparisons,'seed_order_comparisons':seed_results,'timing':timing_summary,'timing_ratios':time_ratios,
            'statistics_timing':statistics_timing,'flat_representation':flat,'extensions':{'E3':'excluded: optional Voronoi extension outside core campaign',
              'E7':'excluded per plan 7.1: no validated VEM solver at E2 closure; conclusions limited to geometry',
              'E8':'excluded per condition C4-11: no validated parallel implementation; no speedup asserted'}}
    (out/'analysis.json').write_text(json.dumps(report,indent=2)+'\n')
    (out/'failures.json').write_text(json.dumps(failures,indent=2)+'\n')
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        from matplotlib.ticker import PercentFormatter
        variants=('cavity-ascending','face-area','face-incircle','edge','pairs')
        plt.rcParams.update({'font.size':14,'axes.labelsize':16,'xtick.labelsize':14,'ytick.labelsize':14})
        labels={'cavity-ascending':'Cavidades','face-area':'Face área','face-incircle':'Face radio','edge':'Edge','pairs':'Pares'}
        colors={variant:f'C{i}' for i,variant in enumerate(variants)}
        fig,axes=plt.subplots(1,2,figsize=(11,4.5),layout='constrained')
        for variant in variants:
            data=[row for row in summary if row['variant']==variant and row['status']!='failed' and row['family']!='real']
            if data:axes[0].scatter([row['reduction'] for row in data],[row['kernel_fraction_lower'] for row in data],s=20,alpha=.6,label=labels[variant],color=colors[variant])
        axes[0].set(xlabel='Reducción de celdas',ylabel='Fracción mínima confirmada con kernel');axes[0].legend(fontsize=14);axes[0].set(xlim=(0,1),ylim=(0,1.03))
        axes[0].xaxis.set_major_formatter(PercentFormatter(1));axes[0].yaxis.set_major_formatter(PercentFormatter(1))
        for family,marker in [('random','o'),('quality','s')]:
            for variant in variants[:3]:
                values=[row for row in timing_summary if row['family']==family and row['variant']==variant]
                if values:axes[1].loglog([r['n'] for r in values],[r['algorithm_ms']['median'] for r in values],marker=marker,label=f'{family.title()}: {labels[variant]}',color=colors[variant])
        axes[1].set(xlabel='Vértices objetivo',ylabel='Tiempo de algoritmo (ms)');axes[1].legend(fontsize=14)
        fig.savefig(out/'quality-and-timing.pdf');fig.savefig(out/'quality-and-timing.svg');fig.savefig(out/'quality-and-timing.png',dpi=180);plt.close(fig)
    except ImportError:pass
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',required=True);p.add_argument('--out',required=True)
    a=p.parse_args();report=analyze(a.root,a.out)
    print(json.dumps({'status_counts':report['status_counts'],'jobs':report['planned_jobs_completed_or_failed']}))
