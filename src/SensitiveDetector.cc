#include "SensitiveDetector.hh"


SensitiveDetector::SensitiveDetector(const G4String &sdName, G4int detID, const G4String &detName, const G4bool isOpt)
    : G4VSensitiveDetector(sdName), detID(detID), detName(detName), isLED(isOpt) {
    collectionName.insert("EdepHits");
    if (isLED) collectionName.insert("OptHits");
}

void SensitiveDetector::Initialize(G4HCofThisEvent *hce) {
    hits = new SDHitCollection(SensitiveDetectorName, collectionName[0]);
    indexByVol.clear();

    if (HCID < 0) {
        HCID = G4SDManager::GetSDMpointer()->GetCollectionID(hits);
    }
    hce->AddHitsCollection(HCID, hits);

    if (isLED) {
        optHC = new SDHitCollection(SensitiveDetectorName, "OptHits");
        auto optID = G4SDManager::GetSDMpointer()->GetCollectionID(optHC);
        hce->AddHitsCollection(optID, optHC);
    } else {
        optHC = nullptr;
    }
}

G4bool SensitiveDetector::ProcessHits(G4Step *step, G4TouchableHistory *) {
    auto *track = step->GetTrack();
    auto pd = track->GetDefinition();

    if (isLED and pd == G4OpticalPhoton::OpticalPhotonDefinition()) {
        if (optHC && step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
            auto *post = step->GetPostStepPoint();
            auto touch = post->GetTouchableHandle();

            const auto T = touch->GetHistory()->GetTopTransform();
            const auto x_loc = T.Inverse().TransformPoint(post->GetPosition());

            auto *h = new SDHit();
            h->isOptical = true;
            h->volumeID = detID;
            h->x_loc_mm = x_loc.x() / mm;
            h->y_loc_mm = x_loc.y() / mm;
            h->z_loc_mm = x_loc.z() / mm;
            h->phi = std::atan2(x_loc.y(), x_loc.x());
            h->t_ns = post->GetGlobalTime() / ns;
            h->copyNo = touch->GetVolume()->GetCopyNo();

            optHC->insert(h);
            return true;
        }
        return false;
    }

    const G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.) return false;

    const G4VTouchable *touch = step->GetPreStepPoint()->GetTouchable();
    const int volumeID = touch->GetVolume()->GetCopyNo();
    const std::uint64_t hitKey = BuildHitKey(touch, track->GetTrackID());

    const G4double t = step->GetPreStepPoint()->GetGlobalTime();

    // НОВОЕ: Получаем локальные координаты и имя частицы
    auto* prePoint = step->GetPreStepPoint();
    G4ThreeVector globalPos = prePoint->GetPosition();
    const G4AffineTransform transformation = touch->GetHistory()->GetTopTransform();
    G4ThreeVector localPos = transformation.Inverse().TransformPoint(globalPos);

    G4String particleName = pd->GetParticleName();

    SDHit *hit = FindOrCreateHit(hitKey, volumeID);
    hit->AddEdep(edep);
    hit->UpdateTmin(t);

    // НОВОЕ: Сохраняем координаты и имя частицы
    hit->x_loc_mm = localPos.x() / mm;
    hit->y_loc_mm = localPos.y() / mm;
    hit->z_loc_mm = localPos.z() / mm;
    hit->t_ns = t / ns;
    hit->copyNo = volumeID;
    hit->particleName = particleName;
    hit->particlePDG = pd->GetPDGEncoding();
    hit->trackID = track->GetTrackID();
    hit->parentTrackID = track->GetParentID();

    if (detID == 3 || detID == 5) {
        const int raw = (detID == 5) ? (volumeID - 50000) : volumeID;
        if (raw >= 0) {
            hit->fiberLayer = raw / 1000;
            hit->fiberRow = raw % 1000;
        }
        hit->fiberPlane = (detID == 3) ? "X" : "Y";

        const int depth = touch->GetHistoryDepth();
        for (int level = 0; level <= depth; ++level) {
            const auto *vol = touch->GetVolume(level);
            if (!vol) continue;
            if (vol->GetName() == "FiberModulePV") {
                hit->fiberModule = touch->GetCopyNumber(level);
                break;
            }
        }
    }

    return true;
}

std::uint64_t SensitiveDetector::BuildHitKey(const G4VTouchable *touch, G4int trackID) {
    std::uint64_t key = 1469598103934665603ull;
    constexpr std::uint64_t prime = 1099511628211ull;
    const int depth = touch->GetHistoryDepth();
    for (int level = 0; level <= depth; ++level) {
        const auto *vol = touch->GetVolume(level);
        const std::uint64_t copyNo = static_cast<std::uint64_t>(touch->GetCopyNumber(level));
        const std::uint64_t volPtr = reinterpret_cast<std::uintptr_t>(vol);
        key ^= copyNo + 0x9e3779b97f4a7c15ull + (key << 6) + (key >> 2);
        key *= prime;
        key ^= volPtr + 0x9e3779b97f4a7c15ull + (key << 6) + (key >> 2);
        key *= prime;
    }
    key ^= static_cast<std::uint64_t>(trackID) + 0x9e3779b97f4a7c15ull + (key << 6) + (key >> 2);
    key *= prime;
    return key;
}

SDHit *SensitiveDetector::FindOrCreateHit(std::uint64_t hitKey, G4int volumeID) {
    auto it = indexByVol.find(hitKey);
    if (it != indexByVol.end()) {
        return (*hits)[it->second];
    }

    auto *h = new SDHit(volumeID);
    int idx = hits->insert(h) - 1;
    indexByVol.emplace(hitKey, idx);
    return h;
}