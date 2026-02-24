#ifndef ANALYSISMANAGER_HH
#define ANALYSISMANAGER_HH

#include <G4AnalysisManager.hh>
#include <G4UnitsTable.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <globals.hh>
#include <Sizes.hh>
#include <fstream>
#include <string>

#ifdef G4MULTITHREADED
#include <G4Threading.hh>
#endif

class AnalysisManager {
public:
    G4String fileName = "NADYA";

    explicit AnalysisManager(const std::string &);
    ~AnalysisManager() = default;

    void Open();
    void Close();

#ifdef G4MULTITHREADED
    /** Merge per-thread CSV parts into final baseName.csv (call only on master after run). */
    static void MergeCsvParts(const std::string& baseName);
#endif

    // Event summary
    void FillEventRow(G4int eventID, G4int nPrimaries, G4int nHits);

    // Primary particles
    void FillPrimaryRow(G4int eventID, const G4String &primaryName,
                        G4double E_MeV, const G4ThreeVector &dir);

    // Hits (MAIN DATA!)
    void FillHitRow(G4int eventID, G4int detID, const G4String &detName,
                    G4int copyNo, G4double edep_MeV,
                    G4double t_ns, const G4String &particleName);

    // Interactions (simplified - only critical)
    void FillInteractionRow(G4int eventID, G4int trackID, G4int parentID,
                           const G4String &process, const G4String &volumeName, G4double E_MeV);

    // CSV outputs for post-processing
    void FillPrimaryCsvRow(G4int eventID, const G4String &particleName, G4double E_MeV,
                           const G4ThreeVector &dir);
    void FillFiberCsvRow(G4int eventID, const G4String &particleName, const G4String &plane,
                         G4int moduleID, G4int layerID, G4int rowID, G4int copyNo,
                         G4double edep_MeV, G4double t_ns);
    void FillHitCsvRow(G4int eventID, G4int trackID, G4int parentTrackID,
                       const G4String &particleName, G4int particlePDG, G4int detID,
                       const G4String &detName, G4int copyNo, G4double edep_MeV,
                       const G4String &plane, G4int moduleID, G4int layerID,
                       G4int rowID, G4int crystalIx, G4int crystalIy);
    void FillSecondaryCsvRow(G4int eventID, G4int secondaryTrackID, G4int parentTrackID, G4int parentPDG,
                             const G4String &parentName, const G4String &process,
                             const G4String &birthVolumeName, const G4String &secondaryName,
                             G4int secondaryPDG, G4double E_MeV, const G4ThreeVector &dir0, G4double t0_ns);
    void FillCrystalCsvRow(G4int eventID, G4int crystalCopyNo, G4int crystalIx, G4int crystalIy,
                           G4double edep_MeV, G4double t_ns, const G4String &particleName);
    void FillEdepCsvRow(G4int eventID, G4double edepVeto_MeV, G4double edepTrig1Lower_MeV,
                        G4double edepTrig1Upper_MeV, G4double edepTrig2Lower_MeV,
                        G4double edepTrig2Upper_MeV, G4double edepFiberX_MeV,
                        G4double edepFiberY_MeV, G4double edepCalo_MeV);

private:
    G4int eventNT{-1};
    G4int primaryNT{-1};
    G4int hitsNT{-1};           // With coordinates!
    G4int interactionsNT{-1};   // Simplified

    void Book();
#ifdef G4MULTITHREADED
    /** Thread-specific suffix for CSV filenames (e.g. "_0", "_master"). */
    std::string CsvSuffix() const;
#endif

    std::ofstream primaryCsv;
    std::ofstream fiberCsv;
    std::ofstream hitsCsv;
    std::ofstream secondaryCsv;
    std::ofstream crystalCsv;
    std::ofstream edepCsv;
};

#endif //ANALYSISMANAGER_HH
