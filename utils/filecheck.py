#!/usr/bin/env python3

######################################
# Check the presence of output files #
######################################
# Use case:
# Get a quick overview of which jobs were successfull,
# and which ones are still running or failed,
# by simply checking which output files are present.

# Note: for now limited to 2D and 1D template production;
# datacard production and fitting are not included in this check.
# Update: also check postfit plots.

# Note: the contents of the files is not checked,
# just the presence of files with the expected name.


import sys
import os
import argparse
import glob


def get_by_field(infolist, field):
    res = {}
    fieldvalues = list(set([el[field] for el in infolist]))
    for fieldvalue in fieldvalues:
        res[fieldvalue] = [el for el in infolist if el[field]==fieldvalue]
    return res


def count_by_field(infolist, field):
    by_field = get_by_field(infolist, field)
    res = {key: len(val) for key, val in by_field.items()}


if __name__=='__main__':

    # parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--dir', default=os.getcwd())
    args = parser.parse_args()

    # print arguments
    print('running with following configuration:')
    for arg in vars(args):
        print('  - {}: {}'.format(arg,getattr(args,arg)))

    # find all 2D template files
    template2ddir = os.path.join(args.dir, 'templates2D')
    t2dfiles = sorted(glob.glob(os.path.join(template2ddir, '*.root')))
    t2dinfo = []
    for f in t2dfiles:
        parts = os.path.basename(f).replace('.root', '').split('_')
        wp = parts[2]
        year = parts[3]
        pt = parts[4]
        info = {'file': f, 'wp': wp, 'year': year, 'pt': pt}
        t2dinfo.append(info)

    # printouts
    print(f'Found {len(t2dinfo)} 2D template files.')
    for field in ['wp', 'year', 'pt']:
        print(f'Grouped by {field}:')
        by_field = get_by_field(t2dinfo, field)
        for key, val in by_field.items(): print(f'  - {key}: {len(val)}')
    print('----------')

    # find all 1D template files
    template1ddir = os.path.join(args.dir, 'templates1D')
    t1dfiles = sorted(glob.glob(os.path.join(template1ddir, '*.root')))
    t1dinfo = []
    for f in t1dfiles:
        parts = os.path.basename(f).replace('.root', '').split('_')
        wp = parts[2]
        year = parts[3]
        pt = parts[4]
        cat = parts[6]
        info = {'file': f, 'wp': wp, 'year': year, 'pt': pt, 'cat': cat}
        t1dinfo.append(info)

    # printouts
    print(f'Found {len(t1dinfo)} 1D template files.')
    for field in ['wp', 'year', 'pt', 'cat']:
        print(f'Grouped by {field}:')
        by_field = get_by_field(t1dinfo, field)
        for key, val in by_field.items(): print(f'  - {key}: {len(val)}')
    print('----------')

    # find all postfit plots
    postfitplotdir = os.path.join(args.dir, 'templates1D/plots_postfit')
    pffiles = sorted(glob.glob(os.path.join(template1ddir, '*.png')))
    pfinfo = []
    for f in pffiles:
        parts = os.path.basename(f).replace('.png', '').split('_')
        wp = parts[2]
        year = parts[3]
        pt = parts[4]
        cat = parts[5]
        info = {'file': f, 'wp': wp, 'year': year, 'pt': pt, 'cat': cat}
        pfinfo.append(info)

    # printouts
    print(f'Found {len(pfinfo)} postfit files.')
    for field in ['wp', 'year', 'pt', 'cat']:
        print(f'Grouped by {field}:')
        by_field = get_by_field(pfinfo, field)
        for key, val in by_field.items(): print(f'  - {key}: {len(val)}')
    print('----------')
