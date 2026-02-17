#ifndef ANALYSISMANAGER_HH
#define ANALYSISMANAGER_HH

#include <G4AnalysisManager.hh>
#include <G4UnitsTable.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <globals.hh>
#include <Sizes.hh>

class AnalysisManager {
public:
    G4String fileName = "NADYA";

    explicit AnalysisManager(const std::string &);
    ~AnalysisManager() = default;

    void Open();
    void Close();

    // Event summary
    void FillEventRow(G4int eventID, G4int nPrimaries, G4int nHits);

    // Primary particles
    void FillPrimaryRow(G4int eventID, const G4String &primaryName,
                        G4double E_MeV, const G4ThreeVector &dir,
                        const G4ThreeVector &pos_mm);

    // Hits with coordinates (MAIN DATA!)
    void FillHitRow(G4int eventID, G4int detID, const G4String &detName,
                    G4int copyNo, G4double edep_MeV,
                    const G4ThreeVector &pos_mm, G4double t_ns,
                    const G4String &particleName);

    // Interactions (simplified - only critical)
    void FillInteractionRow(G4int eventID, G4int trackID, G4int parentID,
                           const G4String &process, const G4String &volumeName,
                           const G4ThreeVector &pos_mm, G4double E_MeV);

private:
    G4int eventNT{-1};
    G4int primaryNT{-1};
    G4int hitsNT{-1};           // With coordinates!
    G4int interactionsNT{-1};   // Simplified

    void Book();
};

#endif //ANALYSISMANAGER_HH
