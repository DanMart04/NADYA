#include <G4MTRunManager.hh>
#include <G4UIExecutive.hh>
#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#include <G4UImanager.hh>
#include <FTFP_BERT.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4OpticalPhysics.hh>
#include <G4OpticalParameters.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4RadioactiveDecayPhysics.hh>
#include <Randomize.hh>
#include <CLHEP/Random/RanecuEngine.h>

#include "Geometry.hh"
#include "Sizes.hh"
#include "Configuration.hh"
#include "ActionInitialization.hh"
#include "RunAction.hh"
#include "CountRates.hh"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace Configuration;

namespace {
    // ---------- Вспомогательные функции ----------
    std::string Trim(std::string st) {
        auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        st.erase(st.begin(), std::find_if(st.begin(), st.end(), notSpace));
        st.erase(std::find_if(st.rbegin(), st.rend(), notSpace).base(), st.end());
        return st;
    }

    std::vector<G4String> Split(const G4String& line) {
        std::vector<G4String> result;
        std::stringstream ss(line);
        G4String token;
        while (std::getline(ss, token, ',')) {
            token = Trim(token);
            if (!token.empty())
                result.push_back(token);
        }
        return result;
    }

    std::string ReadValue(const std::string& key, const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            G4Exception("ReadValue", "FILE_OPEN_FAIL",
                        FatalException, ("Cannot open " + filepath).c_str());
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.find(key) != std::string::npos) {
                return line.substr(key.length() + 1);
            }
        }
        return "";
    }

    // ---------- Сохранение конфигурации ----------
    void SaveConfig(const std::string& macroFile,
                    const std::string& configPath,
                    double area,
                    const std::vector<double>& effArea,
                    const std::vector<double>& effAreaOpt,
                    G4int crystalOnly, G4int crystalAndVeto,
                    G4int crystalOnlyOpt, G4int crystalAndVetoOpt) {

        const int N = std::stoi(ReadValue("/run/beamOn", macroFile));

        EnergyRange er{};
        FluxType fType{};
        FluxParams fp{};

        if (fluxType == "PLAW") {
            fType = FluxType::PLAW;
            fp.A = std::stod(ReadValue("A:", configPath));
            fp.alpha = std::stod(ReadValue("alpha:", configPath));
            fp.E_piv = std::stod(ReadValue("E_Piv:", configPath));
            er.Emin = std::stod(ReadValue("E_min:", configPath));
            er.Emax = std::stod(ReadValue("E_max:", configPath));
        } else if (fluxType == "COMP") {
            fType = FluxType::COMP;
            fp.A = std::stod(ReadValue("A:", configPath));
            fp.alpha = std::stod(ReadValue("alpha:", configPath));
            fp.E_piv = std::stod(ReadValue("E_Piv:", configPath));
            fp.E_peak = std::stod(ReadValue("E_Peak:", configPath));
            er.Emin = std::stod(ReadValue("E_min:", configPath));
            er.Emax = std::stod(ReadValue("E_max:", configPath));
        } else if (fluxType == "SEP") {
            fType = FluxType::SEP;
            fp.sep_year = std::stoi(ReadValue("year:", configPath));
            fp.sep_order = std::stoi(ReadValue("order:", configPath));
            fp.sep_csv_path = "../SEP_coefficients.CSV";
            er.Emin = std::stod(ReadValue("E_min:", configPath));
            er.Emax = std::stod(ReadValue("E_max:", configPath));
        } else if (fluxType == "Galactic") {
            fType = FluxType::GALACTIC;
            fp.phiMV = std::stod(ReadValue("phiMV:", configPath));
            fp.particle = ReadValue("particle:", configPath);
            er.Emin = std::stod(ReadValue("E_min:", configPath));
            er.Emax = std::stod(ReadValue("E_max:", configPath));
        } else if (fluxType == "Table") {
            fType = FluxType::TABLE;
            fp.particle = ReadValue("particle:", configPath);
            fp.table_path = ReadValue("table_path:", configPath);
            er.Emin = std::stod(ReadValue("E_min:", configPath));
            er.Emax = std::stod(ReadValue("E_max:", configPath));
        } else {
            fType = FluxType::UNIFORM;
            er.Emin = std::stod(ReadValue("E_min:", configPath));
            er.Emax = std::stod(ReadValue("E_max:", configPath));
        }

        RateCounts counts{crystalOnly, crystalAndVeto};
        RateCounts countsOpt{crystalOnlyOpt, crystalAndVetoOpt};

        RateResult rr{};
        bool rate_ok = true;
        try {
            rr = computeRate(fType, fp, er, area, N, counts);
        } catch (const std::exception& ex) {
            rate_ok = false;
        }

        RateResult rrReal{};
        bool rate_real_ok = true;
        try {
            rrReal = computeRateReal(fType, fp, er, effArea, nBins);
        } catch (const std::exception& ex) {
            rate_real_ok = false;
        }

        RateResult rr_opt{};
        bool rate_opt_ok = useOptics;
        try {
            rr_opt = computeRate(fType, fp, er, area, N, countsOpt);
        } catch (const std::exception& ex) {
            rate_opt_ok = false;
        }

        RateResult rrReal_opt{};
        bool rate_real_opt_ok = useOptics;
        try {
            rrReal_opt = computeRateReal(fType, fp, er, effAreaOpt, nBins);
        } catch (const std::exception& ex) {
            rate_real_opt_ok = false;
        }

        std::ostringstream buf;

        buf << "N: " << N << "\n\n";
        buf << "Detector_type: " << detectorType << "\n";
        buf << "Crystal_SiPM_configuration: " << crystalSiPMConfig << "\n";
        buf << "Tyvek_surface: " << (polishedTyvek ? "polished" : "diffuse") << "\n\n";
        buf << "Use_optics: " << useOptics << "\n\n";
        buf << "Flux_type: " << fluxType << "\n";
        buf << "Flux_dir: " << fluxDirection << "\n";

        buf << "Flux_params:\n{\n\t";
        if (fluxType == "PLAW") {
            buf << "A: " << std::stod(ReadValue("A:", configPath)) << ",\n\t";
            buf << "alpha: " << std::stod(ReadValue("alpha:", configPath)) << ",\n\t";
            buf << "E_Piv: " << std::stod(ReadValue("E_Piv:", configPath)) << "\n";
        } else if (fluxType == "COMP") {
            buf << "A: " << std::stod(ReadValue("A:", configPath)) << ",\n\t";
            buf << "alpha: " << std::stod(ReadValue("alpha:", configPath)) << ",\n\t";
            buf << "E_Piv: " << std::stod(ReadValue("E_Piv:", configPath)) << ",\n\t";
            buf << "E_Peak: " << std::stod(ReadValue("E_Peak:", configPath)) << "\n";
        } else if (fluxType == "SEP") {
            buf << "year: " << std::stoi(ReadValue("year:", configPath)) << ",\n\t";
            buf << "order: " << std::stoi(ReadValue("order:", configPath)) << "\n";
        } else if (fluxType == "Galactic") {
            buf << "phiMV: " << std::stod(ReadValue("phiMV:", configPath)) << ",\n\t";
            buf << "particle: " << ReadValue("particle:", configPath) << "\n";
        } else if (fluxType == "Table") {
            buf << "table_path: " << ReadValue("table_path:", configPath) << ",\n\t";
            buf << "particle: " << ReadValue("particle:", configPath) << "\n";
        } else if (fluxType == "Uniform") {
            buf << "fractions: " << ReadValue("fractions:", configPath) << "\n";
        }
        buf << "}\n\n";

        buf << "Particles: [";
        if (fluxType == "PLAW") {
            buf << "gamma";
        } else if (fluxType == "SEP") {
            buf << "proton";
        } else if (fluxType == "Galactic" || fluxType == "Table") {
            buf << ReadValue("particle:", configPath);
        } else if (fluxType == "Uniform") {
            buf << ReadValue("particles:", configPath);
        }
        buf << "]\n";

        buf << "Energies:\n{\n\t";
        if (fluxType == "PLAW" || fluxType == "COMP") {
            buf << "gamma: ";
        } else if (fluxType == "SEP") {
            buf << "proton: ";
        } else if (fluxType == "Galactic" || fluxType == "Table") {
            buf << ReadValue("particle:", configPath) << ": ";
        } else if (fluxType == "Uniform") {
            std::vector<G4String> particles = Split(ReadValue("particles:", configPath));
            std::vector<G4String> EminVec = Split(ReadValue("E_min:", configPath));
            std::vector<G4String> EmaxVec = Split(ReadValue("E_max:", configPath));
            for (size_t i = 0; i < particles.size(); i++) {
                buf << (i == 0 ? "" : "\t") << particles[i] << ": (" << EminVec[i] << ", " << EmaxVec[i] << "),\n";
            }
        }
        if (fluxType != "Uniform") {
            buf << "(" << ReadValue("E_min:", configPath) << ", " << ReadValue("E_max:", configPath) << ")\n";
        }
        buf << "}\n\n";

        buf << "Counts:\n{\n\t";
        buf << "Crystal_only: " << crystalOnly << "\n\t";
        buf << "Veto_then_Crystal: " << crystalAndVeto << "\n}\n\n";

        buf << "Optical_Counts:\n{\n\t";
        buf << "Crystal_only: " << crystalOnlyOpt << "\n\t";
        buf << "Veto_then_Crystal: " << crystalAndVetoOpt << "\n}\n\n";

        buf << "Thresholds:\n{\n\t";
        buf << std::fixed << std::setprecision(6);
        if (rate_ok) {
            buf << "Crystal: " << eCrystalThreshold << "\n\t";
            buf << "Veto: " << eVetoThreshold << "\n";
        }
        buf << "}\n\n";

        buf << "Optical_thresholds:\n{\n\t";
        buf << std::fixed << std::setprecision(6);
        if (rate_ok) {
            buf << "Crystal: " << oCrystalThreshold << "\n\t";
            buf << "Veto: " << oVetoThreshold << "\n\t";
            buf << "BottomVeto: " << oBottomVetoThreshold << "\n";
        }
        buf << "}\n\n";

        buf << "Rates:\n{\n\t";
        buf << std::fixed << std::setprecision(6);
        if (rate_ok) {
            buf << "Area: " << area << "\n\t";
            buf << "Integral: " << rr.integral << "\n\t";
            buf << "Ndot: " << rr.Ndot << "\n\t";
            buf << "Rate_Crystal_only: " << rr.rateCrystal << "\n\t";
            buf << "Rate_Both: " << rr.rateBoth << "\n\t";
        } else {
            buf << "Area: NaN\n\t";
            buf << "Integral: NaN\n\t";
            buf << "Ndot: NaN\n\t";
            buf << "Rate_Crystal_only: NaN\n\t";
            buf << "Rate_Both: NaN\n\t";
        }
        if (rate_real_ok) {
            buf << "Rate_Real: " << rrReal.rateRealCrystal << "\n";
        } else {
            buf << "Rate_Real: NaN\n";
        }
        buf << "}\n\n";

        buf << "Optical_rates:\n{\n\t";
        buf << std::fixed << std::setprecision(6);
        if (rate_opt_ok) {
            buf << "Area: " << area << "\n\t";
            buf << "Integral: " << rr_opt.integral << "\n\t";
            buf << "Ndot: " << rr_opt.Ndot << "\n\t";
            buf << "Rate_Crystal_only: " << rr_opt.rateCrystal << "\n\t";
            buf << "Rate_Both: " << rr_opt.rateBoth << "\n\t";
        } else {
            buf << "Area: NaN\n\t";
            buf << "Integral: NaN\n\t";
            buf << "Ndot: NaN\n\t";
            buf << "Rate_Crystal_only: NaN\n\t";
            buf << "Rate_Both: NaN\n\t";
        }
        if (rate_real_opt_ok) {
            buf << "Rate_Real: " << rrReal_opt.rateRealCrystal << "\n";
        } else {
            buf << "Rate_Real: NaN\n";
        }
        buf << "}\n\n";

        auto sanitize = [](std::string ss) {
            for (char& c : ss) if (c == ' ') c = '_';
            return ss;
        };

        std::string filename = "info_" + detectorType + "_" + fluxType;
        if (fluxType == "Galactic") {
            const std::string part = ReadValue("particle:", configPath);
            const std::string phi = ReadValue("phiMV:", configPath);
            filename += "_particle:" + part + "_phiMV:" + phi + ".txt";
        } else if (fluxType == "Uniform") {
            const std::string part = ReadValue("particles:", configPath);
            filename += "_particle:" + part + ".txt";
        } else {
            filename += ".txt";
        }
        filename = sanitize(filename);

        std::ofstream out(filename);
        if (!out.is_open()) {
            G4cerr << "Ошибка: не удалось открыть файл " << filename << G4endl;
            return;
        }
        out << buf.str();
        out.close();

        std::cout << "Configuration saved in " << filename << std::endl;
    }
} // namespace

int main(int argc, char **argv) {
    G4String macroFile = "../run.mac";
    G4String geomConfigPath = "../geometry_txt";
    G4int numThreads = G4Threading::G4GetNumberOfCores();
    G4bool useUI = true;

    G4double area = 0.0;
    std::vector<G4double> effArea, effAreaOpt;
    G4int crystalOnly = 0, crystalAndVeto = 0;
    G4int crystalOnlyOpt = 0, crystalAndVetoOpt = 0;

    FluxDir dir = FluxDir::Isotropic; // временно, будет переопределено

    // ---------- Парсинг аргументов командной строки ----------
    for (int i = 0; i < argc; i++) {
        std::string input(argv[i]);
        if (input == "-i" || input == "--input") {
            macroFile = argv[i + 1];
            useUI = false;
            viewDeg = 360 * deg;
        } else if (input == "-t" || input == "--threads") {
            numThreads = std::stoi(argv[i + 1]);
        } else if (input == "-ys" || input == "--yield-scale") {
            yieldScale = std::stoi(argv[i + 1]);
        } else if (input == "--bins") {
            nBins = std::stoi(argv[i + 1]);
        } else if ((input == "-vd" || input == "--view-deg") && useUI) {
            viewDeg = std::stod(argv[i + 1]) * deg;
        } else if (input == "-ct" || input == "--crystal-threshold") {
            eCrystalThreshold = std::stod(argv[i + 1]) * MeV;
        } else if (input == "-vt" || input == "--veto-threshold") {
            eVetoThreshold = std::stod(argv[i + 1]) * MeV;
        } else if (input == "-oct" || input == "--crystal-optic-threshold") {
            oCrystalThreshold = std::stoi(argv[i + 1]);
        } else if (input == "-ovt" || input == "--veto-optic-threshold") {
            oVetoThreshold = std::stoi(argv[i + 1]);
        } else if (input == "-obvt" || input == "--bottom-veto-optic-threshold") {
            oBottomVetoThreshold = std::stoi(argv[i + 1]);
        } else if (input == "-noUI") {
            useUI = false;
        } else if (input == "--polished") {
            polishedTyvek = true;
        } else if (input == "-d" || input == "--detector") {
            detectorType = argv[i + 1];
        } else if (input == "-csc" || input == "--crystal-sipm-config" || input == "-sipm") {
            crystalSiPMConfig = argv[i + 1];
        } else if (input == "-f" || input == "--flux-type") {
            fluxType = argv[i + 1];
        } else if (input == "--flux-dir" || input == "--f-dir" || input == "-fd") {
            fluxDirection = argv[i + 1];
        } else if (input == "--use-optics") {
            useOptics = true;
        } else if (input == "--save-secondaries") {
            saveSecondaries = true;
        } else if (input == "--save-photons") {
            savePhotons = true;
        } else if (input == "-g" || input == "--geom-config") {
            geomConfigPath = argv[i + 1];
        } else if (input == "-o" || input == "--output-file") {
            outputFile = argv[i + 1];
            outputFile += ".root";
        }
    }

    savePhotons = savePhotons && useOptics;

    // Путь к конфигурационному файлу потока
    std::string configPath = "../Flux_config/" + fluxType + "_params.txt";

    // Генератор случайных чисел
    CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);
    CLHEP::HepRandom::setTheSeed(time(nullptr));

    // Создание RunManager (многопоточный, если разрешено)
#ifdef G4MULTITHREADED
    G4MTRunManager* runManager = new G4MTRunManager();
    runManager->SetNumberOfThreads(numThreads);
#else
    G4RunManager* runManager = new G4RunManager();
#endif

    // Геометрия
    Geometry* geometry = new Geometry();
    runManager->SetUserInitialization(geometry);

    // Физика
    FTFP_BERT* physicsList = new FTFP_BERT();
    physicsList->ReplacePhysics(new G4EmStandardPhysics_option4());
    physicsList->ReplacePhysics(new G4RadioactiveDecayPhysics());

    if (useOptics) {
        G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
        G4OpticalParameters* op = G4OpticalParameters::Instance();
        op->SetProcessActivation("Cerenkov", true);
        op->SetProcessActivation("Scintillation", true);
        op->SetProcessActivation("OpAbsorption", true);
        op->SetProcessActivation("OpRayleigh", true);
        op->SetProcessActivation("OpMieHG", true);
        op->SetProcessActivation("OpBoundary", true);
        op->SetScintTrackSecondariesFirst(true);
        op->SetCerenkovTrackSecondariesFirst(false);
        physicsList->RegisterPhysics(opticalPhysics);
    }
    physicsList->RegisterPhysics(new G4StepLimiterPhysics());
    runManager->SetUserInitialization(physicsList);

    // Чтение границ энергий из файла параметров потока
    G4double EminMeV = std::max(std::stod(ReadValue("E_min:", configPath)) * MeV,
                                 eCrystalThreshold);
    G4double EmaxMeV = std::stod(ReadValue("E_max:", configPath)) * MeV;

    // Определение направления потока
    if (fluxDirection == "isotropic") {
        dir = FluxDir::Isotropic;
    } else if (fluxDirection == "isotropic_up") {
        dir = FluxDir::Isotropic_up;
    } else if (fluxDirection == "isotropic_down") {
        dir = FluxDir::Isotropic_down;
    } else if (fluxDirection == "vertical_up") {
        dir = FluxDir::Vertical_up;
    } else if (fluxDirection == "vertical_down") {
        dir = FluxDir::Vertical_down;
    } else if (fluxDirection == "horizontal") {
        dir = FluxDir::Horizontal;
    }

    // Вычисление площади области генерации
    {
        const double halfY_mm = static_cast<double>(std::max(Sizes::Envelope::halfX, Sizes::Envelope::halfY));
        const double sizeZ_mm = static_cast<double>(Sizes::Envelope::sizeZ);
        const double radius_mm = std::sqrt(halfY_mm * halfY_mm + sizeZ_mm * sizeZ_mm) + 5.0;
        area = AreaGen_cm2(halfY_mm, sizeZ_mm, radius_mm, radius_mm, dir);
    }

    // Инициализация действий пользователя
    runManager->SetUserInitialization(new ActionInitialization(area, EminMeV, EmaxMeV));
    runManager->Initialize();

    // Визуализация (если нужна)
    G4VisManager* visManager = nullptr;
    if (useUI) {
        visManager = new G4VisExecutive();
        visManager->Initialize();
    }
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    // Запуск сеанса
    if (!useUI) {
        UImanager->ApplyCommand("/control/execute " + macroFile);
    } else {
        G4UIExecutive* ui = new G4UIExecutive(argc, argv, "qt");
        UImanager->ApplyCommand("/control/execute ../vis.mac");
        ui->SessionStart();
        delete ui;
    }

    // Получение результатов после завершения сеанса
    const RunAction* runAction = dynamic_cast<const RunAction*>(runManager->GetUserRunAction());
    if (runAction) {
        const auto& [cOnly, cAndV] = runAction->GetCounts();
        crystalOnly = cOnly;
        crystalAndVeto = cAndV;
        effArea = runAction->GetEffArea();
        const auto& [cOnlyOpt, cAndVOpt] = runAction->GetOptCounts();
        crystalOnlyOpt = cOnlyOpt;
        crystalAndVetoOpt = cAndVOpt;
        effAreaOpt = runAction->GetEffAreaOpt();
    }

    // Сохранение конфигурации
    SaveConfig(macroFile, configPath, area,
               effArea, effAreaOpt,
               crystalOnly, crystalAndVeto,
               crystalOnlyOpt, crystalAndVetoOpt);

    // Очистка
    delete visManager;
    delete runManager;
    return 0;
}
