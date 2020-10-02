#ifndef Validation_MuonCSCDigis_CSCStubEfficiencyValidation_H
#define Validation_MuonCSCDigis_CSCStubEfficiencyValidation_H

#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigiCollection.h"
#include "Validation/MuonCSCDigis/interface/CSCBaseValidation.h"
#include "Validation/MuonCSCDigis/interface/CSCStubMatcher.h"

#include "Validation/MuonCSCDigis/interface/CSCBaseValidation.h"

class CSCStubEfficiencyValidation : public CSCBaseValidation {
public:
  CSCStubEfficiencyValidation(const edm::InputTag &inputTag, const edm::ParameterSet& pset, edm::ConsumesCollector &&iC);
  ~CSCStubEfficiencyValidation() override;
  void bookHistograms(DQMStore::IBooker &);
  virtual void analyze(const edm::Event &, const edm::EventSetup &);
  // I am not sure if I need another function. This may be it...

private:
  bool isSimTrackGood(const SimTrack& t);
  
  edm::EDGetTokenT<CSCALCTDigiCollection> alcts_Token_;
  edm::EDGetTokenT<CSCCLCTDigiCollection> clcts_Token_;
  edm::EDGetTokenT<CSCCorrelatedLCTDigiCollection> lcts_Token_;
  edm::EDGetTokenT<CSCCorrelatedLCTDigiCollection> mplcts_Token_;
  
  CSCStubMatcher cscStubMatcher_;

  MonitorElement *numeratorPlots[10];
  MonitorElement *denominatorPlots[10];

  edm::EDGetTokenT<edm::SimVertexContainer> simVertexInput_;
  edm::EDGetTokenT<edm::SimTrackContainer> simTrackInput_;
  double simTrackMinPt_;
  double simTrackMinEta_;
  double simTrackMaxEta_;
};

#endif
