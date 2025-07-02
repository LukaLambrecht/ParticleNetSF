#!/usr/bin/env python 

# Run the full scale factor measurement chain

# import external modules
import os
import sys
import six
import json
import argparse

# import local modules
topdir = os.path.dirname(__file__)
sys.path.append(topdir)
import jobtools.condortools as ct


# read command-line args
# todo: add optional arguments for more flexibility
parser = argparse.ArgumentParser()
parser.add_argument('-c', '--category', required=True, choices=['top', 'W'])
parser.add_argument('-v', '--version', required=True, choices=['nominal', 'MD'])
parser.add_argument('-y', '--year', required=True, nargs='+',
  help='Data-taking year (note that multiple can be provided, separated by spaces).')
parser.add_argument('--wps', default='auto',
  help='Path to json file with working point definitions'
      +' (default: automatically determine correct file based on category and version)')
parser.add_argument('-r', '--runmode', default='interactive', choices=['interactive', 'condor'],
  help='Run in terminal or in condor job submission')
parser.add_argument('--cmssw', default='auto',
  help='Path to CMSSW installation (only used in job submission; needed to set the environment)')
parser.add_argument('--jobdir', default='auto',
  help='Define path where job log files should appear (only used in job submission)')
args = parser.parse_args()

# parse working point json file
if args.wps=='auto':
    args.wps = os.path.join('wps', f'wps_{args.category.lower()}_{args.version.lower()}.json')
    print(f'Found working point file {args.wps}')

# check working point json file
if not os.path.exists(args.wps):
    msg = f'Json file with working point definitions {args.wps} does not exist.'
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
with open(args.wps, 'r') as f:
    wps = json.load(f)
print(f'Read the following working points from {args.wps}:')
print(json.dumps(wps, indent=2))

# check if all requested years are present in the working point dict
for year in args.year:
    if year not in wps.keys():
        msg = f'Year {year} not found in {args.wps}.'
        raise Exception(msg)

# loop over years and mistag rates
jobs = []
for year in args.year:
   for mistag_rate, wp in wps[year].items():
      
      # make the commands to run
      cmds = []
      cmds.append( f'make2DTemplates.C("{year}", "tt1l", "{wp}", "1.00")' )
      cmds.append( f'make1DTemplates.C("{year}", "tt1l", "{wp}", "1.00", false, "")' )
      cmds.append( f'makeDatacards.C("{year}", "tt1l", "{wp}", "1.00")' )
      cmds.append( f'makeFits.C("{year}", "{args.category.lower()}", "{wp}", "1.00", "tt1l")' )
     
      for idx, cmd in enumerate(cmds):
          cmds[idx] = f'root -l -q \'{cmd}\''
          # todo: make alternative for the "| tee ${args.category}_${year}_${args.version}_${wp}.txt')" part of the last command

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
