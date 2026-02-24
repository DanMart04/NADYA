#include "RunAction.hh"


RunAction::RunAction() : crystalOnly(0), crystalAndVeto(0) {
    const G4String fileName = "NADYA.root";
    analysisManager = new AnalysisManager(fileName);

    auto *mgr = G4AccumulableManager::Instance();
    mgr->Register(crystalOnly);
    mgr->Register(crystalAndVeto);
}

RunAction::~RunAction() {
    delete analysisManager;
}

void RunAction::BeginOfRunAction(const G4Run *) {
    analysisManager->Open();
    auto *mgr = G4AccumulableManager::Instance();
    mgr->Reset();
}

void RunAction::EndOfRunAction(const G4Run *) {
    analysisManager->Close();
    auto *mgr = G4AccumulableManager::Instance();
    mgr->Merge();
#ifdef G4MULTITHREADED
    if (G4Threading::IsMasterThread()) {
        AnalysisManager::MergeCsvParts("events_primary");
        AnalysisManager::MergeCsvParts("events_fibers");
        AnalysisManager::MergeCsvParts("events_hits");
        AnalysisManager::MergeCsvParts("events_secondaries");
        AnalysisManager::MergeCsvParts("events_crystals");
        AnalysisManager::MergeCsvParts("events_edep");
    }
#endif
    if (G4Threading::IsMasterThread()) {
        totals.crystalAndVeto = crystalAndVeto.GetValue();
        totals.crystalOnly = crystalOnly.GetValue();
    }
}
