#include "Geometry.hh"

using namespace Sizes;

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
    tofTopLV        = detector->GetTOFTopLV();
    tofBottomLV     = detector->GetTOFBottomLV();
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

    if (tofTopLV) {
        auto* tofTopSD = new SensitiveDetector("TOFTopSD", 0, "TOFTop");
        sdManager->AddNewDetector(tofTopSD);
        tofTopLV->SetSensitiveDetector(tofTopSD);
    }

    if (tofBottomLV) {
        auto* tofBottomSD = new SensitiveDetector("TOFBottomSD", 1, "TOFBottom");
        sdManager->AddNewDetector(tofBottomSD);
        tofBottomLV->SetSensitiveDetector(tofBottomSD);
    }

    if (vetoLV) {
        auto* vetoSD = new SensitiveDetector("VetoSD", 2, "Veto");
        sdManager->AddNewDetector(vetoSD);
        vetoLV->SetSensitiveDetector(vetoSD);
    }

    if (coordDetectorLV) {
        auto* coordSD = new SensitiveDetector("CoordSD", 3, "TOFFibers");
        sdManager->AddNewDetector(coordSD);
        coordDetectorLV->SetSensitiveDetector(coordSD);
    }

    if (fiberStripLV) {
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
        auto* bottomSD = new SensitiveDetector("BottomSD", 7, "BottomVolume");
        sdManager->AddNewDetector(bottomSD);
        bottomVolumeLV->SetSensitiveDetector(bottomSD);
    }
}
