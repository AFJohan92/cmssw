#ifndef Validation_MuonCSCDigis_CSCStubEfficiencyValidation_H
#define Validation_MuonCSCDigis_CSCStubEfficiencyValidation_H

#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigiCollection.h"
#include "Validation/MuonCSCDigis/interface/CSCBaseValidation.h"
#include "Validation/MuonCSCDigis/interface/CSCStubMatcher.h"

class CSCStubEfficiencyValidation : public CSCBaseValidation {
public:
  CSCStubEfficiencyValidation(const edm::InputTag &inputTag, edm::ConsumesCollector &&iC);
  ~CSCStubEfficiencyValidation() override;
  void bookHistograms(DQMStore::IBooker &);
  void analyze(const edm::Event &, const edm::EventSetup &, const edm::ParameterSet& pset) override;
  // I am not sure if I need another function. This may be it...

private:
  edm::EDGetTokenT<CSCALCTDigiCollection> alcts_Token_;
  edm::EDGetTokenT<CSCALCTDigiCollection> alcts_Token_;
  edm::EDGetTokenT<CSCALCTDigiCollection> alcts_Token_;
  CSCStubMatcher cscStubMatcher_;

  MonitorElement *theTimeBinPlots[10];
  MonitorElement *theNDigisPerLayerPlots[10];
  MonitorElement *theNDigisPerEventPlot;
  // Add more plots here when I figure out how to actually get the info and plot the efficiencies
};

#endif
