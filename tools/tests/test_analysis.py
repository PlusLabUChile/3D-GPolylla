import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from analyze import interval,holm


class InferenceTest(unittest.TestCase):
    def test_degenerate_bootstrap_and_no_observations(self):
        result=interval([.125]*5)
        self.assertEqual(result,{'n':5,'median':.125,'ci95_low':.125,'ci95_high':.125})
        self.assertIsNone(interval([]))

    def test_holm_known_adjustment_in_original_order(self):
        adjusted=holm([.01,.04,.03])
        for observed,expected in zip(adjusted,[.03,.06,.06]):self.assertAlmostEqual(observed,expected)
