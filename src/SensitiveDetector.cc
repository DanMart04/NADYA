#include "SensitiveDetector.hh"

#include <cmath>

#include "Sizes.hh"


SensitiveDetector::SensitiveDetector(const G4String &sdName, G4int detID, G4String detName)
    : G4VSensitiveDetector(sdName), detID(detID), detName(std::move(detName)) {
    collectionName.insert("EdepHits");
}

void SensitiveDetector::Initialize(G4HCofThisEvent *hce) {
    hits = new SDHitCollection(SensitiveDetectorName, collectionName[0]);
    indexByVol.clear();

    if (HCID < 0) {
        HCID = G4SDManager::GetSDMpointer()->GetCollectionID(hits);
    }
    hce->AddHitsCollection(HCID, hits);
}

G4bool SensitiveDetector::ProcessHits(G4Step *step, G4TouchableHistory *) {
    const G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.) return false;

    auto* track = step->GetTrack();
    if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
        return false;
    }

    const G4VTouchable *touch = step->GetPreStepPoint()->GetTouchable();
    const int volumeID = touch->GetVolume()->GetCopyNo();

    const G4double t = step->GetPreStepPoint()->GetGlobalTime();

    SDHit *hit = FindOrCreateHit(volumeID);

    // Optionally decode TOF fiber indices (plane/module/layer/fiberIndex)
    // for CoordSD (X plane) and FiberSD (Y plane).
    if (detName == "TOFFibers" || detName == "Fiber") {
        // Only need to decode once per hit object.
        if (hit->plane < 0 || hit->module < 0 || hit->layer < 0 || hit->fiberIndex < 0) {
            using namespace Sizes;

            // Determine which plane this SD corresponds to: 0 = X, 1 = Y.
            const G4int plane = (detName == "TOFFibers") ? 0 : 1;

            // Recompute rows per layer exactly as in Detector::ConstructTOFFibers.
            const G4double stepFiber = 2.0 * TOFFibers::fiberRadius + TOFFibers::fiberStripGap;
            G4int rows = 0;
            if (plane == 0) {
                const G4int maxRowsX = static_cast<G4int>(std::floor(TOFFibers::sizeX / stepFiber));
                rows = std::max(0, std::min(TOFFibers::fiberRowsX, maxRowsX));
            } else {
                const G4int maxRowsY = static_cast<G4int>(std::floor(TOFFibers::sizeY / stepFiber));
                rows = std::max(0, std::min(TOFFibers::fiberRowsY, maxRowsY));
            }

            G4int layer = -1;
            G4int fiberIndex = -1;
            if (rows > 0) {
                layer = volumeID / rows;
                fiberIndex = volumeID % rows;
            }

            G4int module = -1;
            const G4int depth = touch->GetHistoryDepth();
            for (G4int level = 0; level < depth; ++level) {
                auto* pv = touch->GetVolume(level);
                if (!pv) continue;
                if (pv->GetName() == "FiberModulePV") {
                    module = pv->GetCopyNo();
                    break;
                }
            }

            hit->plane = plane;
            hit->module = module;
            hit->layer = layer;
            hit->fiberIndex = fiberIndex;
        }
    }

    hit->AddEdep(edep);
    hit->UpdateTmin(t);

    return true;
}

SDHit *SensitiveDetector::FindOrCreateHit(G4int volumeID) {
    auto it = indexByVol.find(volumeID);
    if (it != indexByVol.end()) {
        return (*hits)[it->second];
    }

    auto *h = new SDHit(volumeID);
    int idx = hits->insert(h) - 1;
    indexByVol.emplace(volumeID, idx);
    return h;
}