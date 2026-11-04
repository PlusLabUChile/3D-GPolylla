"""Numerically checked dual certificate for empty cell/face kernels.

Normalized physical kernels lie in [-1,1]^d. For any nonnegative weights w,
feasibility of n_i.x+b_i <= 0 implies (sum w*n_i).x+sum w*b_i <= 0.
Thus sum(w*b)-||sum(w*n)||_1 > 0 contradicts feasibility in that box.
The returned margin subtracts a conservative floating point accumulation bound.
"""
import numpy as np
from scipy.optimize import linprog


def certify_absence(normals,offsets,tolerance=1e-8):
    normals=np.asarray(normals,dtype=float);offsets=np.asarray(offsets,dtype=float)
    dimension=normals.shape[1]
    result=linprog([0.]*dimension+[-1.],A_ub=np.column_stack((normals,np.ones(len(normals)))),b_ub=-offsets,
                   bounds=[(-1.,1.)]*dimension+[(None,None)],method='highs',
                   options={'primal_feasibility_tolerance':1e-9,'dual_feasibility_tolerance':1e-9,'threads':1})
    record={'certified':False,'solver_status':int(result.status),'box_bound':1.,'tolerance':tolerance}
    if not result.success:return record
    weights=np.maximum(0.,-np.asarray(result.ineqlin.marginals,dtype=float))
    if not np.isfinite(weights).all() or weights.sum()<=0:return record
    weights/=weights.sum()
    residual=weights@normals
    weighted_offset=float(weights@offsets)
    accumulation_scale=float(weights@np.abs(offsets)+weights@np.sum(np.abs(normals),axis=1))
    roundoff=32*np.finfo(float).eps*(len(weights)+dimension+1)*max(1.,accumulation_scale)
    margin=weighted_offset-float(np.abs(residual).sum())-roundoff
    record.update(certified=bool(margin>tolerance),margin=margin,weighted_offset=weighted_offset,
                  dual_residual_l1=float(np.abs(residual).sum()),roundoff_bound=roundoff,
                  signed_radius=float(result.x[-1]),weights=weights.tolist())
    return record


def exact_tetrahedron_orientation(points):
    """Orientation sign of the input binary64 coordinates, using exact rationals."""
    from fractions import Fraction
    p=[[Fraction.from_float(float(value)) for value in point] for point in points]
    a,b,c=[[p[i][j]-p[0][j] for j in range(3)] for i in (1,2,3)]
    determinant=(a[0]*(b[1]*c[2]-b[2]*c[1])-a[1]*(b[0]*c[2]-b[2]*c[0])+a[2]*(b[0]*c[1]-b[1]*c[0]))
    return (determinant>0)-(determinant<0)
