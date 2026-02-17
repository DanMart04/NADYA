#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include <G4VUserDetectorConstruction.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4NistManager.hh>
#include <G4Box.hh>
#include <G4VisAttributes.hh>
#include <G4SystemOfUnits.hh>
#include <G4SDManager.hh>
#include <G4LogicalVolumeStore.hh>
#include <algorithm>
#include <string>

#include "Detector.hh"
#include "SensitiveDetector.hh"
#include "Sizes.hh"

class Geometry : public G4VUserDetectorConstruction {
public:
    Geometry();
    ~Geometry() override = default;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

private:
    G4NistManager* nist{};
    G4Material* worldMat{};

    G4LogicalVolume* worldLV{};
    G4VPhysicalVolume* worldPV{};

    Detector* detector{};

    // Sensitive LVs
    G4LogicalVolume* tofTopLV{};
    G4LogicalVolume* tofBottomLV{};
    G4LogicalVolume* vetoLV{};
    G4LogicalVolume* coordDetectorLV{};
    G4LogicalVolume* copperPlateLV{};
    G4LogicalVolume* fiberStripLV{};
    G4LogicalVolume* calorimeterLV{};
    G4LogicalVolume* bottomVolumeLV{};
};

#endif // GEOMETRY_HH
