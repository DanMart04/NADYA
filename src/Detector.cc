#include "Detector.hh"

namespace {
inline G4double ZInInstrument(G4double zWorld) {
    return zWorld - Sizes::Envelope::centerZ();
}
}

using namespace Sizes;

Detector::Detector(G4LogicalVolume* wLV, G4NistManager* nistMan)
    : worldLV(wLV), nist(nistMan) {
    DefineMaterials();
    DefineVisual();
}

void Detector::DefineMaterials() {
    airMat = nist->FindOrBuildMaterial("G4_AIR");
    tofMat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    vetoMat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    fiberMat = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    fiberCladMat = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    calorimeterMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
}

void Detector::DefineVisual() {
    visTOF = new G4VisAttributes(G4Color(0.0, 1.0, 0.0, 0.7));
    visTOF->SetForceSolid(true);

    visVeto = new G4VisAttributes(G4Color(1.0, 0.0, 0.0, 0.55));
    visVeto->SetForceSolid(true);

    visCoordDetector = new G4VisAttributes(G4Color(0.0, 0.5, 1.0, 0.8));
    visCoordDetector->SetForceSolid(true);

    visFiber = new G4VisAttributes(G4Color(0.0, 0.7, 0.9, 0.85));
    visFiber->SetForceSolid(true);

    visCalorimeter = new G4VisAttributes(G4Color(0.6, 0.2, 0.8, 0.75));
    visCalorimeter->SetForceSolid(true);
}

void Detector::Construct() {
    ConstructShellAndContainers();
    ConstructVeto();
    ConstructTOF();
    ConstructTOFFibers();
    ConstructCalorimeter();
}

void Detector::ConstructShellAndContainers() {
    const G4bool checkOverlaps = true;

    auto* instrumentSolid = new G4Box("InstrumentSolid",
                                      Envelope::halfX,
                                      Envelope::halfY,
                                      Envelope::halfZ);
    cubeOuterLV = new G4LogicalVolume(instrumentSolid, airMat, "InstrumentLV");
    cubeOuterLV->SetVisAttributes(G4VisAttributes::GetInvisible());
    new G4PVPlacement(nullptr,
                      G4ThreeVector(0, 0, Envelope::centerZ()),
                      cubeOuterLV,
                      "InstrumentPV",
                      worldLV,
                      false,
                      0,
                      checkOverlaps);

}

void Detector::ConstructVeto() {
    const G4bool checkOverlaps = true;

    auto* acOuter = new G4Box("ACOuterBox",
                              VetoAC::outerHalfX,
                              VetoAC::outerHalfY,
                              VetoAC::height() / 2.0);
    auto* acInner = new G4Box("ACInnerBox",
                              VetoAC::innerHalfX,
                              VetoAC::innerHalfY,
                              VetoAC::height() / 2.0 + 1.0 * mm);
    auto* acShellSolid = new G4SubtractionSolid("ACShellSolid", acOuter, acInner);

    vetoLV = new G4LogicalVolume(acShellSolid, vetoMat, "ACShellLV");
    vetoLV->SetVisAttributes(visVeto);

    new G4PVPlacement(nullptr,
                      G4ThreeVector(0, 0, ZInInstrument(VetoAC::centerZ())),
                      vetoLV,
                      "ACShellPV",
                      cubeOuterLV,
                      false,
                      0,
                      checkOverlaps);
}

void Detector::ConstructTOF() {
    const G4bool checkOverlaps = true;

    // Lower layer in a pair: strips run along Y (segmentation in X).
    auto* xStripSolid = new G4Box("TriggerXStripSolid",
                                  Trigger::stripWidth / 2.0,
                                  Trigger::halfY,
                                  Trigger::panelThickness / 2.0);
    // Upper layer in a pair: strips run along X (segmentation in Y).
    auto* yStripSolid = new G4Box("TriggerYStripSolid",
                                  Trigger::halfX,
                                  Trigger::stripWidth / 2.0,
                                  Trigger::panelThickness / 2.0);

    trigger1LowerLV = new G4LogicalVolume(xStripSolid, tofMat, "Trigger1LowerStripLV");
    trigger1UpperLV = new G4LogicalVolume(yStripSolid, tofMat, "Trigger1UpperStripLV");
    trigger2LowerLV = new G4LogicalVolume(xStripSolid, tofMat, "Trigger2LowerStripLV");
    trigger2UpperLV = new G4LogicalVolume(yStripSolid, tofMat, "Trigger2UpperStripLV");
    trigger1LowerLV->SetVisAttributes(visTOF);
    trigger1UpperLV->SetVisAttributes(visTOF);
    trigger2LowerLV->SetVisAttributes(visTOF);
    trigger2UpperLV->SetVisAttributes(visTOF);

    for (G4int i = 0; i < Trigger::segmentCount; ++i) {
        const G4double x = -Trigger::halfX + (i + 0.5) * Trigger::stripWidth;
        const G4double y = -Trigger::halfY + (i + 0.5) * Trigger::stripWidth;

        new G4PVPlacement(nullptr,
                          G4ThreeVector(x, 0.0, ZInInstrument(Trigger1Lower::centerZ())),
                          trigger1LowerLV,
                          "Trigger1LowerStripPV",
                          cubeOuterLV,
                          false,
                          i,
                          checkOverlaps);
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0.0, y, ZInInstrument(Trigger1Upper::centerZ())),
                          trigger1UpperLV,
                          "Trigger1UpperStripPV",
                          cubeOuterLV,
                          false,
                          i,
                          checkOverlaps);
        new G4PVPlacement(nullptr,
                          G4ThreeVector(x, 0.0, ZInInstrument(Trigger2Lower::centerZ())),
                          trigger2LowerLV,
                          "Trigger2LowerStripPV",
                          cubeOuterLV,
                          false,
                          i,
                          checkOverlaps);
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0.0, y, ZInInstrument(Trigger2Upper::centerZ())),
                          trigger2UpperLV,
                          "Trigger2UpperStripPV",
                          cubeOuterLV,
                          false,
                          i,
                          checkOverlaps);
    }
}

void Detector::ConstructTOFFibers() {
    const G4bool checkOverlaps = true;

    const G4double moduleHalfZ = TOFFibers::fiberModuleThickness / 2.0;
    const G4double planeHalfZ = moduleHalfZ / 2.0;
    const G4double coreRadius = std::max(0.0 * mm, TOFFibers::fiberRadius - TOFFibers::fiberCladThickness);
    const G4double centerDist = TOFFibers::layerCenterDist;

    auto* moduleSolid = new G4Box("FiberModuleSolid",
                                  TOFFibers::halfX,
                                  TOFFibers::halfY,
                                  moduleHalfZ);
    auto* moduleLV = new G4LogicalVolume(moduleSolid, airMat, "FiberModuleLV");
    moduleLV->SetVisAttributes(G4VisAttributes::GetInvisible());

    auto* planeSolid = new G4Box("FiberPlaneSolid",
                                 TOFFibers::halfX,
                                 TOFFibers::halfY,
                                 planeHalfZ);
    auto* fiberPlaneXLV = new G4LogicalVolume(planeSolid, airMat, "FiberPlaneXLV");
    auto* fiberPlaneYLV = new G4LogicalVolume(planeSolid, airMat, "FiberPlaneYLV");
    fiberPlaneXLV->SetVisAttributes(G4VisAttributes::GetInvisible());
    fiberPlaneYLV->SetVisAttributes(G4VisAttributes::GetInvisible());

    auto* fiberCladX = new G4Tubs("TOFFiberCladdingX", coreRadius, TOFFibers::fiberRadius, TOFFibers::halfY, 0.0, 360.0 * deg);
    auto* fiberCladY = new G4Tubs("TOFFiberCladdingY", coreRadius, TOFFibers::fiberRadius, TOFFibers::halfX, 0.0, 360.0 * deg);
    auto* fiberCoreX = new G4Tubs("TOFFiberCoreX", 0.0, coreRadius, TOFFibers::halfY, 0.0, 360.0 * deg);
    auto* fiberCoreY = new G4Tubs("TOFFiberCoreY", 0.0, coreRadius, TOFFibers::halfX, 0.0, 360.0 * deg);

    auto* fiberCladXLV = new G4LogicalVolume(fiberCladX, fiberCladMat, "TOFFiberCladdingXLV");
    auto* fiberCladYLV = new G4LogicalVolume(fiberCladY, fiberCladMat, "TOFFiberCladdingYLV");
    coordDetectorLV = new G4LogicalVolume(fiberCoreX, fiberMat, "TOFFiberXLV");
    fiberCoreLV = new G4LogicalVolume(fiberCoreY, fiberMat, "TOFFiberYLV");

    auto* visFiberClad = new G4VisAttributes(G4Color(0.25, 0.9, 0.95, 0.35));
    visFiberClad->SetForceSolid(true);
    fiberCladXLV->SetVisAttributes(visFiberClad);
    fiberCladYLV->SetVisAttributes(visFiberClad);
    coordDetectorLV->SetVisAttributes(visCoordDetector);
    fiberCoreLV->SetVisAttributes(visFiber);

    auto* rotAlongY = new G4RotationMatrix();
    rotAlongY->rotateX(90.0 * deg);
    auto* rotAlongX = new G4RotationMatrix();
    rotAlongX->rotateY(90.0 * deg);

    // Конфигурация одного модуля: пара плоскостей X/Y.
    new G4PVPlacement(nullptr,
                      G4ThreeVector(0.0, 0.0, +planeHalfZ),
                      fiberPlaneXLV,
                      "FiberPlaneXPV",
                      moduleLV,
                      false,
                      0,
                      checkOverlaps);
    new G4PVPlacement(nullptr,
                      G4ThreeVector(0.0, 0.0, -planeHalfZ),
                      fiberPlaneYLV,
                      "FiberPlaneYPV",
                      moduleLV,
                      false,
                      0,
                      checkOverlaps);

    const G4double step = 2.0 * TOFFibers::fiberRadius + TOFFibers::fiberStripGap;
    const G4int maxRowsX = static_cast<G4int>(std::floor(TOFFibers::sizeX / step));
    const G4int maxRowsY = static_cast<G4int>(std::floor(TOFFibers::sizeY / step));
    const G4int rowsXBase = std::max(0, std::min(TOFFibers::fiberRowsX, maxRowsX));
    const G4int rowsYBase = std::max(0, std::min(TOFFibers::fiberRowsY, maxRowsY));
    const G4double usedXBase = rowsXBase * step;
    const G4double usedYBase = rowsYBase * step;
    const G4double leftGuard = TOFFibers::fiberRadius;
    const G4double marginXBase = std::max((TOFFibers::sizeX - usedXBase) / 2.0, leftGuard);
    const G4double marginYBase = std::max((TOFFibers::sizeY - usedYBase) / 2.0, leftGuard);

    for (G4int l = 0; l < TOFFibers::fiberLayersPerPlane; ++l) {
        const G4double zLayer = -planeHalfZ + TOFFibers::fiberRadius + l * centerDist;
        const G4double rowShift = (l % 3 == 1) ? (-TOFFibers::fiberRadius) : 0.0;
        const G4int rowsX = rowsXBase;
        const G4int rowsY = rowsYBase;

        for (G4int i = 0; i < rowsX; ++i) {
            const G4double x = -TOFFibers::halfX + marginXBase + TOFFibers::fiberRadius + rowShift + i * step;
            const G4int copyNo = l * 1000 + i;
            new G4PVPlacement(rotAlongY,
                              G4ThreeVector(x, 0.0, zLayer),
                              fiberCladXLV,
                              "TOFFiberXCladPV",
                              fiberPlaneXLV,
                              false,
                              copyNo,
                              checkOverlaps);
            new G4PVPlacement(rotAlongY,
                              G4ThreeVector(x, 0.0, zLayer),
                              coordDetectorLV,
                              "TOFFiberXCorePV",
                              fiberPlaneXLV,
                              false,
                              copyNo,
                              checkOverlaps);
        }
        for (G4int i = 0; i < rowsY; ++i) {
            const G4double y = -TOFFibers::halfY + marginYBase + TOFFibers::fiberRadius + rowShift + i * step;
            const G4int copyNo = 50000 + l * 1000 + i;
            new G4PVPlacement(rotAlongX,
                              G4ThreeVector(0.0, y, zLayer),
                              fiberCladYLV,
                              "TOFFiberYCladPV",
                              fiberPlaneYLV,
                              false,
                              copyNo,
                              checkOverlaps);
            new G4PVPlacement(rotAlongX,
                              G4ThreeVector(0.0, y, zLayer),
                              fiberCoreLV,
                              "TOFFiberYCorePV",
                              fiberPlaneYLV,
                              false,
                              copyNo,
                              checkOverlaps);
        }
    }

    // Размещаем модули X+Y последовательно по Z; по умолчанию без зазора.
    for (G4int m = 0; m < TOFFibers::fiberModuleCount; ++m) {
        const G4double moduleCenterZWorld =
            TOFFibers::bottomZ + moduleHalfZ + m * (TOFFibers::fiberModuleThickness + TOFFibers::fiberModuleGap);
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0.0, 0.0, ZInInstrument(moduleCenterZWorld)),
                          moduleLV,
                          "FiberModulePV",
                          cubeOuterLV,
                          false,
                          m,
                          checkOverlaps);
    }
}

void Detector::ConstructCalorimeter() {
    const G4bool checkOverlaps = true;

    auto* crystalSolid = new G4Box("CsICrystalSolid",
                                   Calorimeter::crystalWidth / 2.0,
                                   Calorimeter::crystalLength / 2.0,
                                   Calorimeter::crystalHeight / 2.0);

    for (G4int ix = 0; ix < Calorimeter::rowsX; ++ix) {
        for (G4int iy = 0; iy < Calorimeter::rowsY; ++iy) {
            const G4String crystalName = "CsICrystal_" + std::to_string(ix) + "_" + std::to_string(iy);
            auto* crystalLV = new G4LogicalVolume(crystalSolid, calorimeterMat, crystalName + "LV");
            crystalLV->SetVisAttributes(visCalorimeter);

            const G4double x = -Calorimeter::totalWidth() / 2.0
                               + (ix + 0.5) * Calorimeter::crystalWidth
                               + ix * Calorimeter::gap;
            const G4double y = -Calorimeter::totalLength() / 2.0
                               + (iy + 0.5) * Calorimeter::crystalLength
                               + iy * Calorimeter::gap;

            new G4PVPlacement(nullptr,
                              G4ThreeVector(x, y, ZInInstrument(Calorimeter::centerZ())),
                              crystalLV,
                              crystalName + "PV",
                              cubeOuterLV,
                              false,
                              ix * 10 + iy,
                              checkOverlaps);

            if (ix == 0 && iy == 0) {
                calorimeterLV = crystalLV;
            }
        }
    }
}

