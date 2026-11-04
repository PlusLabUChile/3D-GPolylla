import csv
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


@unittest.skipUnless(os.environ.get('GPOLYLLA_EXE'), 'Set GPOLYLLA_EXE to test the compiled CLI')
class CsvTest(unittest.TestCase):
    def test_aggregate_and_detailed_schema_and_values(self):
        source=Path(__file__).resolve().parents[2]
        with tempfile.TemporaryDirectory() as directory:
            output=Path(directory)/'basic.visf'
            subprocess.run([os.environ['GPOLYLLA_EXE'],'-n',str(source/'data/basic.node'),'-e',str(source/'data/basic.ele'),
                            '-o',str(output),'--make-stats','--detail-stats'],check=True,capture_output=True)
            for path in (output.with_suffix('.csv'),Path(directory)/'basic_detail/details.csv'):
                with path.open() as f:rows=list(csv.reader(f))
                self.assertEqual(len(rows),2)
                self.assertEqual(len(rows[0]),len(rows[1]))
                self.assertNotIn('',rows[0])
                values=dict(zip(rows[0],rows[1]))
                if 'edge ratio (min)' in values:
                    self.assertAlmostEqual(float(values['edge ratio (min)']),2**-.5,places=6)
                    self.assertEqual(float(values['surface ratio (min)']),1)
                else:
                    self.assertEqual(float(values['poly volume']),1)
                    self.assertEqual(float(values['kernel volume']),1)
