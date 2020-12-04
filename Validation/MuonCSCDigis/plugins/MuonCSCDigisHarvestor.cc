#ifndef Validation_MuonCSCDigis_MuonCSCDigisHarvestor_h
#define Validation_MuonCSCDigis_MuonCSCDigisHarvestor_h

#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"

class MuonCSCDigisHarvestor : public MuonGEMBaseHarvestor {
public:
  /// constructor
  explicit MuonCSCDigisHarvestor(const edm::ParameterSet&);
  /// destructor
  ~MuonCSCDigisHarvestor() override {}

  void dqmEndJob(DQMStore::IBooker&, DQMStore::IGetter&) override;

private:
};

MuonCSCDigisHarvestor::MuonCSCDigisHarvestor(const edm::ParameterSet& pset)
    : MuonGEMBaseHarvestor(pset, "MuonGEMDigisHarvestor") {}

void MuonCSCDigisHarvestor::dqmEndJob(DQMStore::IBooker& booker, DQMStore::IGetter& getter) {
  TString eff_folder = "MuonCSCDigisV/CSCDigisTask/Efficiency/";
}

DEFINE_FWK_MODULE(MuonCSCDigisHarvestor);

#endif
