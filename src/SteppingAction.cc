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

    // Record only critical interactions for gamma
    // compt = Compton, phot = photoelectric, conv = pair production
    if (processName != "compt" && processName != "phot" && processName != "conv") {
        return;
    }

    // Check it's a gamma
    auto* track = step->GetTrack();
    auto* particle = track->GetDefinition();
    if (particle->GetParticleName() != "gamma") {
        return;
    }

    // Get interaction info
    G4int trackID = track->GetTrackID();
    G4int parentID = track->GetParentID();

    auto* prePoint = step->GetPreStepPoint();
    auto* touchable = prePoint->GetTouchable();
    G4String volumeName = touchable->GetVolume()->GetName();

    G4ThreeVector pos = prePoint->GetPosition() / mm;

    // Secondary energy
    G4double E_MeV = 0.0;
    const G4TrackVector* secondaries = step->GetSecondary();
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
