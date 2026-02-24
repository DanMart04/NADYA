#ifndef SIZES_HH
#define SIZES_HH

#include <G4SystemOfUnits.hh>
#include <algorithm>

namespace Sizes
{
    const G4bool viewMode = false;

    namespace Instrument {
        const G4double sizeX = 100.0 * mm;
        const G4double sizeY = 100.0 * mm;
        const G4double sizeZ = 100.0 * mm;
        const G4double halfX = sizeX / 2.0;
        const G4double halfY = sizeY / 2.0;
        const G4double halfZ = sizeZ / 2.0;
        const G4double bottomZ = 0.0 * mm;
        const G4double topZ = bottomZ + sizeZ;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    namespace Stack {
        const G4double caloBottomZ = 0.0 * mm;
        const G4double gapCaloToTrigger2 = 0.0 * mm;
        const G4double gapTrigger2ToFiber = 0.0 * mm;
        const G4double gapFiberToTrigger1 = 0.0 * mm;
        const G4double gapInsideTriggerPair = 0.0 * mm;
    }

    namespace Calorimeter {
        const G4double crystalWidth = 25.0 * mm;
        const G4double crystalLength = 25.0 * mm;
        const G4double crystalHeight = 20.0 * mm;
        const G4int rowsX = 2;
        const G4int rowsY = 2;
        const G4double gap = 0.0 * mm;
        const G4double bottomZ = Stack::caloBottomZ;
        const G4double topZ = bottomZ + crystalHeight;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
        inline G4double totalWidth() { return rowsX * crystalWidth + (rowsX - 1) * gap; }
        inline G4double totalLength() { return rowsY * crystalLength + (rowsY - 1) * gap; }
    }

    namespace TOFFibers {
        const G4double sizeX = 50.0 * mm;
        const G4double sizeY = 50.0 * mm;
        const G4double halfX = sizeX / 2.0;
        const G4double halfY = sizeY / 2.0;
        const G4int fiberRowsX = 48;
        const G4int fiberRowsY = 48;
        const G4double fiberRadius = (0.5) * mm;
        const G4double fiberCladThickness = 0.1 * mm;
        const G4double fiberStripGap = 0.0 * mm;
        const G4int fiberLayersPerPlane = 3;
        const G4double layerCenterDist = fiberRadius * 2.0 + fiberStripGap;
        const G4double planeThickness = 2.0 * fiberRadius + (fiberLayersPerPlane - 1) * layerCenterDist;
        const G4int fiberModuleCount = 5;
        const G4double fiberModuleThickness = 2.0 * planeThickness;
        const G4double fiberModuleGap = 0.0 * mm;
        const G4double thickness = fiberModuleCount * fiberModuleThickness + (fiberModuleCount - 1) * fiberModuleGap;
    }

    namespace Trigger {
        const G4int segmentCount = 7;
        const G4double stripWidth = 7.0 * mm;
        const G4double panelSizeX = segmentCount * stripWidth;
        const G4double panelSizeY = segmentCount * stripWidth;
        const G4double halfX = panelSizeX / 2.0;
        const G4double halfY = panelSizeY / 2.0;
        const G4double panelThickness = 7.0 * mm;

        const G4double pair2LowerBottomZ = Calorimeter::topZ + Stack::gapCaloToTrigger2;
        const G4double pair2LowerTopZ = pair2LowerBottomZ + panelThickness;
        const G4double pair2UpperBottomZ = pair2LowerTopZ + Stack::gapInsideTriggerPair;
        const G4double pair2UpperTopZ = pair2UpperBottomZ + panelThickness;

        const G4double fibersBottomZ = pair2UpperTopZ + Stack::gapTrigger2ToFiber;
        const G4double fibersTopZ = fibersBottomZ + TOFFibers::thickness;

        const G4double pair1LowerBottomZ = fibersTopZ + Stack::gapFiberToTrigger1;
        const G4double pair1LowerTopZ = pair1LowerBottomZ + panelThickness;
        const G4double pair1UpperBottomZ = pair1LowerTopZ + Stack::gapInsideTriggerPair;
        const G4double pair1UpperTopZ = pair1UpperBottomZ + panelThickness;
    }

    namespace TOFFibers {
        const G4double bottomZ = Trigger::fibersBottomZ;
        const G4double topZ = Trigger::fibersTopZ;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    namespace Trigger1Lower {
        inline G4double centerZ() { return (Trigger::pair1LowerTopZ + Trigger::pair1LowerBottomZ) / 2.0; }
    }

    namespace Trigger1Upper {
        inline G4double centerZ() { return (Trigger::pair1UpperTopZ + Trigger::pair1UpperBottomZ) / 2.0; }
    }

    namespace Trigger2Lower {
        inline G4double centerZ() { return (Trigger::pair2LowerTopZ + Trigger::pair2LowerBottomZ) / 2.0; }
    }

    namespace Trigger2Upper {
        inline G4double centerZ() { return (Trigger::pair2UpperTopZ + Trigger::pair2UpperBottomZ) / 2.0; }
    }

    namespace VetoAC {
        const G4double outerHalfX = 42.0 * mm;
        const G4double outerHalfY = 42.0 * mm;
        const G4double thickness = 7.0 * mm;
        const G4double innerHalfX = outerHalfX - thickness;
        const G4double innerHalfY = outerHalfY - thickness;
        const G4double bottomZ = Trigger::pair2LowerBottomZ;
        const G4double topZ = Trigger::pair1UpperTopZ;
        inline G4double height() { return topZ - bottomZ; }
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    namespace Envelope {
        const G4double marginXY = 0.0 * mm;
        const G4double marginZ = 0.0 * mm;
        const G4double halfX = std::max(50.0 * mm, VetoAC::outerHalfX + marginXY);
        const G4double halfY = std::max(50.0 * mm, VetoAC::outerHalfY + marginXY);
        const G4double bottomZ = Calorimeter::bottomZ - marginZ;
        const G4double topZ = std::max(Trigger::pair1UpperTopZ, VetoAC::topZ) + marginZ;
        const G4double sizeZ = topZ - bottomZ;
        const G4double halfZ = sizeZ / 2.0;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    namespace CubeInner {
        const G4double halfX = Envelope::halfX;
        const G4double halfY = Envelope::halfY;
        const G4double bottomZ = Envelope::bottomZ;
        const G4double topZ = Envelope::topZ;
        inline G4double height() { return topZ - bottomZ; }
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }
}

#endif // SIZES_HH
