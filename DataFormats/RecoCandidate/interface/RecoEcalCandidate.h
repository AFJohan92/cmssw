#ifndef RecoCandidate_RecoEcalCandidate_h
#define RecoCandidate_RecoEcalCandidate_h
/** \class reco::RecoEcalCandidate
 *
 * Reco Candidates with a Super Cluster component
 *
 * \author Luca Lista, INFN
 *
 * \version $Id: RecoEcalCandidate.h,v 1.1 2006/07/26 07:42:08 llista Exp $
 *
 */
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidateFwd.h"

namespace reco {

  class RecoEcalCandidate : public RecoCandidate {
  public:
    /// default constructor
    RecoEcalCandidate() : RecoCandidate() { }
    /// constructor from values
    RecoEcalCandidate( Charge q , const LorentzVector & p4, const Point & vtx = Point( 0, 0, 0 ) ) :
      RecoCandidate( q, p4, vtx ) { }
    /// destructor
    virtual ~RecoEcalCandidate();
    /// returns a clone of the candidate
    virtual RecoEcalCandidate * clone() const;
    /// set reference to track
    void setSuperCluster( const reco::SuperClusterRef & r ) { superCluster_ = r; }

  private:
    /// check overlap with another candidate
    virtual bool overlap( const Candidate & ) const;
    /// reference to a track
    virtual reco::SuperClusterRef superCluster() const;
    /// reference to a track
    reco::SuperClusterRef superCluster_;
  };
  
}

#endif
