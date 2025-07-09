# Small utility script to compare templates in two different ROOT files.

# Use case: synchronization studies (e.g. before and after making a change).


import os
import sys
import uproot
import numpy as np


if __name__=='__main__':

    # read command line args
    # (maybe extend later)
    files = sys.argv[1:]

    # read all histograms from all files
    print('Reading files...')
    histograms = {}
    for file in files:
        with uproot.open(file) as f:
            hist_names = sorted(list(f.classnames().keys()))
            hists = {}
            for hist_name in hist_names:
                hists[hist_name] = (f[hist_name].values(), f[hist_name].errors())
            histograms[file] = hists

    # compare histogram names
    print('Comparing histogram names...')
    ref_hist_names = list(histograms[files[0]].keys())
    for testfile in files[1:]:
        test_hist_names = list(histograms[testfile].keys())
        if test_hist_names == ref_hist_names: continue
        extra = []
        missing = []
        for hist_name in test_hist_names:
            if hist_name not in ref_hist_names: extra.append(hist_name)
        for hist_name in ref_hist_names:
            if hist_name not in test_hist_names: missing.append(hist_name)
        print(f'  Found mismatch in file {files[testidx]} (compared to {files[0]})')
        print(f'    - extra: {extra}')
        print(f'    - missing: {missing}')

    # compare histogram contents
    print('Comparing histogram content...')
    ref_hists = histograms[files[0]]
    ntot = 0
    diff = []
    for hist_name, ref_hist in ref_hists.items():
        for testfile in files[1:]:
            if hist_name not in histograms[file].keys(): continue
            test_hist = histograms[file][hist_name]
            ref_values = ref_hist[0]
            ref_errors = ref_hist[1]
            test_values = test_hist[0]
            test_errors = test_hist[1]
            values_diff = np.sqrt(np.sum(np.square(ref_values - test_values)))
            errors_diff = np.sqrt(np.sum(np.square(ref_errors - test_errors)))
            if values_diff > 1e-12 or errors_diff > 1e-12: diff.append(hist_name)
            ntot += 1
    print(f'Checked {ntot} histograms, found {len(diff)} different:')
    print(diff)
