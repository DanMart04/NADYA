#include "AnalysisManager.hh"
#include <iomanip>
#ifdef G4MULTITHREADED
#include <filesystem>
#include <fstream>
#include <algorithm>
namespace fs = std::filesystem;
#endif

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
    analysisManager->FinishNtuple(primaryNT);

    // ═══════════════════════════════════════════════════════
    // 3. HITS - detector hits (MAIN)
    // ═══════════════════════════════════════════════════════
    // detID: 0=Trigger1Lower, 1=Trigger1Upper, 2=Veto, 3=FiberX,
    //        4=Trigger2Lower, 5=FiberY, 6=Calo, 7=Trigger2Upper
    hitsNT = analysisManager->CreateNtuple("hits", "detector hits");
    analysisManager->CreateNtupleIColumn("eventID");
    analysisManager->CreateNtupleIColumn("detID");
    analysisManager->CreateNtupleSColumn("det_name");
    analysisManager->CreateNtupleIColumn("copyNo");
    analysisManager->CreateNtupleDColumn("edep_MeV");
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
    analysisManager->CreateNtupleDColumn("E_MeV");      // Secondary energy
    analysisManager->FinishNtuple(interactionsNT);
}

void AnalysisManager::Open() {
    G4AnalysisManager::Instance()->OpenFile(fileName);

#ifdef G4MULTITHREADED
    const std::string suf = CsvSuffix();
    primaryCsv.open("events_primary" + suf + ".csv", std::ios::out | std::ios::trunc);
    fiberCsv.open("events_fibers" + suf + ".csv", std::ios::out | std::ios::trunc);
    hitsCsv.open("events_hits" + suf + ".csv", std::ios::out | std::ios::trunc);
    secondaryCsv.open("events_secondaries" + suf + ".csv", std::ios::out | std::ios::trunc);
    crystalCsv.open("events_crystals" + suf + ".csv", std::ios::out | std::ios::trunc);
    edepCsv.open("events_edep" + suf + ".csv", std::ios::out | std::ios::trunc);
#else
    primaryCsv.open("events_primary.csv", std::ios::out | std::ios::trunc);
    fiberCsv.open("events_fibers.csv", std::ios::out | std::ios::trunc);
    hitsCsv.open("events_hits.csv", std::ios::out | std::ios::trunc);
    secondaryCsv.open("events_secondaries.csv", std::ios::out | std::ios::trunc);
    crystalCsv.open("events_crystals.csv", std::ios::out | std::ios::trunc);
    edepCsv.open("events_edep.csv", std::ios::out | std::ios::trunc);
#endif

    primaryCsv << "event_id,particle,E_MeV,dir_x,dir_y,dir_z\n";
    fiberCsv << "event_id,particle,plane,module_id,layer_id,row_id,copy_no,edep_MeV,t_ns\n";
    hitsCsv << "event_id,track_id,parent_track_id,is_secondary,particle,particle_pdg,det_id,det_name,copy_no,edep_MeV,plane,module_id,layer_id,row_id,crystal_ix,crystal_iy\n";
    secondaryCsv << "event_id,secondary_track_id,parent_track_id,parent_pdg,parent_name,process,birth_volume,secondary_name,secondary_pdg,E_MeV,dir0_x,dir0_y,dir0_z,t0_ns\n";
    crystalCsv << "event_id,crystal_copy_no,crystal_ix,crystal_iy,edep_MeV,t_ns,particle\n";
    edepCsv << "event_id,edep_veto_MeV,edep_trigger1_lower_MeV,edep_trigger1_upper_MeV,edep_trigger2_lower_MeV,edep_trigger2_upper_MeV,edep_fiber_x_MeV,edep_fiber_y_MeV,edep_calo_MeV,edep_total_MeV\n";
}

void AnalysisManager::Close() {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

    if (primaryCsv.is_open()) primaryCsv.close();
    if (fiberCsv.is_open()) fiberCsv.close();
    if (hitsCsv.is_open()) hitsCsv.close();
    if (secondaryCsv.is_open()) secondaryCsv.close();
    if (crystalCsv.is_open()) crystalCsv.close();
    if (edepCsv.is_open()) edepCsv.close();
}

#ifdef G4MULTITHREADED
std::string AnalysisManager::CsvSuffix() const {
    const G4int id = G4Threading::G4GetThreadId();
    if (id < 0) return "_master";
    return "_" + std::to_string(id);
}

void AnalysisManager::MergeCsvParts(const std::string& baseName) {
    std::string prefix = baseName + "_";
    std::string outPath = baseName + ".csv";
    std::vector<std::string> parts;
    try {
        for (const auto& entry : fs::directory_iterator(".")) {
            if (!entry.is_regular_file()) continue;
            std::string name = entry.path().filename().string();
            if (name.size() > prefix.size() + 4 &&
                name.compare(0, prefix.size(), prefix) == 0 &&
                name.compare(name.size() - 4, 4, ".csv") == 0) {
                parts.push_back(name);
            }
        }
    } catch (const std::exception& e) {
        G4cerr << "[MergeCsvParts] " << baseName << ": " << e.what() << G4endl;
        return;
    }
    if (parts.empty()) return;
    std::sort(parts.begin(), parts.end());
    std::ofstream out(outPath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        G4cerr << "[MergeCsvParts] Cannot open " << outPath << G4endl;
        return;
    }
    bool headerWritten = false;
    for (const std::string& partPath : parts) {
        std::ifstream in(partPath);
        if (!in.is_open()) continue;
        std::string line;
        if (!std::getline(in, line)) { in.close(); continue; }
        if (!headerWritten) {
            out << line << "\n";
            headerWritten = true;
        }
        while (std::getline(in, line))
            out << line << "\n";
        in.close();
        try { fs::remove(partPath); } catch (...) {}
    }
}
#endif

void AnalysisManager::FillEventRow(G4int eventID, G4int nPrimaries, G4int nHits) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(eventNT, 0, eventID);
    analysisManager->FillNtupleIColumn(eventNT, 1, nPrimaries);
    analysisManager->FillNtupleIColumn(eventNT, 2, nHits);
    analysisManager->AddNtupleRow(eventNT);
}

void AnalysisManager::FillPrimaryRow(G4int eventID, const G4String &primaryName,
                                     G4double E_MeV, const G4ThreeVector &dir) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(primaryNT, 0, eventID);
    analysisManager->FillNtupleSColumn(primaryNT, 1, primaryName);
    analysisManager->FillNtupleDColumn(primaryNT, 2, E_MeV);
    analysisManager->FillNtupleDColumn(primaryNT, 3, dir.x());
    analysisManager->FillNtupleDColumn(primaryNT, 4, dir.y());
    analysisManager->FillNtupleDColumn(primaryNT, 5, dir.z());
    analysisManager->AddNtupleRow(primaryNT);
}

void AnalysisManager::FillHitRow(G4int eventID, G4int detID, const G4String &detName,
                                 G4int copyNo, G4double edep_MeV,
                                 G4double t_ns, const G4String &particleName) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(hitsNT, 0, eventID);
    analysisManager->FillNtupleIColumn(hitsNT, 1, detID);
    analysisManager->FillNtupleSColumn(hitsNT, 2, detName);
    analysisManager->FillNtupleIColumn(hitsNT, 3, copyNo);
    analysisManager->FillNtupleDColumn(hitsNT, 4, edep_MeV);
    analysisManager->FillNtupleDColumn(hitsNT, 5, t_ns);
    analysisManager->FillNtupleSColumn(hitsNT, 6, particleName);
    analysisManager->AddNtupleRow(hitsNT);
}

void AnalysisManager::FillInteractionRow(G4int eventID, G4int trackID, G4int parentID,
                                        const G4String &process, const G4String &volumeName,
                                        G4double E_MeV) {
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(interactionsNT, 0, eventID);
    analysisManager->FillNtupleIColumn(interactionsNT, 1, trackID);
    analysisManager->FillNtupleIColumn(interactionsNT, 2, parentID);
    analysisManager->FillNtupleSColumn(interactionsNT, 3, process);
    analysisManager->FillNtupleSColumn(interactionsNT, 4, volumeName);
    analysisManager->FillNtupleDColumn(interactionsNT, 5, E_MeV);
    analysisManager->AddNtupleRow(interactionsNT);
}

void AnalysisManager::FillPrimaryCsvRow(G4int eventID, const G4String &particleName, G4double E_MeV,
                                        const G4ThreeVector &dir) {
    if (!primaryCsv.is_open()) return;
    primaryCsv << eventID << ","
               << particleName << ","
               << std::setprecision(10) << E_MeV << ","
               << dir.x() << "," << dir.y() << "," << dir.z() << "\n";
}

void AnalysisManager::FillFiberCsvRow(G4int eventID, const G4String &particleName, const G4String &plane,
                                      G4int moduleID, G4int layerID, G4int rowID, G4int copyNo,
                                      G4double edep_MeV, G4double t_ns) {
    if (!fiberCsv.is_open()) return;
    fiberCsv << eventID << ","
             << particleName << ","
             << plane << ","
             << moduleID << ","
             << layerID << ","
             << rowID << ","
             << copyNo << ","
             << std::setprecision(10) << edep_MeV << ","
             << t_ns << "\n";
}

void AnalysisManager::FillHitCsvRow(G4int eventID, G4int trackID, G4int parentTrackID,
                                    const G4String &particleName, G4int particlePDG, G4int detID,
                                    const G4String &detName, G4int copyNo, G4double edep_MeV,
                                    const G4String &plane, G4int moduleID, G4int layerID, G4int rowID,
                                    G4int crystalIx, G4int crystalIy) {
    if (!hitsCsv.is_open()) return;
    const G4int isSecondary = parentTrackID > 0 ? 1 : 0;
    hitsCsv << eventID << ","
            << trackID << ","
            << parentTrackID << ","
            << isSecondary << ","
            << particleName << ","
            << particlePDG << ","
            << detID << ","
            << detName << ","
            << copyNo << ","
            << std::setprecision(10) << edep_MeV << ","
            << plane << "," << moduleID << "," << layerID << "," << rowID << ","
            << crystalIx << ","
            << crystalIy << "\n";
}

void AnalysisManager::FillSecondaryCsvRow(G4int eventID, G4int secondaryTrackID, G4int parentTrackID, G4int parentPDG,
                                          const G4String &parentName, const G4String &process,
                                          const G4String &birthVolumeName, const G4String &secondaryName,
                                          G4int secondaryPDG, G4double E_MeV, const G4ThreeVector &dir0, G4double t0_ns) {
    if (!secondaryCsv.is_open()) return;
    secondaryCsv << eventID << ","
                 << secondaryTrackID << ","
                 << parentTrackID << ","
                 << parentPDG << ","
                 << parentName << ","
                 << process << ","
                 << birthVolumeName << ","
                 << secondaryName << ","
                 << secondaryPDG << ","
                 << std::setprecision(10) << E_MeV << ","
                 << dir0.x() << "," << dir0.y() << "," << dir0.z() << "," << t0_ns << "\n";
}

void AnalysisManager::FillCrystalCsvRow(G4int eventID, G4int crystalCopyNo, G4int crystalIx, G4int crystalIy,
                                        G4double edep_MeV, G4double t_ns, const G4String &particleName) {
    if (!crystalCsv.is_open()) return;
    crystalCsv << eventID << ","
               << crystalCopyNo << ","
               << crystalIx << ","
               << crystalIy << ","
               << std::setprecision(10) << edep_MeV << ","
               << t_ns << "," << particleName << "\n";
}

void AnalysisManager::FillEdepCsvRow(G4int eventID, G4double edepVeto_MeV, G4double edepTrig1Lower_MeV,
                                     G4double edepTrig1Upper_MeV, G4double edepTrig2Lower_MeV,
                                     G4double edepTrig2Upper_MeV, G4double edepFiberX_MeV,
                                     G4double edepFiberY_MeV, G4double edepCalo_MeV) {
    if (!edepCsv.is_open()) return;
    const G4double total = edepVeto_MeV + edepTrig1Lower_MeV + edepTrig1Upper_MeV + edepTrig2Lower_MeV +
                           edepTrig2Upper_MeV + edepFiberX_MeV + edepFiberY_MeV + edepCalo_MeV;
    edepCsv << eventID << ","
            << std::setprecision(10)
            << edepVeto_MeV << ","
            << edepTrig1Lower_MeV << ","
            << edepTrig1Upper_MeV << ","
            << edepTrig2Lower_MeV << ","
            << edepTrig2Upper_MeV << ","
            << edepFiberX_MeV << ","
            << edepFiberY_MeV << ","
            << edepCalo_MeV << ","
            << total << "\n";
}