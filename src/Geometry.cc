#include "Geometry.hh"
#include <G4UserLimits.hh>

using namespace Sizes;

namespace {
const G4double triggerStepMax = 1.0 * mm;
const G4double fiberStepMax = 0.2 * mm;
}

Geometry::Geometry() {
    nist = G4NistManager::Instance();
}

G4VPhysicalVolume* Geometry::Construct() {
    worldMat = nist->FindOrBuildMaterial("G4_Galactic");

    const G4double maxR = std::max(Envelope::halfX, Envelope::halfY);
    const G4double zMax = Envelope::topZ;
    const G4double zMin = Envelope::bottomZ;
    const G4double halfWorld = 3.0 * std::max({maxR, std::abs(zMax), std::abs(zMin)});

    auto* worldBox = new G4Box("World", halfWorld, halfWorld, halfWorld);
    worldLV = new G4LogicalVolume(worldBox, worldMat, "WorldLV");
    worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

    worldPV = new G4PVPlacement(nullptr,
                                G4ThreeVector(0, 0, 0),
                                worldLV,
                                "WorldPV",
                                nullptr,
                                false, 0, false);

    detector = new Detector(worldLV, nist);
    detector->Construct();

    // Sensitive
    trigger1LowerLV = detector->GetTrigger1LowerLV();
    trigger1UpperLV = detector->GetTrigger1UpperLV();
    trigger2LowerLV = detector->GetTrigger2LowerLV();
    trigger2UpperLV = detector->GetTrigger2UpperLV();
    vetoLV          = detector->GetVetoLV();
    coordDetectorLV = detector->GetCoordDetectorLV();
    copperPlateLV   = detector->GetCopperPlateLV();
    fiberStripLV    = detector->GetFiberStripLV();
    calorimeterLV   = detector->GetCalorimeterLV();
    bottomVolumeLV  = detector->GetBottomVolumeLV();

    return worldPV;
}

void Geometry::ConstructSDandField() {
    auto* sdManager = G4SDManager::GetSDMpointer();

    auto* triggerLimits = new G4UserLimits(triggerStepMax);
    auto* fiberLimits = new G4UserLimits(fiberStepMax);

    if (trigger1LowerLV) {
        trigger1LowerLV->SetUserLimits(triggerLimits);
        auto* trigger1LowerSD = new SensitiveDetector("Trigger1LowerSD", 0, "Trigger1Lower");
        sdManager->AddNewDetector(trigger1LowerSD);
        trigger1LowerLV->SetSensitiveDetector(trigger1LowerSD);
    }

    if (trigger1UpperLV) {
        trigger1UpperLV->SetUserLimits(triggerLimits);
        auto* trigger1UpperSD = new SensitiveDetector("Trigger1UpperSD", 1, "Trigger1Upper");
        sdManager->AddNewDetector(trigger1UpperSD);
        trigger1UpperLV->SetSensitiveDetector(trigger1UpperSD);
    }

    if (trigger2LowerLV) {
        trigger2LowerLV->SetUserLimits(triggerLimits);
        auto* trigger2LowerSD = new SensitiveDetector("Trigger2LowerSD", 4, "Trigger2Lower");
        sdManager->AddNewDetector(trigger2LowerSD);
        trigger2LowerLV->SetSensitiveDetector(trigger2LowerSD);
    }

    if (trigger2UpperLV) {
        trigger2UpperLV->SetUserLimits(triggerLimits);
        auto* trigger2UpperSD = new SensitiveDetector("Trigger2UpperSD", 7, "Trigger2Upper");
        sdManager->AddNewDetector(trigger2UpperSD);
        trigger2UpperLV->SetSensitiveDetector(trigger2UpperSD);
    }

    if (vetoLV) {
        auto* vetoSD = new SensitiveDetector("VetoSD", 2, "Veto");
        sdManager->AddNewDetector(vetoSD);
        vetoLV->SetSensitiveDetector(vetoSD);
    }

    if (coordDetectorLV) {
        coordDetectorLV->SetUserLimits(fiberLimits);
        auto* coordSD = new SensitiveDetector("CoordSD", 3, "TOFFibers");
        sdManager->AddNewDetector(coordSD);
        coordDetectorLV->SetSensitiveDetector(coordSD);
    }

    if (fiberStripLV) {
        fiberStripLV->SetUserLimits(fiberLimits);
        auto* fiberSD = new SensitiveDetector("FiberSD", 5, "Fiber");
        sdManager->AddNewDetector(fiberSD);
        fiberStripLV->SetSensitiveDetector(fiberSD);
    }

    auto* caloSD = new SensitiveDetector("CalorimeterSD", 6, "Calorimeter");
    sdManager->AddNewDetector(caloSD);

    auto* lvStore = G4LogicalVolumeStore::GetInstance();
    for (G4int ix = 0; ix < Calorimeter::rowsX; ++ix) {
        for (G4int iy = 0; iy < Calorimeter::rowsY; ++iy) {
            G4String crystalName = "CsICrystal_" + std::to_string(ix) + "_" + std::to_string(iy) + "LV";
            G4LogicalVolume* crystalLV = lvStore->GetVolume(crystalName, false);
            if (crystalLV) {
                crystalLV->SetSensitiveDetector(caloSD);
            }
        }
    }

    if (bottomVolumeLV) {
        auto* bottomSD = new SensitiveDetector("BottomSD", 8, "BottomVolume");
        sdManager->AddNewDetector(bottomSD);
        bottomVolumeLV->SetSensitiveDetector(bottomSD);
    }
}
