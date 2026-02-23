#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include <G4UserEventAction.hh>
#include <G4ThreeVector.hh>
#include <G4String.hh>
#include <G4Event.hh>
#include <G4Run.hh>
#include <G4RunManager.hh>
#include <G4SDManager.hh>
#include <G4HCofThisEvent.hh>
#include <G4SystemOfUnits.hh>
#include <G4Track.hh>
#include <cfloat>
#include <vector>
#include <map>

#include "Geometry.hh"
#include "RunAction.hh"
#include "Sizes.hh"
#include "AnalysisManager.hh"
#include "SDHit.hh"

class G4Event;
class Geometry;

struct PrimaryRec {
    int index = 0;
    int pdg = 0;
    G4String name;
    double E_MeV = 0.0;
    G4ThreeVector dir;
    G4ThreeVector pos_mm;
    double t0_ns = 0.0;
};

struct InteractionRec {
    int trackID = -1;
    int parentID = -1;
    G4String process;
    G4String volumeName;
    G4ThreeVector pos_mm;
    double E_MeV = 0.0;
};

struct SecondaryRec {
    int secondaryTrackID = -1;
    int parentTrackID = -1;
    int parentPDG = 0;
    G4String parentName;
    G4String process;
    G4String birthVolumeName;
    G4String secondaryName;
    int secondaryPDG = 0;
    G4ThreeVector birthPos_mm;
    G4ThreeVector dir0;
    double E_MeV = 0.0;
    double t0_ns = 0.0;
};

class EventAction : public G4UserEventAction {
public:
    std::vector<PrimaryRec> primBuf;
    std::vector<InteractionRec> interBuf;
    std::vector<SecondaryRec> secBuf;

    EventAction(AnalysisManager *, RunAction *, G4double, G4double, G4bool);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event *) override;
    void EndOfEventAction(const G4Event *) override;

private:
    void WritePrimaries_(int eventID);
    int WriteInteractions_(int eventID);
    int WriteSecondaries_(int eventID);
    int WriteHitsFromSD_(const G4Event *evt, int eventID);

    inline void MarkCrystal() { hasCrystal = true; }
    inline void MarkVeto() { hasVeto = true; }

    AnalysisManager *analysisManager = nullptr;

    // Detector mapping: {HC Name, detID, Readable Name}
    // detID: 0=Trigger1Lower, 1=Trigger1Upper, 2=Veto, 3=FiberX,
    //        4=Trigger2Lower, 5=FiberY, 6=Calorimeter, 7=Trigger2Upper
    std::vector<std::tuple<G4String, int, G4String>> detMap;
    std::vector<int> HCIDs;

    int nPrimaries = 0;
    int nInteractions = 0;
    int nHits = 0;

    RunAction* run = nullptr;
    bool hasCrystal = false;
    bool hasVeto = false;

    bool saveOptics = false;

    G4double eCrystalThreshold;
    G4double eVetoThreshold;
};

#endif //EVENTACTION_HH
