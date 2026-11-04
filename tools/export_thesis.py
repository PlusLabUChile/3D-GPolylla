#!/usr/bin/env python3
"""Export measured aggregates to Typst; never substitutes missing data with 0."""
import argparse
from collections import defaultdict
import csv
import json
from pathlib import Path
import shutil
import numpy as np

LABELS={'cavity-ascending':'Cavidades','face-area':'Face área','face-incircle':'Face radio',
        'face-area-repair':'Face área + reparación','face-incircle-repair':'Face radio + reparación',
        'edge':'Edge (referencia)','pairs':'Pares'}
FAMILIES={'random':'Random','poisson':'Poisson','grid':'Grid','quality':'Quality','real':'Thingi10K'}


def pct(value):return '—' if value is None else f'{100*float(value):.1f} %'
def num(value):return '—' if value is None else f'{float(value):.2f}'


def export(analysis_dir,thesis_dir):
    analysis_dir=Path(analysis_dir);thesis_dir=Path(thesis_dir)
    report=json.loads((analysis_dir/'analysis.json').read_text())
    with (analysis_dir/'mesh-summary.csv').open() as f:rows=list(csv.DictReader(f))
    data=thesis_dir/'resultados'/'campana-20261103';data.mkdir(parents=True,exist_ok=True)
    for name in ('analysis.json','mesh-summary.csv','failures.json','quality-and-timing.pdf','quality-and-timing.svg','quality-and-timing.png'):
        if (analysis_dir/name).exists():shutil.copyfile(analysis_dir/name,data/name)
    lines=['// Generado desde resultados medidos; regenerar con tools/export_thesis.py.','']
    for family,title in FAMILIES.items():
        cells=[]
        for variant,label in LABELS.items():
            group=[r for r in rows if r['family']==family and r['variant']==variant and r.get('stratum')!='reference']
            valid=[r for r in group if r['status']=='complete']
            reduction=float(np.median([float(r['reduction']) for r in valid])) if valid else None
            lower=float(np.median([float(r['kernel_fraction_lower']) for r in valid])) if valid else None
            upper=float(np.median([float(r['kernel_fraction_upper']) for r in valid])) if valid else None
            isolated=float(np.median([float(r['isolated'])/float(r['cells']) for r in valid])) if valid else None
            cells.append(f'    [{label}], [{len(valid)}/{len(group)}], [{pct(reduction)}], [{pct(lower)} – {pct(upper)}], [{pct(isolated)}],')
        label='tab:ex-calidad' if family=='random' else 'tab:ex-calidad-'+family
        lines+=['#figure(', '  table(', '    columns: (1.8fr, 0.7fr, 0.8fr, 1.4fr, 0.8fr), inset: 4pt,',
                '    table.header([*Variante*], [*Válidas*], [*$R$*], [*$f_K$ acotado*], [*Aislados*]),',*cells,
                f'  ), caption: [Calidad en {title}. Medianas por malla; el intervalo de $f_K$ separa incertidumbre numérica. Aislados: fracción de celdas unitarias.],',
                f') <{label}>','']
    lines+=['#figure(', '  table(', '    columns: (0.8fr, 1.5fr, 0.7fr, 1.5fr, 0.9fr), inset: 4pt,',
            '    table.header([*Familia*], [*Variante*], [*Vértices*], [*Algoritmo: mediana (Q1; Q3), ms*], [*RSS máx., MiB*]),']
    for family in ('random','quality'):
        for variant in ('cavity-ascending','face-area','face-incircle','face-area-repair','face-incircle-repair'):
            candidates=[r for r in report['timing'] if r['family']==family and r['variant']==variant and r['repetitions']==10 and r['n']<=50000]
            if not candidates:continue
            r=max(candidates,key=lambda r:r['n']);t=r['algorithm_ms']
            lines.append(f'    [{FAMILIES[family]}], [{LABELS[variant]}], [{r["vertices"]}], [{num(t["median"])} ({num(t["q25"])}; {num(t["q75"])})], [{num(r["rss_kib"]["max"]/1024)}],')
    lines+=['  ), caption: [Recursos del mayor tamaño núcleo con diez repeticiones completas por variante. La escritura corresponde a pertenencia, no VISF.],',
            ') <tab:ex-recursos>','']
    lines+=['#figure(', '  table(', '    columns: (1fr, 1fr, 1fr, 1fr, 1fr), inset: 4pt,',
            '    table.header([*Familia*], [*Listas actuales, MiB*], [*Aplanadas, MiB*], [*Retenido actual, MiB*], [*Retenido aplanado, MiB*]),']
    for r in report['flat_representation']:
        if r['status']!='complete':continue
        inv=r['inventory']
        lines.append(f'    [{FAMILIES[r["family"]]}], [{num(int(r["nested_capacity_bytes"])/1024**2)}], [{num(int(r["flat_actual_bytes"])/1024**2)}], [{num(int(inv["retained_sum_bytes"])/1024**2)}], [{num(r["flat_retained_sum_bytes"]/1024**2)}],')
    lines+=['  ), caption: [E6: capacidades efectivamente reservadas. El total retenido incluye entrada, copia común de salida y auxiliares persistentes; no es el RSS ni incluye metadatos del asignador.],',
            ') <tab:ex-memoria>','']
    if (data/'quality-and-timing.svg').exists():
        lines+=['#figure(image("../resultados/campana-20261103/quality-and-timing.svg", width: 100%),',
                '  caption: [Reducción y fracción confirmada de kernel por malla sintética; tiempos de algoritmo por tamaño objetivo.],',
                ') <fig:ex-campana>','']
    (thesis_dir/'capitulos'/'resultados-campana.typ').write_text('\n'.join(lines))


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--analysis-dir',required=True);p.add_argument('--thesis-dir',required=True)
    a=p.parse_args();export(a.analysis_dir,a.thesis_dir)
