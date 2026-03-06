#pragma once
#include <FTFP_BERT.hh>

class PhysicsList : public FTFP_BERT {
public:
    explicit PhysicsList(G4bool useOptics = false);
    ~PhysicsList() override;
};
