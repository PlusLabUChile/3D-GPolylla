#!/usr/bin/env python3
"""Run the unmodified reference classes against a canonical, efficient adapter.

Reference sources are copied outside 3D-Polylla before execution; SHA256 is
recorded. Geometry selection is parametrized in a subclass. The legacy Face
'incircle' implementation uses squared side lengths in Heron's formula;
face-incircle explicitly corrects that calculation and records the distinction.
"""
import argparse
from collections import defaultdict
import hashlib
import importlib.util
import itertools
import json
from pathlib import Path
import shutil
import sys
from types import SimpleNamespace as NS
import numpy as np
from evaluate import load_mesh, cell_boundary


def adapter(node, ele):
    v, t, _ = load_mesh(node, ele)
    faces = defaultdict(list)
    for ti, tetra in enumerate(t):
        for i in range(4):
            faces[tuple(sorted(int(x) for j, x in enumerate(tetra) if j != i))].append(ti)
    keys = sorted(faces)
    edge_keys = sorted({pair for face in keys for pair in itertools.combinations(face, 2)})
    edge_ids = {key: i for i, key in enumerate(edge_keys)}
    face_ids = {key: i for i, key in enumerate(keys)}
    face_list = []
    for fi, face in enumerate(keys):
        neighbors = faces[face]
        if len(neighbors) > 2:
            raise ValueError('Non-manifold input')
        face_list.append(NS(i=fi, v1=face[0], v2=face[1], v3=face[2], n1=neighbors[0],
                            n2=neighbors[1] if len(neighbors) == 2 else -1,
                            edges=[edge_ids[pair] for pair in itertools.combinations(face, 2)],
                            is_boundary=len(neighbors) == 1, area=-1))
    tetra_list = []
    edges_to_tetra = defaultdict(list)
    for ti, tetra in enumerate(t):
        tetra_faces = sorted(face_ids[tuple(sorted(int(x) for j, x in enumerate(tetra) if j != i))] for i in range(4))
        tetra_edges = [edge_ids[pair] for pair in itertools.combinations(sorted(map(int, tetra)), 2)]
        for edge in tetra_edges:
            edges_to_tetra[edge].append(ti)
        tetra_list.append(NS(i=ti, faces=tetra_faces, edges=tetra_edges,
                             neighs=[face_list[fi].n2 if face_list[fi].n1 == ti else face_list[fi].n1 for fi in tetra_faces]))
    edge_list = []
    for ei, edge in enumerate(edge_keys):
        incident = edges_to_tetra[ei]
        # Walk the tetrahedron fan using faces incident to this edge. Boundary
        # fans start at a boundary tetrahedron; interior fans form a cycle.
        edge_faces = []
        adjacency = defaultdict(list)
        boundary_tetra = []
        for ti in incident:
            for fi in tetra_list[ti].faces:
                face = face_list[fi]
                if ei not in face.edges:
                    continue
                nxt = face.n2 if face.n1 == ti else face.n1
                if nxt == -1:
                    boundary_tetra.append(ti)
                else:
                    adjacency[ti].append((nxt, fi))
        start = min(boundary_tetra or incident)
        order, seen, current = [], set(), start
        while current not in seen:
            seen.add(current); order.append(current)
            choices = [(nxt, fi) for nxt, fi in adjacency[current] if nxt not in seen]
            if not choices:
                break
            current, fi = min(choices)
            edge_faces.append(fi)
        all_faces = sorted({fi for ti in incident for fi in tetra_list[ti].faces if ei in face_list[fi].edges})
        if len(order) != len(incident):
            raise ValueError('Disconnected edge fan')
        # Repair routines require the cyclic order of faces, with boundary
        # faces at the ends of an open fan.
        if boundary_tetra:
            first_faces = [fi for fi in tetra_list[order[0]].faces if ei in face_list[fi].edges and face_list[fi].is_boundary]
            last_faces = [fi for fi in tetra_list[order[-1]].faces if ei in face_list[fi].edges and face_list[fi].is_boundary]
            ordered_faces = sorted(first_faces)[:1] + edge_faces + sorted(last_faces)[-1:]
        else:
            closing = [fi for nxt, fi in adjacency[order[-1]] if nxt == start]
            ordered_faces = edge_faces + closing[:1]
        edge_list.append(NS(i=ei, v1=edge[0], v2=edge[1], tetrahedrons=order,
                            faces=ordered_faces if set(ordered_faces) == set(all_faces) else all_faces,
                            length=-1, is_boundary=bool(boundary_tetra), first_tetra=start))
    return NS(node_list=[NS(i=i, x=p[0], y=p[1], z=p[2]) for i, p in enumerate(v)],
              face_list=face_list, tetra_list=tetra_list, edge_list=edge_list,
              n_nodes=len(v), n_faces=len(face_list), n_tetrahedrons=len(t), n_edges=len(edge_list))


def run(node, ele, variant, output, reference, cache):
    cache = Path(cache); cache.mkdir(parents=True, exist_ok=True)
    hashes = {}
    for name in ('mesh.py', 'PolyllaFace.py', 'PolyllaEdge.py'):
        src = Path(reference) / name
        hashes[name] = hashlib.sha256(src.read_bytes()).hexdigest()
        dst = cache / name
        if not dst.exists() or dst.read_bytes() != src.read_bytes():
            shutil.copyfile(src, dst)
    sys.path.insert(0, str(cache.resolve()))
    sys.setrecursionlimit(100000)
    from PolyllaFace import PolyllaFace
    from PolyllaEdge import PolyllaEdge
    mesh = adapter(node, ele)
    class Face(PolyllaFace):
        def calculate_max_incircle_faces(self):
            if 'legacy' in variant:
                return super().calculate_max_incircle_faces()
            score = []
            for f in self.mesh.face_list:
                p = [np.array([self.mesh.node_list[i].x, self.mesh.node_list[i].y, self.mesh.node_list[i].z]) for i in (f.v1,f.v2,f.v3)]
                area2 = np.linalg.norm(np.cross(p[1]-p[0], p[2]-p[0]))
                score.append(area2 if 'area' in variant else area2 / sum(np.linalg.norm(p[i]-p[(i+1)%3]) for i in range(3)))
            score = np.rint(np.array(score) / max(score) * 1e12)
            return [max(range(4), key=lambda i: score[tetra.faces[i]]) for tetra in self.mesh.tetra_list]
        def count_barrierFaces(self, polyhedron):
            return super().count_barrierFaces(polyhedron) if variant.endswith('-repair') else 0
    algorithm = PolyllaEdge(mesh) if variant == 'edge' else Face(mesh)
    owner = np.full(mesh.n_tetrahedrons, -1, dtype=np.int64)
    for pi, poly in enumerate(algorithm.polyhedral_mesh):
        for ti in poly.tetras:
            if ti < 0 or ti >= len(owner) or owner[ti] != -1:
                raise ValueError(f'Reference duplicates or invalid tetrahedron {ti}')
            owner[ti] = pi
    if np.any(owner < 0):
        raise ValueError(f'Reference leaves {int(np.sum(owner < 0))} tetrahedra uncovered')
    # Canonical IDs make equality independent of region traversal order.
    mapping = {}
    canonical = []
    for pi in owner:
        if int(pi) not in mapping:
            mapping[int(pi)] = len(mapping)
        canonical.append(mapping[int(pi)])
    np.savetxt(output, canonical, fmt='%d')
    Path(str(output)+'.provenance.json').write_text(json.dumps({'source_sha256': hashes,
                'variant': variant, 'adapter': 'canonical face/edge adjacency; no input retetrahedralization',
                'incircle': 'legacy squared-side expression' if 'legacy' in variant else 'geometric inradius'}, indent=2)+'\n')


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    for name in ('node','ele','variant','out','reference','cache'):
        p.add_argument('--'+name, required=True)
    a = p.parse_args()
    run(a.node,a.ele,a.variant,a.out,a.reference,a.cache)
