#include "AnalysisManager.hh"

using namespace Sizes;

AnalysisManager::AnalysisManager(const std::string &fName) : fileName(fName) {
    Book();
}

void AnalysisManager::Book() {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetDefaultFileType("root");
    analysisManager->SetFileName(fileName);
    analysisManager->SetVerboseLevel(0);
    analysisManager->SetNtupleActivation(true);

#ifdef G4MULTITHREADED
    analysisManager->SetNtupleMerging(true);
#endif

    // ═══════════════════════════════════════════════════════
    // 1. EVENT - summary per event
    // ═══════════════════════════════════════════════════════
    eventNT = analysisManager->CreateNtuple("event", "per-event summary");
    analysisManager->CreateNtupleIColumn("eventID");
    analysisManager->CreateNtupleIColumn("n_primaries");
    analysisManager->CreateNtupleIColumn("n_hits");
    analysisManager->FinishNtuple(eventNT);

    // ═══════════════════════════════════════════════════════
    // 2. PRIMARY - primary particles
    // ═══════════════════════════════════════════════════════
    primaryNT = analysisManager->CreateNtuple("primary", "primary particles");
    analysisManager->CreateNtupleIColumn("eventID");
    analysisManager->CreateNtupleSColumn("particle");
    analysisManager->CreateNtupleDColumn("E_MeV");
    analysisManager->CreateNtupleDColumn("dir_x");
    analysisManager->CreateNtupleDColumn("dir_y");
    analysisManager->CreateNtupleDColumn("dir_z");
    analysisManager->CreateNtupleDColumn("pos_x_mm");
    analysisManager->CreateNtupleDColumn("pos_y_mm");
    analysisManager->CreateNtupleDColumn("pos_z_mm");
    analysisManager->FinishNtuple(primaryNT);

    // ═══════════════════════════════════════════════════════
    // 3. HITS - detector hits WITH COORDINATES (MAIN!)
    // ═══════════════════════════════════════════════════════
    // detID: 0=TOFTop, 1=TOFBottom, 2=Veto, 3=Coord, 4=FiberX, 5=FiberY, 6=Calo
    hitsNT = analysisManager->CreateNtuple("hits", "detector hits with positions");
    analysisManager->CreateNtupleIColumn("eventID");
    analysisManager->CreateNtupleIColumn("detID");
    analysisManager->CreateNtupleSColumn("det_name");
    analysisManager->CreateNtupleIColumn("copyNo");
    analysisManager->CreateNtupleDColumn("edep_MeV");
    analysisManager->CreateNtupleDColumn("x_mm");       // Local X
    analysisManager->CreateNtupleDColumn("y_mm");       // Local Y
    analysisManager->CreateNtupleDColumn("z_mm");       // Local Z
    analysisManager->CreateNtupleDColumn("t_ns");
    analysisManager->CreateNtupleSColumn("particle");
    analysisManager->FinishNtuple(hitsNT);

    // ═══════════════════════════════════════════════════════
    // 4. INTERACTIONS - gamma interactions (simplified)
    // ═══════════════════════════════════════════════════════
    interactionsNT = analysisManager->CreateNtuple("interactions", "gamma interactions");
    analysisManager->CreateNtupleIColumn("eventID");
    analysisManager->CreateNtupleIColumn("trackID");
    analysisManager->CreateNtupleIColumn("parentID");
    analysisManager->CreateNtupleSColumn("process");
    analysisManager->CreateNtupleSColumn("volume");
    analysisManager->CreateNtupleDColumn("x_mm");
    analysisManager->CreateNtupleDColumn("y_mm");
    analysisManager->CreateNtupleDColumn("z_mm");
    analysisManager->CreateNtupleDColumn("E_MeV");      // Secondary energy
    analysisManager->FinishNtuple(interactionsNT);
}

void AnalysisManager::Open() {
    G4AnalysisManager::Instance()->OpenFile(fileName);
}

void AnalysisManager::Close() {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}

void AnalysisManager::FillEventRow(G4int eventID, G4int nPrimaries, G4int nHits) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(eventNT, 0, eventID);
    analysisManager->FillNtupleIColumn(eventNT, 1, nPrimaries);
    analysisManager->FillNtupleIColumn(eventNT, 2, nHits);
    analysisManager->AddNtupleRow(eventNT);
}

void AnalysisManager::FillPrimaryRow(G4int eventID, const G4String &primaryName,
                                     G4double E_MeV, const G4ThreeVector &dir,
                                     const G4ThreeVector &pos_mm) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(primaryNT, 0, eventID);
    analysisManager->FillNtupleSColumn(primaryNT, 1, primaryName);
    analysisManager->FillNtupleDColumn(primaryNT, 2, E_MeV);
    analysisManager->FillNtupleDColumn(primaryNT, 3, dir.x());
    analysisManager->FillNtupleDColumn(primaryNT, 4, dir.y());
    analysisManager->FillNtupleDColumn(primaryNT, 5, dir.z());
    analysisManager->FillNtupleDColumn(primaryNT, 6, pos_mm.x());
    analysisManager->FillNtupleDColumn(primaryNT, 7, pos_mm.y());
    analysisManager->FillNtupleDColumn(primaryNT, 8, pos_mm.z());
    analysisManager->AddNtupleRow(primaryNT);
}

void AnalysisManager::FillHitRow(G4int eventID, G4int detID, const G4String &detName,
                                 G4int copyNo, G4double edep_MeV,
                                 const G4ThreeVector &pos_mm, G4double t_ns,
                                 const G4String &particleName) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(hitsNT, 0, eventID);
    analysisManager->FillNtupleIColumn(hitsNT, 1, detID);
    analysisManager->FillNtupleSColumn(hitsNT, 2, detName);
    analysisManager->FillNtupleIColumn(hitsNT, 3, copyNo);
    analysisManager->FillNtupleDColumn(hitsNT, 4, edep_MeV);
    analysisManager->FillNtupleDColumn(hitsNT, 5, pos_mm.x());
    analysisManager->FillNtupleDColumn(hitsNT, 6, pos_mm.y());
    analysisManager->FillNtupleDColumn(hitsNT, 7, pos_mm.z());
    analysisManager->FillNtupleDColumn(hitsNT, 8, t_ns);
    analysisManager->FillNtupleSColumn(hitsNT, 9, particleName);
    analysisManager->AddNtupleRow(hitsNT);
}

void AnalysisManager::FillInteractionRow(G4int eventID, G4int trackID, G4int parentID,
                                        const G4String &process, const G4String &volumeName,
                                        const G4ThreeVector &pos_mm, G4double E_MeV) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(interactionsNT, 0, eventID);
    analysisManager->FillNtupleIColumn(interactionsNT, 1, trackID);
    analysisManager->FillNtupleIColumn(interactionsNT, 2, parentID);
    analysisManager->FillNtupleSColumn(interactionsNT, 3, process);
    analysisManager->FillNtupleSColumn(interactionsNT, 4, volumeName);
    analysisManager->FillNtupleDColumn(interactionsNT, 5, pos_mm.x());
    analysisManager->FillNtupleDColumn(interactionsNT, 6, pos_mm.y());
    analysisManager->FillNtupleDColumn(interactionsNT, 7, pos_mm.z());
    analysisManager->FillNtupleDColumn(interactionsNT, 8, E_MeV);
    analysisManager->AddNtupleRow(interactionsNT);
}
