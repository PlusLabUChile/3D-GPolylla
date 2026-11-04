import sys
from pathlib import Path
import unittest
import warnings
import itertools
import numpy as np
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from evaluate import quality, load_mesh, kernel_measure


class QualityTest(unittest.TestCase):
    def voxels(self, cells):
        vertices, lookup, tetras = [], {}, []
        def vertex(p):
            p = tuple(p)
            if p not in lookup:
                lookup[p] = len(vertices); vertices.append(p)
            return lookup[p]
        for origin in cells:
            for axes in itertools.permutations(range(3)):
                p = np.array(origin)
                tetra = [vertex(p)]
                for axis in axes:
                    p = p.copy(); p[axis] += 1
                    tetra.append(vertex(p))
                tetras.append(tetra)
        vertices, tetras = np.array(vertices, dtype=float), np.array(tetras)
        volumes = np.full(len(tetras), 1/6)
        return vertices, tetras, volumes

    def test_cube(self):
        v, t, volumes = self.voxels([(0,0,0)])
        q = quality(v, t, volumes)
        self.assertEqual(q['kernel_state'], 'present')
        self.assertAlmostEqual(q['kernel_fraction'], 1)
        self.assertEqual(q['faces_merged'], 6)
        self.assertEqual(q['edges_merged'], 12)
        self.assertAlmostEqual(q['edge_ratio_merged'], 1)

    def test_subdivided_cube_has_twelve_geometric_edges(self):
        v, t, volumes = self.voxels([(0,0,0), (1,0,0), (0,1,0), (1,1,0)])
        q = quality(v,t,volumes)
        self.assertEqual(q['faces_merged'], 6)
        self.assertEqual(q['edges_merged'], 12)
        self.assertAlmostEqual(q['edge_ratio_merged'], .5)

    def test_thin_tetrahedron_uses_exact_convexity_identity(self):
        for height in (1e-12,1e-16):
            v=np.array([[0.,0.,0.],[1.,0.,0.],[0.,1.,0.],[.2,.2,height]])
            q=quality(v,np.array([[0,1,2,3]]),np.array([height/6]))
            self.assertEqual(q['kernel_state'],'present')
            self.assertEqual(q['kernel_fraction'],1)
            self.assertEqual(q['faces_merged'],4)
            self.assertEqual(q['rho3_merged'],1)

    def test_flat_tetrahedron_is_not_promoted_by_noisy_volume(self):
        v=np.array([[0.,0.,0.],[1.,0.,0.],[0.,1.,0.],[.2,.2,0.]])
        with self.assertRaisesRegex(ValueError,'exact orientation'):
            quality(v,np.array([[0,1,2,3]]),np.array([1e-20]))

    def test_l_and_u(self):
        v, t, volumes = self.voxels([(0,0,0), (1,0,0), (0,1,0)])
        q = quality(v, t, volumes)
        self.assertEqual(q['kernel_state'], 'present')
        self.assertAlmostEqual(q['kernel_fraction'], 1/3)
        v, t, volumes = self.voxels([(0,0,0), (1,0,0), (2,0,0), (0,1,0), (2,1,0)])
        q = quality(v, t, volumes)
        self.assertEqual(q['kernel_state'], 'absent_confirmed')
        self.assertEqual(q['kernel_fraction'], 0)

    def test_scale_translation_orientation(self):
        v, t, volumes = self.voxels([(0,0,0), (1,0,0), (0,1,0)])
        for scale in (.001, 1, 1000):
            q = quality(v*scale + [2,-3,4], t[:, ::-1], volumes*scale**3)
            self.assertAlmostEqual(q['kernel_fraction'], 1/3, places=7)

    def test_near_zero_margin_not_absence(self):
        normals = np.vstack((np.eye(3), -np.eye(3)))
        state, _, _ = kernel_measure(normals, np.array([0, -1, -1, 0, 0, 0]))
        self.assertEqual(state, 'indeterminate')



class CertificateTest(unittest.TestCase):
    def test_dual_combination_proves_infeasible_box(self):
        from kernel_certificate import certify_absence
        normals=np.vstack((np.eye(3),-np.eye(3)))
        offsets=np.array([0.,-1.,-1.,.2,-1.,-1.])
        result=certify_absence(normals,offsets)
        self.assertTrue(result['certified'])
        weights=np.array(result['weights'])
        self.assertTrue(np.all(weights>=0))
        self.assertGreater(weights@offsets-np.abs(weights@normals).sum(),.09)

    def test_small_incompatibility_is_not_confirmed(self):
        from kernel_certificate import certify_absence
        normals=np.vstack((np.eye(3),-np.eye(3)))
        result=certify_absence(normals,np.array([0.,-1.,-1.,1e-10,-1.,-1.]))
        self.assertFalse(result['certified'])


if __name__ == '__main__':
    with warnings.catch_warnings():
        warnings.filterwarnings('ignore', message='Unrecognized options detected')
        unittest.main()
