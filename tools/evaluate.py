#!/usr/bin/env python3
"""One double-precision membership evaluator shared by every algorithm.

Kernel = intersection of outward face halfspaces. A normalized Chebyshev LP
provides an interior point or an infeasibility certificate. Thin/near-empty
results remain indeterminate. Qhull never certifies absence by itself.
"""
import argparse
from collections import Counter, defaultdict
import csv
import json
from pathlib import Path
import numpy as np
from scipy.optimize import linprog
from scipy.spatial import ConvexHull, HalfspaceIntersection, QhullError
from scipy.spatial.distance import pdist
from kernel_certificate import certify_absence, exact_tetrahedron_orientation


def rows(path):
    with Path(path).open() as f:
        for line in f:
            line = line.split('#', 1)[0].strip()
            if line:
                yield line.split()


def load_mesh(node, ele):
    source = rows(node)
    count, dim, attrs, markers = map(int, next(source))
    if dim != 3:
        raise ValueError('Expected 3D mesh')
    vertices = np.empty((count, 3))
    ids = {}
    for i in range(count):
        row = next(source)
        external = int(row[0])
        if external in ids or len(row) < 4 + attrs + markers:
            raise ValueError('Duplicate vertex or missing attributes')
        ids[external] = i
        vertices[i] = list(map(float, row[1:4]))
    source = rows(ele)
    count, corners, attrs = map(int, next(source))
    if corners != 4:
        raise ValueError('Expected tetrahedra')
    tetras = np.empty((count, 4), dtype=np.int64)
    for i in range(count):
        row = next(source)
        tetras[i] = [ids[int(x)] for x in row[1:5]]
    p = vertices[tetras]
    volumes = np.abs(np.einsum('ij,ij->i', np.cross(p[:, 1]-p[:, 0], p[:, 2]-p[:, 0]), p[:, 3]-p[:, 0])) / 6
    if not np.isfinite(vertices).all() or not np.isfinite(volumes).all() or np.any(volumes == 0):
        raise ValueError('Non-finite or degenerate input')
    return vertices, tetras, volumes


def cell_boundary(tetras):
    faces = {}
    for t in tetras:
        for i in range(4):
            face = tuple(int(x) for j, x in enumerate(t) if j != i)
            key = tuple(sorted(face))
            if key in faces:
                del faces[key]
            else:
                faces[key] = (face, int(t[i]))
    return list(faces.values())


def kernel_measure(normals, offsets, dimension=3):
    # Inputs are normalized to unit scale before this function.
    a = np.column_stack((normals, np.ones(len(normals))))
    answer = linprog([0.] * dimension + [-1.], A_ub=a, b_ub=-offsets,
                     bounds=[(None, None)] * dimension + [(0., None)], method='highs',
                     options={'dual_feasibility_tolerance': 1e-9, 'primal_feasibility_tolerance': 1e-9,
                              'threads': 1})
    if answer.status == 2:
        # A checked nonnegative dual combination proves infeasibility within
        # the normalized cell box; a solver status alone does not certify it.
        certificate = certify_absence(normals, offsets)
        if certificate['certified']:
            return 'absent_confirmed', 0., None
        return 'indeterminate', None, None
    if not answer.success:
        return 'error', None, None
    if answer.x[-1] <= 1e-8:
        return 'indeterminate', None, None
    try:
        intersections = HalfspaceIntersection(np.column_stack((normals, offsets)), answer.x[:dimension]).intersections
        hull = ConvexHull(intersections)
        return 'present', float(hull.volume), intersections
    except (QhullError, ValueError, FloatingPointError):
        return 'indeterminate', None, None


def merged_faces(points, triangles, normals, offsets, merge_coplanar=True):
    # Adjacent coplanar triangles become one polygon. A hole or branching
    # boundary is explicitly unsupported and leaves merged rho undefined.
    edges = defaultdict(list)
    for i, tri in enumerate(triangles):
        for j in range(3):
            edges[tuple(sorted((int(tri[j]), int(tri[(j+1) % 3]))))].append(i)
    adjacent = defaultdict(list)
    for incident in edges.values():
        if merge_coplanar and len(incident) == 2:
            i, j = incident
            if np.linalg.norm(normals[i]-normals[j]) < 1e-9 and abs(offsets[i]-offsets[j]) < 1e-9:
                adjacent[i].append(j)
                adjacent[j].append(i)
    groups, seen = [], set()
    for seed in range(len(triangles)):
        if seed in seen:
            continue
        pending, group = [seed], []
        seen.add(seed)
        while pending:
            i = pending.pop()
            group.append(i)
            for j in adjacent[i]:
                if j not in seen:
                    seen.add(j)
                    pending.append(j)
        groups.append(group)
    result, unique_edges = [], set()
    for group in groups:
        counts = Counter(tuple(sorted((int(triangles[i][j]), int(triangles[i][(j+1) % 3])))) for i in group for j in range(3))
        boundary = [edge for edge, count in counts.items() if count == 1]
        links = defaultdict(list)
        for u, v in boundary:
            links[u].append(v); links[v].append(u)
        if not links or any(len(v) != 2 for v in links.values()):
            return None, unique_edges
        first = min(links)
        cycle, previous, current = [first], first, links[first][0]
        while current != first and current not in cycle:
            cycle.append(current)
            nxt = links[current][0] if links[current][0] != previous else links[current][1]
            previous, current = current, nxt
        if current != first or len(cycle) != len(links):
            return None, unique_edges
        # Remove collinear subdivision vertices, so lengths/counts describe
        # maximal geometric edges rather than TetGen boundary segmentation.
        changed = True
        while changed and len(cycle) > 3:
            changed = False
            for i in range(len(cycle)):
                a, b, c = points[cycle[i-1]], points[cycle[i]], points[cycle[(i+1) % len(cycle)]]
                u, v = b-a, c-b
                if np.linalg.norm(np.cross(u,v)) <= 1e-10*np.linalg.norm(u)*np.linalg.norm(v) and u@v > 0:
                    cycle.pop(i); changed = True; break
        boundary = [tuple(sorted((cycle[i],cycle[(i+1) % len(cycle)]))) for i in range(len(cycle))]
        unique_edges.update(boundary)
        n = normals[group[0]]
        basis = points[cycle[1]]-points[cycle[0]]
        basis /= np.linalg.norm(basis)
        second = np.cross(n, basis)
        planar = np.column_stack(((points[cycle]-points[cycle[0]]) @ basis,
                                  (points[cycle]-points[cycle[0]]) @ second))
        signed_area = .5 * np.sum(planar[:, 0]*np.roll(planar[:, 1], -1)-planar[:, 1]*np.roll(planar[:, 0], -1))
        if signed_area < 0:
            planar = planar[::-1]
        segment = np.roll(planar, -1, axis=0)-planar
        lengths = np.linalg.norm(segment, axis=1)
        face_normals = np.column_stack((segment[:, 1], -segment[:, 0])) / lengths[:, None]
        face_offsets = -np.einsum('ij,ij->i', face_normals, planar)
        if np.max(planar @ face_normals.T + face_offsets) <= 1e-10:
            state, area_kernel = 'present', abs(signed_area)
        else:
            state, area_kernel, _ = kernel_measure(face_normals, face_offsets, 2)
        area = abs(signed_area)
        diameter = float(pdist(planar).max())
        result.append({'area': area, 'diameter': diameter, 'min_edge': float(lengths.min()),
                       'edge_count': len(boundary), 'kernel_fraction': float(np.clip(area_kernel / area, 0, 1)) if area_kernel is not None else None,
                       'kernel_state': state})
    return result, unique_edges


def quality(vertices, tetras, volumes):
    ids = np.unique(tetras)
    cell_points = vertices[ids]
    origin = cell_points.mean(axis=0)
    scale = np.linalg.norm(np.ptp(cell_points, axis=0))
    points = (cell_points-origin) / scale
    local_tetras = np.searchsorted(ids, tetras)
    boundary = cell_boundary(local_tetras)
    triangles, normals, offsets, areas = [], [], [], []
    for face, opposite in boundary:
        tri = np.array(face)
        p = points[tri]
        normal = np.cross(p[1]-p[0], p[2]-p[0])
        norm = np.linalg.norm(normal)
        if normal @ (points[opposite]-p[0]) > 0:
            tri = tri[::-1]
            normal = -normal
        normal /= norm
        triangles.append(tri)
        normals.append(normal)
        offsets.append(-normal @ points[tri[0]])
        areas.append(norm / 2)
    triangles, normals, offsets, areas = map(np.array, (triangles, normals, offsets, areas))
    volume = float(volumes.sum())
    normalized_volume = volume / scale ** 3
    if len(tetras) == 1:
        # A nondegenerate tetrahedron is convex: its kernel and hull are itself.
        # This identity avoids unstable hull volume ratios on very thin tets.
        if exact_tetrahedron_orientation(vertices[tetras[0]])==0:
            raise ValueError('Degenerate tetrahedron confirmed by exact orientation')
        hull_area = float(areas.sum())
        state, kv = 'present', normalized_volume
    else:
        hull = ConvexHull((cell_points-origin) / scale)
        hull_area = float(hull.area)
        if np.max(points @ normals.T + offsets) <= 1e-10 and abs(hull.volume-normalized_volume) <= normalized_volume*1e-9:
            state, kv = 'present', float(hull.volume)
        elif np.max(offsets) < -1e-8:
            try:
                intersections = HalfspaceIntersection(np.column_stack((normals, offsets)), np.zeros(3)).intersections
                kv = float(ConvexHull(intersections).volume)
                state = 'present'
            except (QhullError, ValueError):
                state, kv, _ = kernel_measure(normals, offsets)
        else:
            state, kv, _ = kernel_measure(normals, offsets)
    fraction = kv / normalized_volume if kv is not None else None
    if fraction is not None and not (-1e-7 <= fraction <= 1 + 1e-7):
        state, fraction, kv = 'error', None, None
    if fraction is not None:
        fraction = float(np.clip(fraction, 0, 1))
    diameter = float(pdist((cell_points-origin) / scale).max())
    edges = set(tuple(sorted((int(t[i]), int(t[(i+1) % 3])))) for t in triangles for i in range(3))
    lengths = np.array([np.linalg.norm(points[a]-points[b]) for a, b in edges])
    face_lengths = np.array([[np.linalg.norm(points[t[i]]-points[t[(i+1) % 3]]) for i in range(3)] for t in triangles])
    face_h = face_lengths.max(axis=1)
    face_min = face_lengths.min(axis=1)
    rho2_face = np.minimum(np.sqrt(areas), face_min) / np.maximum(np.sqrt(areas), face_h)
    rho2 = .5 * (min(np.cbrt(normalized_volume), face_h.min()) / max(np.cbrt(normalized_volume), diameter) + rho2_face.mean())
    rho3 = .5 * (4 / len(triangles) + 1)
    merged, merged_edges = merged_faces(points, triangles, normals, offsets, merge_coplanar=len(tetras)>1)
    merged_rho1 = merged_rho2 = merged_rho3 = None
    merged_edge_ratio = None
    if merged is not None:
        if len(merged)<4:
            merged = None
    if merged is not None:
        lengths_merged = [np.linalg.norm(points[a]-points[b]) for a, b in merged_edges]
        merged_edge_ratio = min(lengths_merged) / max(lengths_merged)
        if fraction is not None and all(f['kernel_fraction'] is not None for f in merged):
            merged_rho1 = fraction * float(np.prod([f['kernel_fraction'] for f in merged]))
        merged_rho2 = .5 * (min(np.cbrt(normalized_volume), min(f['diameter'] for f in merged)) / max(np.cbrt(normalized_volume), diameter)
                           + np.mean([min(np.sqrt(f['area']), f['min_edge']) / max(np.sqrt(f['area']), f['diameter']) for f in merged]))
        merged_rho3 = .5 * (4 / len(merged) + np.mean([3 / f['edge_count'] for f in merged]))
    return {'tetrahedra': len(tetras), 'vertices': len(ids), 'volume': volume,
            'surface_area': float(areas.sum() * scale ** 2), 'kernel_state': state,
            'kernel_volume': kv * scale ** 3 if kv is not None else None, 'kernel_fraction': fraction,
            'volume_ratio_legacy': 1 / fraction if fraction is not None and fraction > 0 else None,
            'surface_ratio': float(areas.sum() / hull_area), 'edge_ratio_triangles': float(lengths.min()/lengths.max()),
            'faces_triangles': len(triangles), 'edges_triangles': len(edges),
            'faces_merged': len(merged) if merged is not None else None, 'edges_merged': len(merged_edges) if merged is not None else None,
            'min_face_to_cell_diameter_merged': min(f['diameter'] for f in merged)/diameter if merged is not None else None,
            'min_edge_to_face_diameter_merged': min(f['min_edge']/f['diameter'] for f in merged) if merged is not None else None,
            'edge_ratio_merged': merged_edge_ratio, 'min_face_to_cell_diameter': float(face_h.min()/diameter),
            'min_edge_to_face_diameter': float(np.min(face_min / face_h)),
            'rho1_triangles': fraction, 'rho2_triangles': float(rho2), 'rho3_triangles': float(rho3),
            'rho1_merged': merged_rho1, 'rho2_merged': merged_rho2, 'rho3_merged': merged_rho3}


def evaluate(node, ele, membership, output):
    vertices, tetras, volumes = load_mesh(node, ele)
    owner = np.loadtxt(membership, dtype=np.int64, ndmin=1)
    if len(owner) != len(tetras) or np.any(owner < 0):
        raise ValueError('Invalid membership coverage')
    order = np.argsort(owner, kind='stable')
    split = np.flatnonzero(np.diff(owner[order])) + 1
    details, failures = [], []
    for ids in np.split(order, split):
        try:
            row = quality(vertices, tetras[ids], volumes[ids])
        except Exception as e:
            row = {'kernel_state': 'error', 'tetrahedra': len(ids), 'error': f'{type(e).__name__}: {e}'}
            failures.append({'cell': int(owner[ids[0]]), 'cause': row['error']})
        row['cell'] = int(owner[ids[0]])
        details.append(row)
    fields = list(dict.fromkeys(k for row in details for k in row))
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.with_suffix('.details.csv').open('w') as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader(); writer.writerows(details)
    states = Counter(row['kernel_state'] for row in details)
    n = len(details)
    report = {'vertices': len(vertices), 'tetrahedra': len(tetras), 'cells': n,
              'reduction': 1 - n / len(tetras), 'isolated': sum(len(ids) == 1 for ids in np.split(order, split)),
              'kernel_states': {state: states[state] for state in ('present', 'absent_confirmed', 'indeterminate', 'error')},
              'kernel_fraction_lower': states['present']/n,
              'kernel_fraction_upper': 1-states['absent_confirmed']/n,
              'failures': failures, 'metrics': {}}
    for field in fields:
        values = [row[field] for row in details if isinstance(row.get(field), (float, int)) and np.isfinite(row[field])]
        if values:
            report['metrics'][field] = {**dict(zip(['min', 'p5', 'median', 'p95', 'max'], map(float, np.quantile(values, [0, .05, .5, .95, 1])))), 'valid_count': len(values)}
    for suffix in ('triangles', 'merged'):
        terms = [row[f'rho1_{suffix}'] * (row[f'rho2_{suffix}']+row[f'rho3_{suffix}'])/2 for row in details
                 if all(row.get(f'rho{i}_{suffix}') is not None for i in (1, 2, 3))]
        report[f'rho_{suffix}'] = float(np.sqrt(np.mean(terms))) if terms else None
        report[f'rho_{suffix}_valid_cells'] = len(terms)
    output.with_suffix('.quality.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    for name in ('node', 'ele', 'membership', 'out'):
        p.add_argument('--'+name, required=True)
    a = p.parse_args()
    print(json.dumps(evaluate(a.node, a.ele, a.membership, a.out), indent=2))
