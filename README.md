# Jet mass scale and resolution factors for ParticleNet jets

## How to install

### Install Combine and CombineHarvester
This repository uses the Combine and CombineHarvester packages.
To install them, follow the instructions given on the [Combine website](https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/latest/#within-cmssw-recommended-for-cms-users).

At the time of writing, installation can be done as follows:
```
# install a recent CMSSW version
cmsrel CMSSW_14_1_0_pre4
cd CMSSW_14_1_0_pre4/src
cmsenv
# clone and build the Combine repository
# (make sure you are in the CMSSW_X_X_X/src directory!)
git clone https://github.com/cms-analysis/HiggsAnalysis-CombinedLimit.git HiggsAnalysis/CombinedLimit
cd HiggsAnalysis/CombinedLimit
git fetch origin
git checkout v10.2.1
scramv1 b clean; scramv1 b
cd ../..
# clone and build the CombineHarvester repository
# (make sure you are in the CMSSW_X_X_X/src directory!)
git clone https://github.com/cms-analysis/CombineHarvester.git CombineHarvester
scram b
```
But details might differ in the future - please refer to the official instructions linked above for the latest recommendations.

Notes:
- Apparently there are some compilation/build warnings for CombineHarvester. These are fine to ignore though.
- The steps above assume installation on `lxplus`. Other systems should also be fine as long as the environment is set correctly.
  For example, on some T2/T3's, one might need to do `source /cvmfs/cms.cern.ch/cmsset_default.sh` first for the `cmsenv` and `cmsrel` commands to be available.

### Install this repository
Go back to your home directory (or wherever you want to install this, doesn't need to be inside the CMSSW directory from the previous step).
Then run the following:
```
git clone -b ParticleNet_TopW_SFs_NanoV9 https://github.com/LukaLambrecht/ParticleNetSF.git
cd ParticleNetSF/
cp TagAndProbeExtended.py $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/python
```

## Input files
To do: a discussion on suitable input files: where to find them or how to produce them.

For now, just use the ones prepared here (on `lxplus`): `/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_<year>/`.

## How to run

### Full chain
Use the script `runFullChain.py` to the full chain, from input files to scale factors, in one go.
Use `python3 runFullChain.py -h` to see a list of all available options and a short explanation for each of them.
The required arguments are:
- `-c / --category`: define which tagger to use. Choose from `top` or `W`.
- `-v / --version`: define which version of the tagger to use. Choose from `nominal` or `MD` (mass-decorrelated).
- `-y / --year`: define which year to run on. Can also be multiple years separated by spaces.

Notes:
- Apart from providing command line arguments, small tweaks might be needed inside the "configuration.h" and "makeFits.C",
  according to the category / version selected for the SF extraction (see also further below).
  This is in the process of being streamlined, so that no additional tweaks are needed, but not yet completed.

### Create 2D histograms
To speed up the process, steps can be run separately instead of in one go.
The first step consists of making 2D histograms (as a function of m(jet) and pt(jet)) from the trees.
```
root [0] .L make2DTemplates.C
root [1] make2DTemplates("<year>", "tt1l", "<category>", "<minimum score>", "<maximum score>”)
```

### Create 1D histograms:
Next, the 2D histograms are projected onto 1D histograms in given bins of pt.
```
root [0] .L make1DTemplates.C
root [1] make1DTemplates("<year>", "tt1l", "<category>", "<minimum score>", "<maximum score>", false, "", "")
```
Where the last three arguments are for prefit and two dummy arguments that are not used (in prefit mode), respectively.
The version in postfit mode is called automatically from `makeFits.C` after doing the fit (see below).

### Create the datacards:
Next, the datacards will be produced.
```
root [0] .L makeDatacards.C
root [1] makeDatacards("<year>", "tt1l", "<category>", "<minimum score>", "<maximum score>")
```

### Do the fit:
Finally, the fits can be performed.
```
root [0] .L makeFits.C
root [1] makeFits("<year>", "<category>", "minimum_score", "maximum_score", "tt1l")
```

## Making modifications
This paragraph lists the files and items that should be modified for some common changes.
This is likely to be updated (for example for running on 2016 pre-VFP and post-VFP without having to change elements in the source code),
but for now these are the steps to follow.

To change the tagger being used:
- Use the correct command line arguments to `runFullChain.py`. Valid choices are:
  - `top nominal`
  - `W nominal`
  - `W MD`.
- Modify `configuration.h`. In particular:
  - algo
  - score_def

To modify the binning in pt
- Modify `configuration.h`. In particular:
  - binsY, minY, maxY (this is used for producing the 2D histograms)
  - pT range (this is used for projecting the 1D histograms)

To select different working points of the tagger:
- Modify the `json` files in the `wps` folder, or create new ones
  (and provide them as command line argument when running `runFullChain.py`)

To use different selections or matching criteria:
- Modify `make2DTemplates.C`, in particular the definition of the cuts and criteria in the function `makeTemplatesTop`.
