#include "SteppingAction.hh"

void SteppingAction::UserSteppingAction(const G4Step* step) {
    // Get EventAction
    auto* ea = dynamic_cast<EventAction*>(
        G4EventManager::GetEventManager()->GetUserEventAction());
    if (!ea) return;

    // Get process info
    const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
    if (!process) return;

    G4String processName = process->GetProcessName();

    auto* track = step->GetTrack();
    auto* particle = track->GetDefinition();
    auto* prePoint = step->GetPreStepPoint();
    auto* touchable = prePoint->GetTouchable();
    G4String volumeName = touchable->GetVolume()->GetName();

    const auto* secondaries = step->GetSecondaryInCurrentStep();
    if (secondaries && !secondaries->empty()) {
        for (const auto* secTrack : *secondaries) {
            if (!secTrack) continue;
            SecondaryRec srec;
            srec.secondaryTrackID = secTrack->GetTrackID();
            srec.parentTrackID = track->GetTrackID();
            srec.parentPDG = particle->GetPDGEncoding();
            srec.parentName = particle->GetParticleName();
            srec.process = processName;
            srec.birthVolumeName = volumeName;
            srec.secondaryName = secTrack->GetDefinition()->GetParticleName();
            srec.secondaryPDG = secTrack->GetDefinition()->GetPDGEncoding();
            srec.E_MeV = secTrack->GetKineticEnergy() / MeV;
            srec.dir0 = secTrack->GetMomentumDirection();
            srec.birthPos_mm = prePoint->GetPosition() / mm;
            srec.t0_ns = prePoint->GetGlobalTime() / ns;
            ea->secBuf.push_back(std::move(srec));
        }
    }

    // Record only critical interactions for gamma
    // compt = Compton, phot = photoelectric, conv = pair production
    if (processName != "compt" && processName != "phot" && processName != "conv") {
        return;
    }

    // Check it's a gamma
    if (particle->GetParticleName() != "gamma") {
        return;
    }

    // Get interaction info
    G4int trackID = track->GetTrackID();
    G4int parentID = track->GetParentID();

    G4ThreeVector pos = prePoint->GetPosition() / mm;

    // Secondary energy
    G4double E_MeV = 0.0;
    if (secondaries && secondaries->size() > 0) {
        // Take energy of first secondary (usually electron or positron)
        E_MeV = (*secondaries)[0]->GetKineticEnergy() / MeV;
    }

    // Create record
    InteractionRec rec;
    rec.trackID = trackID;
    rec.parentID = parentID;
    rec.process = processName;
    rec.volumeName = volumeName;
    rec.pos_mm = pos;
    rec.E_MeV = E_MeV;

    ea->interBuf.push_back(rec);
}
