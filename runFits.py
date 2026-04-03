#!/usr/bin/env python 

# Run the fits.
# This is similar to runFullChain.py, but limited to the fit (including datacard production).
# This allows for finer job splitting for higher parallellization
# (for example: different pt bins in different jobs).

import os
import sys
import six
import json
import argparse

# import local modules
topdir = os.path.dirname(__file__)
sys.path.append(topdir)
import jobtools.condortools as ct


def get_ptnames_from_config(cffile='configuration.h'):
    '''
    Get available pt names from the configuration file.
    This is a bit of a hack, reading a .h file in python as a plain txt file
    and manually parsing the content, but ok for now.
    '''

    # read config file as plain text
    with open(cffile, 'r') as f:
        lines = [line.strip(' \t\n') for line in f.readlines()]

    # look for lines defining the ptnames
    lines = [line for line in lines if line.startswith('ptnames.push_back(')]

    # exctract ptnames from those lines
    ptnames = []
    for line in lines:
        process = line.split('(', 1)[1].split(')', 1)[0].strip('\'"')
        ptnames.append(process)
    return ptnames


# read command-line args
# todo: add optional arguments for more flexibility
parser = argparse.ArgumentParser()
parser.add_argument('-y', '--year', required=True, nargs='+',
  help='Data-taking year (note that multiple can be provided, separated by spaces).')
parser.add_argument('-w', '--working_points', required=True,
  help='Path to json file with working point definitions.')
parser.add_argument('-o', '--outputdir', default='output', type=os.path.abspath,
  help='Define path where output should appear (default: "output").')
parser.add_argument('-p', '--ptnames', default=['all'], nargs='+',
  help='Pt bins to run (note that multiple can be provided, separated by spaces).'
      +' See configuration.h options.'
      +' Use "all" to run all available pt bins defined in configuration.h (+ data).')
parser.add_argument('-r', '--runmode', default='interactive', choices=['interactive', 'condor'],
  help='Run in terminal or in condor job submission.')
parser.add_argument('--cmssw', default='auto',
  help='Path to CMSSW installation (only used in job submission; needed to set the environment).')
parser.add_argument('--jobdir', default='auto',
  help='Define path where job log files should appear (only used in job submission).')
args = parser.parse_args()

# check working point json file
if not os.path.exists(args.working_points):
    msg = f'Json file with working point definitions {args.working_points} does not exist.'
    raise Exception(msg)

# parse CMSSW version
if args.runmode!='interactive' and args.cmssw=='auto':
    if not 'CMSSW_BASE' in os.environ.keys():
        msg = 'Cannot determine CMSSW version automatically,'
        msg += ' as it is not set in the current environment.'
        msg += ' Run cmsenv in the correct CMSSW first,'
        msg += ' or provide the path explicitly.'
        raise Exception(msg)
    args.cmssw = os.environ['CMSSW_BASE']
    print(f'Found CMSSW version {args.cmssw}')

# parse job directory
if args.runmode!='interactive' and args.jobdir=='auto':
    user = os.environ['USER']
    initial = user[0]
    args.jobdir = f'/afs/cern.ch/user/{initial}/{user}/job_log_particlenetsf'
    # (must be on /afs as submission from /eos is not allowed)
    print(f'Set job directory to {args.jobdir}')

# read working point json file
with open(args.working_points, 'r') as f:
    working_points = json.load(f)
print(f'Read the following working points from {args.working_points}:')
print(json.dumps(working_points, indent=2))

# parse years
if 'all' in args.year:
    args.year = list(working_points.keys())

# check if all requested years are present in the working point dict
for year in args.year:
    if year not in working_points.keys():
        msg = f'Year {year} not found in {args.working_points}.'
        raise Exception(msg)

# parse ptnames
all_ptnames = get_ptnames_from_config()
if 'all' in args.ptnames: args.ptnames = all_ptnames[:]

# check if all requested ptnames are available
for ptname in args.ptnames:
    if ptname not in all_ptnames:
        msg = f'Pt bin {ptname} not found in configuration.'
        msg += f' Options are {all_ptnames}.'
        raise Exception(msg)

# make output directory and copy auxiliary files
if not os.path.exists(args.outputdir): os.makedirs(args.outputdir)
os.system(f'cp configuration.h {args.outputdir}')
os.system(f'cp {args.working_points} {args.outputdir}')

# loop over years and mistag rates and pt bins
jobs = []
for year in args.year:
    for mistag_rate, wp in working_points[year].items():
        for ptname in args.ptnames:
      
            # make the commands to run
            cmds = []
            cmds.append( f'makeDatacards.C("{year}", "tt1l", "{ptname}", "{wp}", "1.00", "{args.outputdir}")' )
            cmds.append( f'makeFits.C("{year}", "tt1l", "{ptname}", "{wp}", "1.00", "{args.outputdir}")' )
     
            for idx, cmd in enumerate(cmds):
                cmds[idx] = f'root -l -q \'{cmd}\''

            jobs.append(cmds)

# run jobs interactively
if args.runmode=='interactive':
    for job in jobs:
        for cmd in job:
            print(f'Now running {cmd}')
            os.system(cmd)

# submit jobs to condor
elif args.runmode=='condor':

    # ask for confirmation
    print(f'Will submit {len(jobs)} jobs. Continue? (y/n)')
    go = six.moves.input()
    if not go=='y': sys.exit()

    # settings
    if not os.path.exists(args.jobdir): os.makedirs(args.jobdir)
    name = os.path.join(args.jobdir, 'cjob_particlenetsf')

    # submit jobs
    ct.submitCommandsAsCondorJobs(name, jobs,
          cmssw_version=args.cmssw, jobflavour='workday')
