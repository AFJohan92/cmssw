#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Validation/MuonCSCDigis/interface/CSCStubEfficiencyValidation.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCLayerGeometry.h"

// First goal: have 9 efficiency plots in for ALCT, CLCT and LCT vs eta
// What I need to get there: pull stub information and pass it into an efficiency plotter
// How to get stub information: not sure, but CSC vaidation classes seem to do it
// Which efficiency plotter to use: GEMs use a book1DEff function. Use this? It passes a DQMStore iGetter and iBooker

// Question on constructor: Can I retrieve the inputTag immediately from the pset? I need to initialize the CSCBaseValidation with the IT.
CSCStubEfficiencyValidation::CSCStubEfficiencyValidation(const edm::InputTag &inputTag, const edm::ParameterSet& pset, edm::ConsumesCollector &&iC)
    : CSCBaseValidation(inputTag), theTimeBinPlots(), theNDigisPerLayerPlots() {
  alcts_Token_ = iC.consumes<CSCALCTDigiCollection>(inputTag);
  clcts_Token_ = iC.consumes<CSCCLCTDigiCollection>(inputTag);
  mplcts_Token_ = iC.consumes<CSCMPLCTDigiCollection>(inputTag);
  // Again how do I get the input tag from the PS?

  // Below is what the MuonGEMDigisHarvestor has. I think I will need this kind of information...
  // region_ids_ = pset.getUntrackedParameter<std::vector<Int_t> >("regionIds");
  // station_ids_ = pset.getUntrackedParameter<std::vector<Int_t> >("stationIds");
  // layer_ids_ = pset.getUntrackedParameter<std::vector<Int_t> >("layerIds");

  // Below is what the CSCStubMatcher has. I probably don't need this information, but rather just instantiate the matcher.
  // const auto& cscCLCT = pSet.getParameter<edm::ParameterSet>("cscCLCT");
  // minBXCLCT_ = cscCLCT.getParameter<int>("minBX");
  // maxBXCLCT_ = cscCLCT.getParameter<int>("maxBX");
  // verboseCLCT_ = cscCLCT.getParameter<int>("verbose");
  // minNHitsChamberCLCT_ = cscCLCT.getParameter<int>("minNHitsChamber");
  // <same for ALCT, LCT, and MPLCT>
  // gemDigiMatcher_.reset(new GEMDigiMatcher(pSet, std::move(iC)));
  // cscDigiMatcher_.reset(new CSCDigiMatcher(pSet, std::move(iC)));
  // clctToken_ = iC.consumes<CSCCLCTDigiCollection>(cscCLCT.getParameter<edm::InputTag>("inputTag"));
  // <and the other three
  // geomToken_ = iC.esConsumes<CSCGeometry, MuonGeometryRecord>();
  cscStubMatcher_ = CSCStubMatcher(pset, iC);
}

CSCStubEfficiencyValidation::~CSCStubEfficiencyValidation() {}

// It seems that the other CSC<stub>Validation classes have their own version of bookHistograms, so I am doing the same.
// I think what I'll need here is the 9 efficiency plots mentioned above: ALCT, CLCT, LCT x 3 (what these three are idk...)
// Right now the code below does what CSCALCTDigiValidation does. Just 1+2*10=21 plots: 10xtime, 10xNDigis per layer, and 1xNDigisPerEvent.
void CSCStubEfficiencyValidation::bookHistograms(DQMStore::IBooker &iBooker) {
  theNDigisPerEventPlot = iBooker.book1D("CSCALCTDigisPerEvent", "CSC ALCT Digis per event", 100, 0, 100);
  for (int i = 0; i < 10; ++i) {
    char title1[200], title2[200];
    sprintf(title1, "CSCALCTDigiTimeType%d", i + 1);
    sprintf(title2, "CSCALCTDigisPerLayerType%d", i + 1);
    theTimeBinPlots[i] = iBooker.book1D(title1, title1, 20, 0, 20);
    theNDigisPerLayerPlots[i] = iBooker.book1D(title2, title2, 100, 0, 20);
  }
  // Probably don't need the above
  // What I do need is: NDigisPerStation(or w.e.), NMatchedDigisPerStation (x3stubsx3other)
}

// Similarly, I am doing what CSCALCTDigiValidation does.
void CSCStubEfficiencyValidation::analyze(const edm::Event &e, const edm::EventSetup &eventSetup, const edm::ParameterSet& pset) {
  edm::Handle<CSCALCTDigiCollection> alcts;
  // Probably also want a handle for clcts and lcts

  e.getByToken(alcts_Token_, alcts);
  // Probably also need to access the clcts and lcts with their own tokens too
  if (!alcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get alcts by label " << theInputTag.encode();
  }
  // Not sure where this 'theInputTag is coming from...

  // The following is just a way to count the digis per event. Probably won't need this in the end 
  unsigned nDigisPerEvent = 0;
  for (CSCALCTDigiCollection::DigiRangeIterator j = alcts->begin(); j != alcts->end(); j++) {
    std::vector<CSCALCTDigi>::const_iterator beginDigi = (*j).second.first;
    std::vector<CSCALCTDigi>::const_iterator endDigi = (*j).second.second;
    CSCDetId detId((*j).first.rawId());
    int chamberType = detId.iChamberType();// Returns and int corresponding to chamber type, maybe 1,2,3,4?
    int nDigis = endDigi - beginDigi; // Count NDigis by looking at the number of objects in iterator
    nDigisPerEvent += nDigis;
    theNDigisPerLayerPlots[chamberType - 1]->Fill(nDigis);

    for (std::vector<CSCALCTDigi>::const_iterator digiItr = beginDigi; digiItr != endDigi; ++digiItr) {
      theTimeBinPlots[chamberType - 1]->Fill(digiItr->getBX());// So it's not quite time but rather bunch crossing that is plotted
    }
  }

  // So what needs to happen here is that I need to grab the stub info by layer/chanberType as above.
  // Then I need to match things (idk how) and then also fill plots with what got matched.
  // Then idk if I have to do the division by hand to get the efficiencies or if I can use an existing function

  const auto& simVertex = ps.getParameter<edm::ParameterSet>("simVertex");
  const auto& simTrack = ps.getParameter<edm::ParameterSet>("simTrack");
  simTrackInput_ = consumes<edm::SimTrackContainer>(simTrack.getParameter<edm::InputTag>("inputTag"));
  simTrackMinPt_ = simTrack.getParameter<double>("minPt");
  simTrackMinEta_ = simTrack.getParameter<double>("minEta");
  simTrackMaxEta_ = simTrack.getParameter<double>("maxEta");

  cscStubMatcher_.init(e,eventSetup);
  cscStubMatcher_.match(simTrack,simVertex);
  std::map<unsigned int, CSCCLCTDigiContainer> alcts = cscStubMatcher_.alcts();
  std::map<unsigned int, CSCCLCTDigiContainer> clcts = cscStubMatcher_.clcts();
  std::map<unsigned int, CSCCorrelatedLCTDigiContainer> lcts = cscStubMatcher_.lcts();
  std::map<unsigned int, CSCCorrelatedLCTDigiContainer> mplcts = cscStubMatcher_.mplcts();

  // number of chambIDs is 10?
  for(int chambID = 0; chambID<10; chambID++) {
    if (alcts.contains(chambID)) {
      pass;
    }
  }

  for 
  
}
