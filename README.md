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
- `-y / --year`: define which year to run on. Can also be multiple years separated by spaces.
- `-w / --working_points`: path to a json file with working points. See [here](https://github.com/LukaLambrecht/ParticleNetSF/blob/ParticleNet_TopW_SFs_NanoV9/wps/wps_top.json) for an example of correct formatting.

Further settings are also defined in `configuration.h`.
For example:
- the branch name of the ParticleNet score to use
- whether to consider the top-merged or W-merged category as main process of interest in the fit
- the binning in pT
Make sure to check and modify as needed before running `runFullChain.py`.

### Create 2D histograms
To speed up the process, steps can be run separately instead of in one go.
The first step consists of making 2D histograms (as a function of m(jet) and pt(jet)) from the trees.
```
root [0] .L make2DTemplates.C
root [1] make2DTemplates("<year>", "tt1l", "<minimum score>", "<maximum score", "<output directory>")
```
The resulting histograms are stored under `<chosen output directory>/templates2D`.

### Create 1D histograms:
Next, the 2D histograms are projected onto 1D histograms in given bins of pt.
```
root [0] .L make1DTemplates.C
root [1] make1DTemplates("<year>", "tt1l", "<minimum score>", "<maximum score>", false, "", "<output directory>")
```
Where the 5th argument (`false`) is for prefit and the 6th (`""`) is ignored in prefit mode.
The version in postfit mode is called automatically from `makeFits.C` after doing the fit (see below).

The chosen output directory should be the same one as in the previous step in order to find the correct input files.
The resulting histograms are stored under `<output directory>/templates1D`.

### Create the datacards:
Next, the datacards will be produced.
```
root [0] .L makeDatacards.C
root [1] makeDatacards("<year>", "tt1l", "<minimum score>", "<maximum score>", "<output directory>")
```

The chosen output directory should be the same one as in the previous steps in order to find the correct input files.
The resulting datacards are stored under `<output directory>/templates1D/fitdir`.

### Do the fit:
Finally, the fits can be performed.
```
root [0] .L makeFits.C
root [1] makeFits("<year>", "tt1l", "minimum_score", "maximum_score", "<output directory>")
```

The chosen output directory should be the same one as in the previous steps in order to find the correct input files.
Most of the fit results are stored under `<output directory>/templates1D/fitdir`,
except for the postfit plots, which are stored under `<output directory>/templates1D/postfit_plots`.

## Making modifications
This paragraph lists the files and items that should be modified for some common changes.

To change the tagger being used:
- Modify `configuration.h`. In particular:
  - `score_def` (branch name of the score to use)
  - if needed, also change `score_category` to switch between top-merged and W-merged

To modify the binning in pt
- Modify `configuration.h`. In particular:
  - binsY, minY, maxY (this is used for producing the 2D histograms)
  - pT range (this is used for projecting the 1D histograms)

To select different working points of the tagger:
- Modify the `json` files in the `wps` folder, or create new ones
  (and provide them as command line argument when running `runFullChain.py`)

To use different selections or matching criteria:
- Modify `make2DTemplates.C`, in particular the definition of the cuts and criteria in the function `makeTemplatesTop`.
