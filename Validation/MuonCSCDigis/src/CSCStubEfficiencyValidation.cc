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
CSCStubEfficiencyValidation::CSCStubEfficiencyValidation(const edm::InputTag& inputTag,
                                                         const edm::ParameterSet& pset,
                                                         edm::ConsumesCollector&& iC)
    : CSCBaseValidation(inputTag) {
  const auto& simVertex = pset.getParameter<edm::ParameterSet>("simVertex");
  simVertexInput_ = iC.consumes<edm::SimVertexContainer>(simVertex.getParameter<edm::InputTag>("inputTag"));
  const auto& simTrack = pset.getParameter<edm::ParameterSet>("simTrack");
  simTrackInput_ = iC.consumes<edm::SimTrackContainer>(simTrack.getParameter<edm::InputTag>("inputTag"));
  simTrackMinPt_ = simTrack.getParameter<double>("minPt");
  simTrackMinEta_ = simTrack.getParameter<double>("minEta");
  simTrackMaxEta_ = simTrack.getParameter<double>("maxEta");
  std::cout << "MIN_ETA: " << simTrackMinEta_ << ", MAX_ETA: " << simTrackMaxEta_ << std::endl;

  alcts_Token_ = iC.consumes<CSCALCTDigiCollection>(inputTag);
  clcts_Token_ = iC.consumes<CSCCLCTDigiCollection>(inputTag);
  lcts_Token_ = iC.consumes<CSCCorrelatedLCTDigiCollection>(inputTag);
  //mplcts_Token_ = iC.consumes<CSCCorrelatedLCTDigiCollection>(inputTag);
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

  chambIdsInEndCapStationRings.clear();
  chamberCounterMap.clear();
  int chamberCounter = 0;
  for (int endcap = 1; endcap <= 2; endcap++) {
    for (int station = 1; station <= 4; station++) {
      for (int ring = 1; ring <= 3; ring++) {
        if (ring == 3 && station > 1)
          continue;  // Only station 1 has 3 rings
        std::vector<double> listOfChambers;
        for (int chamber = 1; chamber <= 36; chamber++) {
          CSCDetId ch_id(endcap, station, ring, chamber, 0);
          listOfChambers.push_back(double(ch_id.chamberId()));
        }
        chambIdsInEndCapStationRings.insert(
            {std::to_string(endcap) + std::to_string(station) + std::to_string(ring), listOfChambers});
        chamberCounterMap.insert(
            {std::to_string(endcap) + std::to_string(station) + std::to_string(ring), chamberCounter});
        chamberCounter++;
      }
    }
  }

  etaRanges.clear();
  // ME1/1
  etaRanges.insert({"111", std::tuple(1.6, 2.4)});
  etaRanges.insert({"211", std::tuple(-2.4, -1.6)});
  // ME1/2
  etaRanges.insert({"112", std::tuple(1.2, 1.7)});
  etaRanges.insert({"212", std::tuple(-1.7, -1.2)});
  // ME1/3
  etaRanges.insert({"113", std::tuple(0.9, 1.1)});
  etaRanges.insert({"213", std::tuple(-1.1, -0.9)});
  // ME2/1
  etaRanges.insert({"121", std::tuple(1.6, 2.4)});
  etaRanges.insert({"221", std::tuple(-2.4, -1.6)});
  // ME2/2
  etaRanges.insert({"122", std::tuple(1.0, 1.6)});
  etaRanges.insert({"222", std::tuple(-1.6, -1.0)});
  // ME3/1
  etaRanges.insert({"131", std::tuple(1.7, 2.4)});
  etaRanges.insert({"231", std::tuple(-2.4, -1.7)});
  // ME3/2
  etaRanges.insert({"132", std::tuple(1.1, 1.7)});
  etaRanges.insert({"232", std::tuple(-1.7, -1.1)});
  // ME4/1
  etaRanges.insert({"141", std::tuple(1.8, 2.4)});
  etaRanges.insert({"241", std::tuple(-2.4, -1.8)});
  // ME4/2
  etaRanges.insert({"142", std::tuple(1.2, 1.8)});
  etaRanges.insert({"242", std::tuple(-1.8, -1.2)});
}

CSCStubEfficiencyValidation::~CSCStubEfficiencyValidation() {}

// It seems that the other CSC<stub>Validation classes have their own version of bookHistograms, so I am doing the same.
// I think what I'll need here is the 9 efficiency plots mentioned above: ALCT, CLCT, LCT x 3 (what these three are idk...)
// Right now the code below does what CSCALCTDigiValidation does. Just 1+2*10=21 plots: 10xtime, 10xNDigis per layer, and 1xNDigisPerEvent.
void CSCStubEfficiencyValidation::bookHistograms(DQMStore::IBooker& iBooker) {
  iBooker.setCurrentFolder("MuonCSCDigisV/CSCDigiTask");
  int plotCounter = 0;
  for (int endcap = 1; endcap <= 2; endcap++) {
    // Total of 252 plots per station
    for (int station = 1; station <= 4; station++) {
      // Only station 1 has 3 rings
      for (int ring = 1; ring <= 3; ring++) {
        if (ring == 3 && station > 1)
          continue;
        // 36 chambers in rings 2 and 3, 18 in ring 1.
        //for (int chamber = 1; chamber <= 36; chamber++) {
        //if (station > 1 && ring == 1 && chamber > 18) continue;
        //char title1[200], title2[200];
        //sprintf(title1, "CSCStubDigiNumeratorEndCap"+endcap+"Station"+station+"Ring"+ring);
        //sprintf(title2, "CSCStubDigiDenominatorEndCap"+endcap+"Station"+station+"Ring"+ring);
        std::string title1 = "CSCStubDigiNumeratorEndCap" + std::to_string(endcap) + "Station" +
                             std::to_string(station) + "Ring" + std::to_string(ring);
        std::string title2 = "CSCStubDigiDenominatoratorEndCap" + std::to_string(endcap) + "Station" +
                             std::to_string(station) + "Ring" + std::to_string(ring);
        std::string title3 = "CSCStubDigiEfficienciesEndCap" + std::to_string(endcap) + "Station" +
                             std::to_string(station) + "Ring" + std::to_string(ring);
        numeratorPlots[plotCounter] = iBooker.book1D(title1, title1, 6000, -3.0, 3.0);
        denominatorPlots[plotCounter] = iBooker.book1D(title2, title2, 100, -2.4, 2.4);
        efficiencyPlots[plotCounter] = iBooker.book1D(title3, title3, 100, -2.4, 2.4);
        plotCounter++;
        //}
      }
    }
  }
  std::cout << "Total plots: " << plotCounter + 1 << std::endl;
  testHist = iBooker.book1D("CSCStubDigiTest", "CSCStubDigiTest", 100, -2.4, 2.4);
}

/*
Comment what function does
*/
void CSCStubEfficiencyValidation::analyze(const edm::Event& e, const edm::EventSetup& eventSetup) {
  // Define handles
  edm::Handle<edm::SimTrackContainer> sim_tracks;
  edm::Handle<edm::SimVertexContainer> sim_vertices;
  edm::Handle<CSCALCTDigiCollection> alcts;
  edm::Handle<CSCCLCTDigiCollection> clcts;
  edm::Handle<CSCCorrelatedLCTDigiCollection> lcts;
  //edm::Handle<CSCCorrelatedLCTDigiCollection> mplcts;

  // Use token to retreive event information
  e.getByToken(simTrackInput_, sim_tracks);
  e.getByToken(simVertexInput_, sim_vertices);
  e.getByToken(alcts_Token_, alcts);
  e.getByToken(clcts_Token_, clcts);
  e.getByToken(lcts_Token_, lcts);
  //e.getByToken(mplcts_Token_, mplcts);

  // Initialize StubMatcher
  cscStubMatcher_->init(e, eventSetup);

  const edm::SimTrackContainer& sim_track = *sim_tracks.product();
  const edm::SimVertexContainer& sim_vert = *sim_vertices.product();

  if (!alcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get alcts by label " << theInputTag.encode();
  }
  if (!clcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get clcts by label " << theInputTag.encode();
  }
  if (!lcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get lcts by label " << theInputTag.encode();
  }
  /*
  if (!mplcts.isValid()) {
    edm::LogError("CSCDigiDump") << "Cannot get mplcts by label " << theInputTag.encode();
  }
  */

  // Test output
  std::cout << "Total number of SimTrack in this event: " << sim_track.size() << std::endl;

  edm::SimTrackContainer sim_track_selected;
  for (const auto& t : sim_track) {
    if (!isSimTrackGood(t))
      continue;
    sim_track_selected.push_back(t);
  }
  std::cout << "Good simTracks: " << sim_track_selected.size() << std::endl;
  if (sim_track_selected.size() == 0) {
    std::cout << "No matched simtracks, skip event" << std::endl;
    return;
  }

  // Loop through good tracks, use corresponding vetrex to match stubs, then fill hists of chambers where the stub appears.
  for (const auto& t : sim_track_selected) {
    // Match track to stubs with appropriate vertex
    cscStubMatcher_->match(t, sim_vert[t.vertIndex()]);
    if (std::abs(t.momentum().eta()) < 0.9 || std::abs(t.momentum().eta()) > 3.0)
      continue;
    std::cout << "Matched track eta: " << t.momentum().eta() << std::endl;

    // denom: muons (sim_tracks) within a certain eta range that have >3(4) simhits in a chamber;
    for (int endcap = 1; endcap <= 2; endcap++) {
      for (int station = 1; station <= 4; station++) {
        for (int ring = 1; ring <= 3; ring++) {
          if (ring == 3 && station > 1)
            continue;
          std::tuple etaTuple = etaRanges[std::to_string(endcap) + std::to_string(station) + std::to_string(ring)];
          if (std::get<0>(etaTuple) < t.momentum().eta() && t.momentum().eta() < std::get<1>(etaTuple)) {
            //std::string outputString = "In endcap " + std::to_string(endcap) + ", station " + std::to_string(station) +
            //                           ", ring " + std::to_string(ring) + ", chambers";
            //std::vector<double> theseChambers =
            //    chambIdsInEndCapStationRings[std::to_string(endcap) + std::to_string(station) + std::to_string(ring)];
            //for (int i = 0; double(i) < double(theseChambers.size()); i++) {
            //  outputString = outputString + " " + std::to_string(theseChambers[i]);
            //}
            //std::cout << outputString << std::endl;
            denominatorPlots[chamberCounterMap[std::to_string(endcap) + std::to_string(station) + std::to_string(ring)]]
                ->Fill(t.momentum().eta());
          }
        }
      }
    }

    // Store matched stubs.
    // Key: ChamberID, Value : CSCStubDigiContainer
    std::map<unsigned int, CSCALCTDigiContainer> alcts = cscStubMatcher_->alcts();
    std::map<unsigned int, CSCCLCTDigiContainer> clcts = cscStubMatcher_->clcts();
    std::map<unsigned int, CSCCorrelatedLCTDigiContainer> lcts = cscStubMatcher_->lcts();
    //std::map<unsigned int, CSCCorrelatedLCTDigiContainer> mplcts = cscStubMatcher_->mplcts();

    //num: muons (sim_tracks) which also have a stub in that chamber
    //std::cout << "Size of alcts container: " << alcts.size() << std::endl;
    //for (const auto& s : alcts ) {
    //  CSCDetId ch_id(s.first);
    //  std::cout << s.first << ch_id << std::endl;
    //}

    //for (auto& [myKey, myValue] : alcts) {
      //std::cout << myKey << " is in alcts, with this many elements " << myValue.size() << std::endl;
      //}

    for (int endcap = 1; endcap <= 2; endcap++) {
      for (int station = 1; station <= 4; station++) {
        for (int ring = 1; ring <= 3; ring++) {
	  // Only station 1 has 3 rings
          if (ring == 3 && station > 1)
            continue;
          std::vector<double> listOfChambers =
              chambIdsInEndCapStationRings[std::to_string(endcap) + std::to_string(station) + std::to_string(ring)];
          //std::cout << "Length of chamber list: " << listOfChambers.size() << ", and first is " << listOfChambers[0]
          //          << std::endl;
          for (auto chambID : listOfChambers) {
            for (auto& [myKey, myValue] : alcts) {
              //std::cout << "Try myKey and chambID: " << int(myKey) << " and " << int(chambID) << std::endl;
              if (int(myKey) == int(chambID)) {
                //std::cout << "Found myKey " << int(myKey) << " at chambID " << int(chambID)
		//  << ", with this many alcts: " << myValue.size() << std::endl;
                testHist->Fill(myValue.size());
                //if (myValue.size() > 2) {
                //std::cout << "Filling numerator for chambID " << int(chambID) << "at ring "
		//        << std::to_string(endcap) + std::to_string(station) + std::to_string(ring) << std::endl;
                numeratorPlots[chamberCounterMap[std::to_string(endcap) + std::to_string(station) + std::to_string(ring)]]
                    ->Fill(t.momentum().eta());
                //}
              }
            }
          }
        }
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
