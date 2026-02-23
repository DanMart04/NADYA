#ifndef DETECTOR_HH
#define DETECTOR_HH

#include <G4VisAttributes.hh>
#include <G4SubtractionSolid.hh>
#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4MaterialPropertiesTable.hh>
#include <G4Element.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <vector>
#include <algorithm>

#include "Sizes.hh"

class Detector {
public:
    G4NistManager* nist;

    // Visual attributes
    G4VisAttributes* visTOF{};
    G4VisAttributes* visVeto{};
    G4VisAttributes* visCoordDetector{};
    G4VisAttributes* visFiber{};
    G4VisAttributes* visCalorimeter{};

    Detector(G4LogicalVolume* worldLV, G4NistManager* nistMan);
    ~Detector() = default;

    void DefineMaterials();
    void DefineVisual();
    void Construct();

    // Getters for sensitive volumes
    [[nodiscard]] G4LogicalVolume* GetTrigger1LowerLV() const { return trigger1LowerLV; }
    [[nodiscard]] G4LogicalVolume* GetTrigger1UpperLV() const { return trigger1UpperLV; }
    [[nodiscard]] G4LogicalVolume* GetTrigger2LowerLV() const { return trigger2LowerLV; }
    [[nodiscard]] G4LogicalVolume* GetTrigger2UpperLV() const { return trigger2UpperLV; }
    [[nodiscard]] G4LogicalVolume* GetVetoLV() const { return vetoLV; }
    [[nodiscard]] G4LogicalVolume* GetCoordDetectorLV() const { return coordDetectorLV; }
    [[nodiscard]] G4LogicalVolume* GetCopperPlateLV() const { return copperPlateLV; }
    [[nodiscard]] G4LogicalVolume* GetFiberStripLV() const { return fiberCoreLV; }
    [[nodiscard]] G4LogicalVolume* GetCalorimeterLV() const { return calorimeterLV; }
    [[nodiscard]] G4LogicalVolume* GetBottomVolumeLV() const { return bottomVolumeLV; }

private:
    G4LogicalVolume* worldLV;

    // Materials
    G4Material* airMat{};
    G4Material* tofMat{};
    G4Material* vetoMat{};
    G4Material* fiberMat{};
    G4Material* fiberCladMat{};
    G4Material* calorimeterMat{};

    // Logical volumes (sensitive)
    G4LogicalVolume* trigger1LowerLV{};
    G4LogicalVolume* trigger1UpperLV{};
    G4LogicalVolume* trigger2LowerLV{};
    G4LogicalVolume* trigger2UpperLV{};
    G4LogicalVolume* vetoLV{};
    G4LogicalVolume* coordDetectorLV{};
    G4LogicalVolume* copperPlateLV{};
    G4LogicalVolume* fiberCoreLV{};
    G4LogicalVolume* calorimeterLV{};
    G4LogicalVolume* bottomVolumeLV{};

    // Containers
    G4LogicalVolume* cubeOuterLV{};

    // Build steps
    void ConstructShellAndContainers();
    void ConstructVeto();
    void ConstructTOF();
    void ConstructTOFFibers();
    void ConstructCalorimeter();
};

#endif // DETECTOR_HH
