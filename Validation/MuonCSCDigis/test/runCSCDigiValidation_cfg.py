import FWCore.ParameterSet.Config as cms

from Configuration.Eras.Era_Run3_cff import Run3

process = cms.Process('TEST',Run3)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load("Validation.MuonCSCDigis.cscDigiValidation_cfi")
process.load('Configuration.StandardSequences.DQMSaverAtRunEnd_cff')
process.load('DQMOffline.Configuration.DQMOfflineMC_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)

# Input source
process.source = cms.Source(
    "PoolSource",
    fileNames = cms.untracked.vstring(
        "/store/relval/CMSSW_11_2_0_pre7/RelValSingleMuPt10/GEN-SIM-DIGI-RAW/112X_mcRun3_2021_realistic_v8-v1/20000/0ED98457-2CEC-924D-AAFC-4F3F705C2DCC.root"
        #"/store/relval/CMSSW_11_2_0_pre9/RelValSingleMuPt100/GEN-SIM-DIGI-RAW/112X_mcRun3_2021_realistic_v11-v1/00000/1e211854-2fbf-4ab7-864d-9ee3cba14035.root",
        #"/store/relval/CMSSW_11_2_0_pre9/RelValSingleMuPt100/GEN-SIM-DIGI-RAW/112X_mcRun3_2021_realistic_v11-v1/00000/92533d1f-b029-4f45-a602-ca671852a0ba.root"
        #"/store/relval/CMSSW_11_2_0_pre11/RelValSingleMuPt100/GEN-SIM-DIGI-RAW/112X_mcRun3_2021_realistic_v13-v1/00000/32b157a1-5eb8-47ad-9c29-053ab0efd3b7.root"
        #"/store/relval/CMSSW_11_2_0_pre9/RelValSingleMuPt100/GEN-SIM-DIGI-RAW/112X_mcRun3_2021_realistic_v11-v1/00000/92533d1f-b029-4f45-a602-ca671852a0ba.root"
    ),
    secondaryFileNames = cms.untracked.vstring()
)

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')

process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('DQMIO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:step3_inDQM.root'),
    outputCommands = process.DQMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

# Path and EndPath definitions
process.validation_step = cms.Path(process.mix * process.cscDigiValidation)
#process.dqmsave_step = cms.Path(process.DQMSaver)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)

# Schedule definition
process.schedule = cms.Schedule(
    process.validation_step,
    process.endjob_step,
    process.DQMoutput_step
    #process.dqmsave_step
)
