#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Validation/MuonCSCDigis/interface/CSCStubEfficiencyValidation.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCLayerGeometry.h"

// First goal: have 9 efficiency plots in for ALCT, CLCT and LCT vs eta
// What I need to get there: pull stub information and pass it into an efficiency plotter
// I think I've gotten the stub info
// Which efficiency plotter to use: GEMs use a book1DEff function. Use this? It passes a DQMStore iGetter and iBooker

// Question on constructor: Can I retrieve the inputTag immediately from the pset? I need to initialize the CSCBaseValidation with the IT.
// Something is not right in this constructor!!!
CSCStubEfficiencyValidation::CSCStubEfficiencyValidation(const edm::InputTag &inputTag, const edm::ParameterSet& pset, edm::ConsumesCollector &&iC)
  : CSCBaseValidation(inputTag) {
  const auto& simVertex = pset.getParameter<edm::ParameterSet>("simVertex");
  simVertexInput_ = iC.consumes<edm::SimVertexContainer>(simVertex.getParameter<edm::InputTag>("inputTag"));
  const auto& simTrack = pset.getParameter<edm::ParameterSet>("simTrack");
  simTrackInput_ = iC.consumes<edm::SimTrackContainer>(simTrack.getParameter<edm::InputTag>("inputTag"));
  simTrackMinPt_ = simTrack.getParameter<double>("minPt");
  simTrackMinEta_ = simTrack.getParameter<double>("minEta");
  simTrackMaxEta_ = simTrack.getParameter<double>("maxEta");

  alcts_Token_ = iC.consumes<CSCALCTDigiCollection>(inputTag);
  clcts_Token_ = iC.consumes<CSCCLCTDigiCollection>(inputTag);
  lcts_Token_ = iC.consumes<CSCCorrelatedLCTDigiCollection>(inputTag);
  mplcts_Token_ = iC.consumes<CSCCorrelatedLCTDigiCollection>(inputTag);
  // Again how do I get the input tag from the PS? As in, do I need both arguments or can I get PS from the inputTag?
  // Could it be the way I did above?

  // Initialize stub matcher
  cscStubMatcher_.reset(new CSCStubMatcher(pset, std::move(iC)));
  //cscStubMatcher_ = CSCStubMatcher(pset, std::move(iC));

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
}

CSCStubEfficiencyValidation::~CSCStubEfficiencyValidation() {}

// It seems that the other CSC<stub>Validation classes have their own version of bookHistograms, so I am doing the same.
// I think what I'll need here is the 9 efficiency plots mentioned above: ALCT, CLCT, LCT x 3 (what these three are idk...)
// Right now the code below does what CSCALCTDigiValidation does. Just 1+2*10=21 plots: 10xtime, 10xNDigis per layer, and 1xNDigisPerEvent.
void CSCStubEfficiencyValidation::bookHistograms(DQMStore::IBooker &iBooker) {
  /*
  // Below is an example. Do I even need this function?
  theNDigisPerEventPlot = iBooker.book1D("CSCALCTDigisPerEvent", "CSC ALCT Digis per event", 100, 0, 100);
  for (int i = 0; i < 10; ++i) {
    char title1[200], title2[200];
    sprintf(title1, "CSCALCTDigiTimeType%d", i + 1);
    sprintf(title2, "CSCALCTDigisPerLayerType%d", i + 1);
    theTimeBinPlots[i] = iBooker.book1D(title1, title1, 20, 0, 20);
    theNDigisPerLayerPlots[i] = iBooker.book1D(title2, title2, 100, 0, 20);
  }
  */
}

/*
Comment what function does
*/
void CSCStubEfficiencyValidation::analyze(const edm::Event &e, const edm::EventSetup &eventSetup) {
  // Define handles
  edm::Handle<edm::SimTrackContainer> sim_tracks;
  edm::Handle<edm::SimVertexContainer> sim_vertices;
  edm::Handle<CSCALCTDigiCollection> alcts;
  edm::Handle<CSCCLCTDigiCollection> clcts;
  edm::Handle<CSCCorrelatedLCTDigiCollection> lcts;
  edm::Handle<CSCCorrelatedLCTDigiCollection> mplcts;

  // Use token to retreive event information
  e.getByToken(simTrackInput_, sim_tracks);
  e.getByToken(simVertexInput_, sim_vertices);
  e.getByToken(alcts_Token_, alcts);
  e.getByToken(clcts_Token_, clcts);
  e.getByToken(lcts_Token_, lcts);
  e.getByToken(mplcts_Token_, lcts);

  // Initialize StubMatcher
  cscStubMatcher_->init(e,eventSetup);
  
  // Not sure what this does, does it flatten the track/vertex container?
  const edm::SimTrackContainer& sim_track = *sim_tracks.product();
  const edm::SimVertexContainer& sim_vert = *sim_vertices.product();
  
  // Not sure where this 'theInputTag' is coming from...
  if (!alcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get alcts by label " << theInputTag.encode();
  }
  if (!clcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get clcts by label " << theInputTag.encode();
  }
  if (!lcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get lcts by label " << theInputTag.encode();
  }
  if (!mplcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get mplcts by label " << theInputTag.encode();
  }

  // Test output
  std::cout << "Total number of SimTrack in this event: " << sim_track.size() << std::endl;

  edm::SimTrackContainer sim_track_selected;
  for (const auto& t : sim_track) {
    if (!isSimTrackGood(t))
      continue;
    sim_track_selected.push_back(t);
  }

  // What should I do for the denominator?

  // To build numerator: loop through good tracks, use corresponding vetrex to match stubs, then fill hists of chambers where the stub appears.
  for (const auto& t : sim_track_selected) {
    // Match track to stubs with appropriate vertex
    // Should this be done on the container, or on the entries of the container as done here?
    cscStubMatcher_->match(t, sim_vert[t.vertIndex()]);

    // Store matched stubs.
    // Key: ChamberID, Value : CSCStubDigiContainer
    std::map<unsigned int, CSCALCTDigiContainer> alcts = cscStubMatcher_->alcts();
    std::map<unsigned int, CSCCLCTDigiContainer> clcts = cscStubMatcher_->clcts();
    std::map<unsigned int, CSCCorrelatedLCTDigiContainer> lcts = cscStubMatcher_->lcts();
    std::map<unsigned int, CSCCorrelatedLCTDigiContainer> mplcts = cscStubMatcher_->mplcts();  

    // Loop over all (10?) ChamberIDs to make numerators and denominators for the efficiency
    for(int chambID = 0; chambID<10; chambID++) {
      if (alcts.find( chambID ) != alcts.end() && alcts[chambID].size() > 2) {
	numeratorPlots[chambID]->Fill(1.);
      }
    }
  }
}

bool CSCStubEfficiencyValidation::isSimTrackGood(const SimTrack& t) {
  // SimTrack selection
  if (t.noVertex())
    return false;
  if (t.noGenpart())
    return false;
  // only muons
  if (std::abs(t.type()) != 13)
    return false;
  // pt selection
  if (t.momentum().pt() < simTrackMinPt_)
    return false;
  // eta selection
  const float eta(std::abs(t.momentum().eta()));
  if (eta > simTrackMaxEta_ || eta < simTrackMinEta_)
    return false;
  return true;
}
