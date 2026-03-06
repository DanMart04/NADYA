#pragma once

#include "Spectrum.hh"

class PowerLawSpectrum : public Spectrum {
public:
    explicit PowerLawSpectrum(G4double cThreshold);

private:
    G4double alpha{};

    G4double SampleEnergy() override;
};
