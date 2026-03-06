#pragma once

#include "Spectrum.hh"

class UniformSpectrum : public Spectrum {
public:
    explicit UniformSpectrum(G4double cThreshold);

private:
    std::vector<G4String> particles;
    std::vector<G4double> fractions;
    std::vector<G4double> EminVec;
    std::vector<G4double> EmaxVec;
    G4double eCrystalThreshold;

    static std::vector<G4String> Split(const G4String &line);
    static std::vector<G4double> ParseDoubles(const G4String &line);
    size_t SampleIndex() const;

    G4double SampleEnergy() override;
};
