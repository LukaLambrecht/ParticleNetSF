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

### From working points to score ranges
The values in the `json` files are single score thresholds.
The steps in the chain work with score ranges (the "pass" region being `min < score <= max`),
so the thresholds are translated into ranges by `wptools.py`,
in one of two ways, steered by the `--wp_mode` argument of the run scripts:
- `exclusive` (default): orthogonal ranges, from each threshold to the next higher one,
  e.g. `{"LP": 0.58, "MP": 0.78, "HP": 0.93}` gives 0.58 - 0.78, 0.78 - 0.93 and 0.93 - 1.00.
- `inclusive`: nested ranges, from each threshold to 1.00,
  e.g. the same working points give 0.58 - 1.00, 0.78 - 1.00 and 0.93 - 1.00.

A score range can also be written explicitly in the `json` file as a `[min, max]` pair,
e.g. `{"LP": [0.58, 0.78]}`; such ranges are used as they are, whatever the `--wp_mode`.

Note that the "fail" region is always the complement of the "pass" region.
For exclusive working points, it therefore also contains the jets with a score above the upper edge.
