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
process.load("Validation.MuonCSCDigis.PostProcessor_cff")
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)

# Input source
process.source = cms.Source(
    "PoolSource",
fileNames = cms.untracked.vstring(
    "/store/relval/CMSSW_11_2_0_pre7/RelValSingleMuPt10/GEN-SIM-DIGI-RAW/112X_mcRun3_2021_realistic_v8-v1/20000/0ED98457-2CEC-924D-AAFC-4F3F705C2DCC.root"
                                      #'file:input.root'
    ),
    secondaryFileNames = cms.untracked.vstring()
)

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')

# Path and EndPath definitions
process.validation_step = cms.Path(process.mix * process.cscDigiValidation)
process.harvesting_step = cms.Path(process.cscDigiHarvesting)
process.endjob_step = cms.EndPath(process.endOfProcess)

# Schedule definition
process.schedule = cms.Schedule(
    process.validation_step,
    process.harvesting_step,
    process.endjob_step
)
