#!/usr/bin/env python3
"""Reproducible TetGen inputs for the experiment plan; no reference files changed."""
import argparse
import hashlib
import itertools
import json
import math
from pathlib import Path
import subprocess
import numpy as np
import scipy
from scipy.stats import qmc


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def generate(family, n, seed, out, tetgen, epsilon=0):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    manifest = out / 'manifest.json'
    if manifest.exists():
        info = json.loads(manifest.read_text())
        if all(sha(out / name) == digest for name, digest in info['hashes'].items()):
            return info
        raise ValueError(f'Input checksum changed: {out}')
    rng = np.random.default_rng(seed)
    corners = np.array(list(itertools.product([0., 1.], repeat=3)))
    parameters = {}
    if family == 'random':
        points = np.vstack((rng.random((n - 8, 3)), corners))
    elif family == 'poisson':
        # Radius calibration uses this seed and is preserved in the manifest.
        radius = (0.65 / n) ** (1 / 3)
        for iteration in range(4):
            sampled = qmc.PoissonDisk(3, radius=radius, rng=np.random.default_rng(seed)).fill_space()
            if abs(len(sampled) + 8 - n) / n < .08:
                break
            radius *= ((len(sampled) + 8) / n) ** (1 / 3)
        points = np.vstack((sampled, corners))
        parameters = {'radius': radius, 'calibration_iterations': iteration + 1}
    elif family in ('grid', 'grid-perturbed'):
        k = math.ceil(n ** (1 / 3))
        points = np.array(list(itertools.product(np.linspace(0, 1, k), repeat=3)))
        if epsilon:
            interior = (points > 0) & (points < 1)
            points += rng.uniform(-epsilon, epsilon, points.shape) / (k - 1) * interior
        parameters = {'k': k, 'epsilon': epsilon, 'seed_used': bool(epsilon)}
    elif family == 'quality':
        points = corners
        parameters = {'max_volume': 0.5 / n, 'seed_used': False,
                      'note': 'TetGen PLC refinement is deterministic; seed replicas are not independent.'}
    else:
        raise ValueError(family)
    node = out / 'input.node'
    with node.open('w') as f:
        f.write(f'{len(points)} 3 0 0\n')
        for i, p in enumerate(points):
            f.write(f'{i} ' + ' '.join(format(v, '.17g') for v in p) + '\n')
    if family == 'quality':
        poly = out / 'input.poly'
        faces = [(0, 1, 3, 2), (4, 6, 7, 5), (0, 4, 5, 1), (2, 3, 7, 6), (0, 2, 6, 4), (1, 5, 7, 3)]
        with poly.open('w') as f:
            f.write('0 3 0 0\n6 0\n')
            for face in faces:
                f.write('1 0\n4 ' + ' '.join(map(str, face)) + '\n')
            f.write('0\n0\n')
        cmd = [str(tetgen), f'-pq1.414a{parameters["max_volume"]:.17g}fennzQ', str(poly.resolve())]
    else:
        cmd = [str(tetgen), '-fennzQ', str(node.resolve())]
    with (out / 'tetgen.log').open('w') as log:
        subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=1800)
    calibration_commands = [cmd]
    if family == 'quality':
        for iteration in range(4):
            actual = int((out / 'input.1.node').read_text().split()[0])
            if abs(actual-n)/n < .08:
                break
            parameters['max_volume'] *= actual/n
            cmd = [str(tetgen), f'-pq1.414a{parameters["max_volume"]:.17g}fennzQ', str(poly.resolve())]
            with (out / 'tetgen.log').open('a') as log:
                subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=1800)
            calibration_commands.append(cmd)
        parameters['calibration_commands'] = calibration_commands
    files = ['input.1.node', 'input.1.ele', 'input.1.face', 'input.1.edge']
    info = {'family': family, 'target_vertices': n, 'seed': seed, 'parameters': parameters,
            'command': cmd, 'tetgen_sha256': sha(tetgen), 'numpy': np.__version__, 'scipy': scipy.__version__,
            'vertices': int((out / files[0]).read_text().split()[0]),
            'tetrahedra': int((out / files[1]).read_text().split()[0]),
            'hashes': {name: sha(out / name) for name in files}}
    manifest.write_text(json.dumps(info, indent=2) + '\n')
    return info


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--family', choices=['random', 'poisson', 'grid', 'grid-perturbed', 'quality'], required=True)
    p.add_argument('--n', type=int, required=True)
    p.add_argument('--seed', type=int, default=1)
    p.add_argument('--epsilon', type=float, default=0)
    p.add_argument('--out', required=True)
    p.add_argument('--tetgen', required=True)
    a = p.parse_args()
    print(json.dumps(generate(a.family, a.n, a.seed, a.out, a.tetgen, a.epsilon), indent=2))
