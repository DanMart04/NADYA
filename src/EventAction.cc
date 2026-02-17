#include "EventAction.hh"

using namespace Sizes;

EventAction::EventAction(AnalysisManager* an, RunAction* r, const G4double eCrystalThr, const G4double eVetoThr,
                         const G4bool saveOpt) : analysisManager(an), run(r), eCrystalThreshold(eCrystalThr),
                                                 eVetoThreshold(eVetoThr), saveOptics(saveOpt) {
    // UPDATED detector mapping for NEW geometry
    // {HC Name, detID, Readable Name}
    detMap = {
        {"TOFTopSD/EdepHits",       0, "TOFTop"},
        {"TOFBottomSD/EdepHits",    1, "TOFBottom"},
        {"VetoSD/EdepHits",         2, "Veto"},
        {"CoordSD/EdepHits",        3, "FiberX"},
        {"FiberSD/EdepHits",        5, "FiberY"},
        {"CalorimeterSD/EdepHits",  6, "Calorimeter"},
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

    // 3. Write hits from sensitive detectors (WITH COORDINATES!)
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
        analysisManager->FillPrimaryRow(eventID, p.name, p.E_MeV, p.dir, p.pos_mm);
    }
}

int EventAction::WriteInteractions_(int eventID) {
    int n = 0;
    for (const auto& r : interBuf) {
        // Save only critical interactions
        if (r.process == "compt" || r.process == "phot" || r.process == "conv") {
            analysisManager->FillInteractionRow(eventID, r.trackID, r.parentID,
                                               r.process, r.volumeName, r.pos_mm, r.E_MeV);
            n++;
        }
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

            // Apply thresholds
            if ((detID == 2) && edep_MeV <= eVetoThreshold) {  // Veto
                continue;
            }
            if ((detID == 6) && edep_MeV <= eCrystalThreshold) {  // Calorimeter
                continue;
            }

            // Get coordinates from SDHit (LOCAL coordinates!)
            G4ThreeVector pos_mm(h->x_loc_mm, h->y_loc_mm, h->z_loc_mm);
            G4double t_ns = h->t_ns;
            G4int copyNo = h->copyNo;

            // Get particle name from SDHit
            G4String particleName = h->particleName;

            // Write hit
            analysisManager->FillHitRow(eventID, detID, det_name, copyNo,
                                       edep_MeV, pos_mm, t_ns, particleName);

            nHitsTotal++;

            // Mark for statistics
            if (edep_MeV > 0.0) {
                if (detID == 6) MarkCrystal();      // Calorimeter
                else if (detID == 2) MarkVeto();    // Veto
            }
        }
    }

    return nHitsTotal;
}
