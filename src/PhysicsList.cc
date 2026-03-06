#include "PhysicsList.hh"
#include <G4EmStandardPhysics_option4.hh>
#include <G4OpticalPhysics.hh>
#include <G4OpticalParameters.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4RadioactiveDecayPhysics.hh>

PhysicsList::PhysicsList(const G4bool useOptics)
    : FTFP_BERT(0)
{
    ReplacePhysics(new G4EmStandardPhysics_option4());
    RegisterPhysics(new G4RadioactiveDecayPhysics());

    if (useOptics) {
        G4OpticalParameters* op = G4OpticalParameters::Instance();
        op->SetProcessActivation("Cerenkov", true);
        op->SetProcessActivation("Scintillation", true);
        op->SetProcessActivation("OpAbsorption", true);
        op->SetProcessActivation("OpRayleigh", true);
        op->SetProcessActivation("OpMieHG", true);
        op->SetProcessActivation("OpBoundary", true);
        op->SetScintTrackSecondariesFirst(true);
        op->SetCerenkovTrackSecondariesFirst(false);
        // Регистрация G4OpticalPhysics обязательно после настроек G4OpticalParameters
        RegisterPhysics(new G4OpticalPhysics());
    }

    RegisterPhysics(new G4StepLimiterPhysics());
}

PhysicsList::~PhysicsList() = default;
