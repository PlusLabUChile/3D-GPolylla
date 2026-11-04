import copy
import csv
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from audit_kernels import audit_one
from evaluate import evaluate


class AuditTest(unittest.TestCase):
    def test_false_absence_is_downgraded_without_mutating_primary_outputs(self):
        source=Path(__file__).resolve().parents[2]
        node=source/'data/basic.node';ele=source/'data/basic.ele'
        with tempfile.TemporaryDirectory() as directory:
            folder=Path(directory)
            np.savetxt(folder/'quality_0.membership',np.zeros(5,dtype=int),fmt='%d')
            q=evaluate(node,ele,folder/'quality_0.membership',folder/'quality_0')
            self.assertEqual(q['kernel_states']['present'],1)
            with (folder/'quality_0.details.csv').open() as f:rows=list(csv.DictReader(f))
            for row in rows:
                row['kernel_state']='absent_confirmed'
                for field in ('kernel_volume','kernel_fraction','rho1_triangles','rho1_merged'):row[field]='0'
                row['volume_ratio_legacy']=''
            with (folder/'quality_0.details.csv').open('w') as f:
                writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
            q['kernel_states']={'present':0,'absent_confirmed':1,'indeterminate':0,'error':0}
            q['kernel_fraction_lower']=q['kernel_fraction_upper']=0
            primary=folder/'result.json'
            primary.write_text(json.dumps({'mesh':{'name':'analytic_cube','node':str(node),'ele':str(ele)},'variant':'test','quality':q}))
            before={name:(folder/name).read_bytes() for name in ('result.json','quality_0.details.csv')}
            report=audit_one(primary)
            self.assertEqual(report['checked_absent_cells'],1)
            self.assertEqual(report['certified_absent_cells'],0)
            self.assertEqual(report['downgraded_cells'],[0])
            corrected=report['corrected_quality']
            self.assertEqual(corrected['kernel_states']['indeterminate'],1)
            self.assertEqual(corrected['kernel_fraction_lower'],0)
            self.assertEqual(corrected['kernel_fraction_upper'],1)
            self.assertIsNone(corrected['rho_triangles'])
            for name,data in before.items():self.assertEqual((folder/name).read_bytes(),data)
