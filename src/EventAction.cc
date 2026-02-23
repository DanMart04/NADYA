#include "EventAction.hh"

using namespace Sizes;

EventAction::EventAction(AnalysisManager* an, RunAction* r, const G4double eCrystalThr, const G4double eVetoThr,
                         const G4bool saveOpt) : analysisManager(an), run(r), eCrystalThreshold(eCrystalThr),
                                                 eVetoThreshold(eVetoThr), saveOptics(saveOpt) {
    // UPDATED detector mapping for NEW geometry
    // {HC Name, detID, Readable Name}
    detMap = {
        {"Trigger1LowerSD/EdepHits", 0, "Trigger1Lower"},
        {"Trigger1UpperSD/EdepHits", 1, "Trigger1Upper"},
        {"VetoSD/EdepHits",          2, "Veto"},
        {"CoordSD/EdepHits",         3, "FiberX"},
        {"Trigger2LowerSD/EdepHits", 4, "Trigger2Lower"},
        {"FiberSD/EdepHits",         5, "FiberY"},
        {"CalorimeterSD/EdepHits",   6, "Calorimeter"},
        {"Trigger2UpperSD/EdepHits", 7, "Trigger2Upper"},
    };
    HCIDs.assign(detMap.size(), -1);
}

void EventAction::BeginOfEventAction(const G4Event*) {
    nPrimaries = 0;
    nInteractions = 0;
    nHits = 0;
    hasCrystal = false;
    hasVeto = false;
}

void EventAction::EndOfEventAction(const G4Event* evt) {
    const int eventID = evt->GetEventID();

    // 1. Write primaries
    WritePrimaries_(eventID);
    nPrimaries = static_cast<int>(primBuf.size());
    primBuf.clear();

    // 2. Write interactions (simplified)
    nInteractions = WriteInteractions_(eventID);
    interBuf.clear();

    // 2b. Write all secondaries
    WriteSecondaries_(eventID);
    secBuf.clear();

    // 3. Write hits from sensitive detectors
    nHits = WriteHitsFromSD_(evt, eventID);

    // 4. Write event summary
    analysisManager->FillEventRow(eventID, nPrimaries, nHits);

    // 5. Counts for statistics
    if (hasCrystal && !hasVeto) run->AddCrystalOnly(1);
    if (hasCrystal && hasVeto) run->AddCrystalAndVeto(1);
}

// ═══════════════════════════════════════════════════════
// Private helpers
// ═══════════════════════════════════════════════════════

void EventAction::WritePrimaries_(int eventID) {
    for (const auto& p : primBuf) {
        analysisManager->FillPrimaryRow(eventID, p.name, p.E_MeV, p.dir);
        analysisManager->FillPrimaryCsvRow(eventID, p.name, p.E_MeV, p.dir);
    }
}

int EventAction::WriteInteractions_(int eventID) {
    int n = 0;
    for (const auto& r : interBuf) {
        // Save only critical interactions
        if (r.process == "compt" || r.process == "phot" || r.process == "conv") {
            analysisManager->FillInteractionRow(eventID, r.trackID, r.parentID,
                                               r.process, r.volumeName, r.E_MeV);
            n++;
        }
    }
    return n;
}

int EventAction::WriteSecondaries_(int eventID) {
    int n = 0;
    for (const auto& s : secBuf) {
        analysisManager->FillSecondaryCsvRow(eventID, s.secondaryTrackID, s.parentTrackID, s.parentPDG, s.parentName,
                                             s.process, s.birthVolumeName, s.secondaryName, s.secondaryPDG,
                                             s.E_MeV, s.dir0, s.t0_ns);
        ++n;
    }
    return n;
}

int EventAction::WriteHitsFromSD_(const G4Event* evt, int eventID) {
    auto* hce = evt->GetHCofThisEvent();
    if (!hce) return 0;

    auto* sdm = G4SDManager::GetSDMpointer();

    // Get collection IDs
    for (size_t i = 0; i < detMap.size(); ++i) {
        if (HCIDs[i] < 0) {
            const auto& hcName = std::get<0>(detMap[i]);
            HCIDs[i] = sdm->GetCollectionID(hcName);
        }
    }

    int nHitsTotal = 0;
    G4double edepVeto_MeV = 0.0;
    G4double edepTrig1Lower_MeV = 0.0;
    G4double edepTrig1Upper_MeV = 0.0;
    G4double edepTrig2Lower_MeV = 0.0;
    G4double edepTrig2Upper_MeV = 0.0;
    G4double edepFiberX_MeV = 0.0;
    G4double edepFiberY_MeV = 0.0;
    G4double edepCalo_MeV = 0.0;

    // Loop over all detectors
    for (size_t i = 0; i < detMap.size(); ++i) {
        const int hcID = HCIDs[i];
        if (hcID < 0) continue;

        auto* hc = dynamic_cast<SDHitCollection*>(hce->GetHC(hcID));
        if (!hc) continue;

        const int detID = std::get<1>(detMap[i]);
        const auto& det_name = std::get<2>(detMap[i]);

        const auto N = hc->GetSize();
        for (unsigned j = 0; j < N; ++j) {
            auto* h = (*hc)[j];
            if (!h) continue;

            double edep_MeV = h->edep / MeV;

            if (detID == 2) edepVeto_MeV += edep_MeV;
            else if (detID == 0) edepTrig1Lower_MeV += edep_MeV;
            else if (detID == 1) edepTrig1Upper_MeV += edep_MeV;
            else if (detID == 4) edepTrig2Lower_MeV += edep_MeV;
            else if (detID == 7) edepTrig2Upper_MeV += edep_MeV;
            else if (detID == 3) edepFiberX_MeV += edep_MeV;
            else if (detID == 5) edepFiberY_MeV += edep_MeV;
            else if (detID == 6) edepCalo_MeV += edep_MeV;

            // Apply thresholds
            if ((detID == 2) && edep_MeV <= eVetoThreshold) {  // Veto
                continue;
            }
            if ((detID == 6) && edep_MeV <= eCrystalThreshold) {  // Calorimeter
                continue;
            }

            G4double t_ns = h->t_ns;
            G4int copyNo = h->copyNo;
            G4int trackID = h->trackID;
            G4int parentTrackID = h->parentTrackID;
            G4int particlePDG = h->particlePDG;

            // Get particle name from SDHit
            G4String particleName = h->particleName;

            // Write hit
            analysisManager->FillHitRow(eventID, detID, det_name, copyNo,
                                       edep_MeV, t_ns, particleName);

            if (detID == 3 || detID == 5) {
                analysisManager->FillFiberCsvRow(eventID, particleName, h->fiberPlane, h->fiberModule,
                                                 h->fiberLayer, h->fiberRow, copyNo, edep_MeV, t_ns);
            }
            G4int crystalIx = -1;
            G4int crystalIy = -1;
            if (detID == 6) {
                crystalIx = copyNo / 10;
                crystalIy = copyNo % 10;
                analysisManager->FillCrystalCsvRow(eventID, copyNo, crystalIx, crystalIy,
                                                   edep_MeV, t_ns, particleName);
            }
            analysisManager->FillHitCsvRow(eventID, trackID, parentTrackID, particleName, particlePDG, detID,
                                           det_name, copyNo, edep_MeV, h->fiberPlane, h->fiberModule,
                                           h->fiberLayer, h->fiberRow, crystalIx, crystalIy);

            nHitsTotal++;

            // Mark for statistics
            if (edep_MeV > 0.0) {
                if (detID == 6) MarkCrystal();      // Calorimeter
                else if (detID == 2) MarkVeto();    // Veto
            }
        }
    }

    analysisManager->FillEdepCsvRow(eventID, edepVeto_MeV, edepTrig1Lower_MeV, edepTrig1Upper_MeV,
                                    edepTrig2Lower_MeV, edepTrig2Upper_MeV, edepFiberX_MeV,
                                    edepFiberY_MeV, edepCalo_MeV);

    return nHitsTotal;
}
