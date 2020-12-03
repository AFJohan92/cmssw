#Below are instructions to run CSCStubEfficiencyValidation
#Essentially they are what runTheMatrix.py -l 23634.0 does but manually to change the N events processed.
#First, cd into ~/workdir/CMSSW11_0_pre4/src/Validation/MuonCSCDigis/src/
#Then, make a working directory
mkdir 23634.0_TTbar_14TeV+2026D51+TTbar_14TeV_TuneCP5_GenSimHLBeamSpot14+DigiTrigger+RecoGlobal+HARVESTGlobal
#Run step1 (this takes a while)
#If you want to run over N events change the argument os -n
cmsDriver.py TTbar_14TeV_TuneCP5_cfi  --conditions auto:phase2_realistic_T15 -n 1000 --era Phase2C9 --eventcontent FEVTDEBUG --relval 9000,100 -s GEN,SIM --datatier GEN-SIM --beamspot HLLHC14TeV --geometry Extended2026D51 --fileout file:step1.root  > step1_TTbar_14TeV+2026D51+TTbar_14TeV_TuneCP5_GenSimHLBeamSpot14+DigiTrigger+RecoGlobal+HARVESTGlobal.log  2>&1
#Run step2 (also takes a while)
cmsDriver.py step2  --conditions auto:phase2_realistic_T15 -s DIGI:pdigi_valid,L1TrackTrigger,L1,DIGI2RAW,HLT:@fake2 --datatier GEN-SIM-DIGI-RAW -n 1000 --geometry Extended2026D51 --era Phase2C9 --eventcontent FEVTDEBUGHLT --filein  file:step1.root  --fileout file:step2.root  > step2_TTbar_14TeV+2026D51+TTbar_14TeV_TuneCP5_GenSimHLBeamSpot14+DigiTrigger+RecoGlobal+HARVESTGlobal.log  2>&1
#Run step3 (thankfully faster)
cmsDriver.py step3  --conditions auto:phase2_realistic_T15 -s RAW2DIGI,L1Reco,RECO,RECOSIM,PAT,VALIDATION:@phase2Validation+@miniAODValidation,DQM:@phase2+@miniAODDQM --datatier GEN-SIM-RECO,MINIAODSIM,DQMIO -n 1000 --geometry Extended2026D51 --era Phase2C9 --eventcontent FEVTDEBUGHLT,MINIAODSIM,DQM --filein  file:step2.root  --fileout file:step3.root  > step3_TTbar_14TeV+2026D51+TTbar_14TeV_TuneCP5_GenSimHLBeamSpot14+DigiTrigger+RecoGlobal+HARVESTGlobal.log  2>&1
#Last is step4
cmsDriver.py step4  --conditions auto:phase2_realistic_T15 -s HARVESTING:@phase2Validation+@phase2+@miniAODValidation+@miniAODDQM --scenario pp --filetype DQM --geometry Extended2026D51 --era Phase2C9 --mc  -n 1000  --filein file:step3_inDQM.root --fileout file:step4.root  > step4_TTbar_14TeV+2026D51+TTbar_14TeV_TuneCP5_GenSimHLBeamSpot14+DigiTrigger+RecoGlobal+HARVESTGlobal.log  2>&1
