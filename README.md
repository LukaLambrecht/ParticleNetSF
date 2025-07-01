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
For now, just use the ones prepared here (on `lxplus`): `/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2018/`

## How to run

### Full chain
This command runs the full chain, from input files to scale factors, in one go:
```
./runFullChain.sh  [T|W]  [2016|2017|2018]  ["Nominal"|"MD"]
```

Notes:
- Small tweaks might be needed inside the "configuration.h" and "makeFits.C" according to the object/version selected for the SF extraction.


### Create 2D histograms pT(jet) vs m(jet) from the trees
To speed up the process, steps can be run separately instead of in one go.
```
root [0] .L make2DTemplates.C
root [1] mainfunction("tt1l","2017","0.90","1.”)
```

### Create the 1D templates:

```
root [0] .L HeavyFlavourZCandleStudies.C
root [1] HeavyFlavourZCandleStudies("2017","tt1l","bb","0.90","1.",false,"pass")
```

### Create the datacards:
The datacards will be produced under particlenet_sf/fitdir (if one keeps the default naming/settings)

```
root [0] .L makeDatacards.C
root [1] makeDatacards("2017","tt1l","bb","0.90","1.")
```


## Do the fit:

```
root [0] .L makeFits.C
root [1] makeFits("2017","bb","0.90","1.","tt1l")
```

 or “zqq” or “tt1L” to fit only specific samples
