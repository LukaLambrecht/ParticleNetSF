# Definition of working points

### Older iterations: ParticleNet
To do: find out where they are coming from.

### Current: ParticleTransformer
Working points obtained from VH analyzers (to double check in AN):
```
ParT_HbbVsQCD: medium purity: 0.85, high purity: 0.95
ParT_HccVsQCD: medium purity: 0.80, high purity: 0.923
```
(same for all years).

However, the working points used here are somewhat arbitrary.
Ideally we scan over a range of reasonable working points
and see that the results don't change very much.
So the values in `wps_w_part.json` are inspired by these values,
but are not strictly bound to remain the same.
