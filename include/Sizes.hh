#ifndef SIZES_HH
#define SIZES_HH

#include <G4SystemOfUnits.hh>
#include <cmath>
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
        const G4double layerGapCsIToFiber = 0.0 * mm;
        const G4double layerGapFiberToTOF = 0.0 * mm;
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

        // Ряды в одном модуле
        const G4int fiberRowsX = 12;
        const G4int fiberRowsY = 12;
        const G4double fiberRadius = (1.7) * mm;
        const G4double fiberCladThickness = 0.2 * mm;
        const G4double fiberStripGap = 0.0 * mm;
        const G4int fiberLayersPerPlane = 3;

        const G4double layerCenterDist = fiberRadius * 2.0 + fiberStripGap;
        const G4double planeThickness =
            2.0 * fiberRadius + (fiberLayersPerPlane - 1) * layerCenterDist;

        const G4int fiberModuleCount = 3;
        const G4double fiberModuleThickness = 2.0 * planeThickness;
        const G4double fiberModuleGap = 0.0 * mm;
        const G4double thickness =
            fiberModuleCount * fiberModuleThickness + (fiberModuleCount - 1) * fiberModuleGap;

        const G4double bottomZ = Calorimeter::topZ + Stack::layerGapCsIToFiber;
        const G4double topZ = bottomZ + thickness;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    namespace TOF {
        const G4double panelSizeX = 50.0 * mm;
        const G4double panelSizeY = 50.0 * mm;
        const G4double halfX = panelSizeX / 2.0;
        const G4double halfY = panelSizeY / 2.0;
        const G4double panelThickness = 5.0 * mm;
        const G4int segmentCount = 5;
        const G4double interPanelGap = 10.0 * mm;  // регулируется отдельно
        const G4double stripWidth = panelSizeX / segmentCount;
        const G4double lowerBottomZ = TOFFibers::topZ + Stack::layerGapFiberToTOF;
        const G4double lowerTopZ = lowerBottomZ + panelThickness;
        const G4double upperBottomZ = lowerTopZ + interPanelGap;
        const G4double upperTopZ = upperBottomZ + panelThickness;
    }

    namespace TOFBottom {
        const G4double halfX = TOF::halfX;
        const G4double halfY = TOF::halfY;
        const G4double thickness = TOF::panelThickness;
        const G4double bottomZ = TOF::lowerBottomZ;
        const G4double topZ = TOF::lowerTopZ;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    namespace TOFTop {
        const G4double halfX = TOF::halfX;
        const G4double halfY = TOF::halfY;
        const G4double thickness = TOF::panelThickness;
        const G4double bottomZ = TOF::upperBottomZ;
        const G4double topZ = TOF::upperTopZ;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    namespace VetoAC {
        const G4double outerHalfX = 50.0 * mm;
        const G4double outerHalfY = 50.0 * mm;
        const G4double thickness = 7.0 * mm;
        const G4double innerHalfX = outerHalfX - thickness;
        const G4double innerHalfY = outerHalfY - thickness;
        const G4double bottomZ = TOFFibers::bottomZ;
        const G4double topZ = TOFTop::topZ;
        inline G4double height() { return topZ - bottomZ; }
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    // Габаритный контейнер строится по фактической высоте стека.
    namespace Envelope {
        const G4double marginXY = 0.0 * mm;
        const G4double marginZ = 0.0 * mm;
        const G4double halfX = std::max(50.0 * mm, VetoAC::outerHalfX + marginXY);
        const G4double halfY = std::max(50.0 * mm, VetoAC::outerHalfY + marginXY);
        const G4double bottomZ = Calorimeter::bottomZ - marginZ;
        const G4double topZ = std::max(TOFTop::topZ, VetoAC::topZ) + marginZ;
        const G4double sizeZ = topZ - bottomZ;
        const G4double halfZ = sizeZ / 2.0;
        inline G4double centerZ() { return (topZ + bottomZ) / 2.0; }
    }

    // Совместимость: используется в PrimaryGeneratorAction.
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
