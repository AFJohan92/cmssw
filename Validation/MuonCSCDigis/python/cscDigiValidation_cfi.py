import FWCore.ParameterSet.Config as cms
from Validation.MuonHits.muonSimHitMatcherPSet import *
from Validation.MuonCSCDigis.muonCSCStubPSet import *
from Validation.MuonCSCDigis.muonCSCDigiPSet import *

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
cscDigiValidation = DQMEDAnalyzer('CSCDigiValidation',
    muonSimHitMatcherPSet,                                                                                                                                     
    muonCSCStubPSet,                                                                                                                                           
    muonCSCDigiPSet,
    simHitsTag = cms.InputTag("mix", "g4SimHitsMuonCSCHits"),
    wireDigiTag = cms.InputTag("simMuonCSCDigis","MuonCSCWireDigi"),
    outputFile = cms.string(''),
    stripDigiTag = cms.InputTag("simMuonCSCDigis","MuonCSCStripDigi"),
    comparatorDigiTag = cms.InputTag("simMuonCSCDigis","MuonCSCComparatorDigi"),
    alctDigiTag = cms.InputTag("simCscTriggerPrimitiveDigis"),
    clctDigiTag = cms.InputTag("simCscTriggerPrimitiveDigis"),
    doSim = cms.bool(False)
    #And GEM matcher?
)

from Configuration.Eras.Modifier_fastSim_cff import fastSim
fastSim.toModify(cscDigiValidation, simHitsTag = "mix:MuonSimHitsMuonCSCHits")
