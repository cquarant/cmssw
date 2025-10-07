Introduction to CMSSW: http://cms-sw.github.io

# Clone in your local lxplus
```
cmssw-el8 # CMSSW 15.1.0 based on lxplus8
cmsrel CMSSW_15_1_0_pre4
cd CMSSW_15_1_0_pre4/src
cmsenv
git cms-checkout-topic cquarant:cq_btldigisoa_20251006 #if errors, to git cms-init first
scram b -j 8
```

# Produce small sample for testing
Example uses a sample of 10 SingleMu Pt 10, with Run4D121 geometry (include latest BTL Geometry v4)
```
runTheMatrix.py --what upgrade -l 34407.0 --nEvents 10
```
# Dump BTLDigi
```
cd SimFastTiming/FastTimingCommon/test/
```
Edit runMTDDigiDump.py specifying the path of the step2.root output file
```
cmsRun runMTDDigiDump.py
```
