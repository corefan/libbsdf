// =================================================================== //
// Copyright (C) 2014-2016 Kimura Ryo                                  //
//                                                                     //
// This Source Code Form is subject to the terms of the Mozilla Public //
// License, v. 2.0. If a copy of the MPL was not distributed with this //
// file, You can obtain one at http://mozilla.org/MPL/2.0/.            //
// =================================================================== //

#include <libbsdf/Writer/DdrWriter.h>

#include <fstream>
#include <iostream>

#include <libbsdf/Brdf/Processor.h>
#include <libbsdf/Common/SpectrumUtility.h>
#include <libbsdf/Common/Version.h>

using namespace lb;

bool DdrWriter::write(const std::string& fileName, const SpecularCoordinatesBrdf& brdf)
{
    std::ofstream fout(fileName.c_str());
    if (fout.fail()) {
        std::cerr << "[DdrReader::write] Could not open: " << fileName << std::endl;
        return false;
    }

    return output(brdf, fout);
}

void DdrWriter::write(const std::string&    fileName,
                      const Brdf&           brdf,
                      bool                  inDirDependentCoordSysUsed)
{
    typedef SpecularCoordinatesBrdf SpecBrdf;
    typedef SpecularCoordinateSystem SpecCoordSys;

    SpecBrdf* exportedBrdf;
    if (dynamic_cast<const SpecBrdf*>(&brdf)) {
        exportedBrdf = new SpecBrdf(dynamic_cast<const SpecBrdf&>(brdf));
    }
    else if (!inDirDependentCoordSysUsed) {
        exportedBrdf = new SpecBrdf(brdf, 10, 1, 181, 37);
    }
    else {
        const SampleSet* ss = brdf.getSampleSet();

        Arrayf inThetaAngles    = ss->getAngles0();
        Arrayf inPhiAngles      = Arrayf::LinSpaced(ss->getNumAngles1(), 0.0, SpecCoordSys::MAX_ANGLE1);
        Arrayf outThetaAngles   = Arrayf::LinSpaced(181,                 0.0, SpecCoordSys::MAX_ANGLE2);
        Arrayf outPhiAngles     = Arrayf::LinSpaced(37,                  0.0, SpecCoordSys::MAX_ANGLE3);

        if (inPhiAngles.size() == 1) {
            inPhiAngles[0] = 0.0f;
        }

        exportedBrdf = new SpecBrdf(brdf, inThetaAngles, inPhiAngles, outThetaAngles, outPhiAngles);
    }

    SampleSet* exportedSs = exportedBrdf->getSampleSet();
    if (exportedSs->getColorModel() == XYZ_MODEL) {
        xyzToSrgb(exportedSs);
    }

    if (exportedBrdf->getNumInTheta() == 1) {
        const SampleSet* ss = exportedBrdf->getSampleSet();

        Arrayf inThetaAngles = Arrayf::LinSpaced(10, 0.0, SpecCoordSys::MAX_ANGLE0);

        SpecBrdf* filledBrdf = new SpecBrdf(*exportedBrdf,
                                            inThetaAngles,
                                            ss->getAngles1(),
                                            ss->getAngles2(),
                                            ss->getAngles3());
        delete exportedBrdf;
        exportedBrdf = filledBrdf;
    }

    exportedBrdf->expandAngles();
    fixEnergyConservation(exportedBrdf);

    DdrWriter::write(fileName, *exportedBrdf);
    delete exportedBrdf;
}

bool DdrWriter::output(const SpecularCoordinatesBrdf& brdf, std::ostream& stream)
{
    std::ios_base::sync_with_stdio(false);

    stream << ";; This file is generated by libbsdf-" << getVersion() << ".\n" << std::endl;

    const SampleSet* ss = brdf.getSampleSet();

    SourceType sourceType = brdf.getSourceType();
    if (sourceType == MEASURED_SOURCE) {
        stream << "Source Measured" << std::endl;
    }
    else if (sourceType == GENERATED_SOURCE) {
        stream << "Source Generated" << std::endl;
    }
    else if (sourceType == EDITED_SOURCE) {
        stream << "Source Edited" << std::endl;
    }
    else {
        stream << "Source Measured" << std::endl;
    }

    if (ss->isIsotropic()) {
        stream << "TypeSym ASymmetrical" << std::endl;
    }
    else {
        stream << "TypeSym ASymmetrical 4D" << std::endl;
    }

    ColorModel colorModel;
    stream << "TypeColorModel ";
    if (ss->getNumWavelengths() == 1) {
        colorModel = MONOCHROMATIC_MODEL;
        stream << "BW" << std::endl;
    }
    else if (ss->getColorModel() == RGB_MODEL ||
        ss->getColorModel() == XYZ_MODEL) {
        colorModel = RGB_MODEL;
        stream << "RGB" << std::endl;
    }
    else {
        colorModel = SPECTRAL_MODEL;
        stream << "spectral " << ss->getNumWavelengths() << std::endl;
    }

    stream << "TypeData Luminance Absolute" << std::endl;

    if (!ss->isIsotropic()) {
        stream << "psi " << brdf.getNumInPhi() << std::endl;
        for (int i = 0; i < brdf.getNumInPhi(); ++i) {
            stream << " " << toDegree(brdf.getInPhi(i));
        }
        stream << std::endl;
    }

    stream << "sigma " << brdf.getNumInTheta() << std::endl;
    for (int i = 0; i < brdf.getNumInTheta(); ++i) {
        stream << " " << toDegree(brdf.getInTheta(i));
    }
    stream << std::endl;

    stream << "phi " << brdf.getNumSpecPhi() << std::endl;
    for (int i = 0; i < brdf.getNumSpecPhi(); ++i) {
        stream << " " << toDegree(brdf.getSpecPhi(i));
    }
    stream << std::endl;

    stream << "theta " << brdf.getNumSpecTheta() << std::endl;
    for (int i = 0; i < brdf.getNumSpecTheta(); ++i) {
        stream << " " << toDegree(brdf.getSpecTheta(i));
    }
    stream << std::endl;

    for (int wlIndex = 0; wlIndex < ss->getNumWavelengths(); ++wlIndex) {
        if (colorModel == MONOCHROMATIC_MODEL) {
            stream << "bw" << std::endl;
        }
        else if (colorModel == RGB_MODEL) {
            if (wlIndex == 0) {
                stream << "red" << std::endl;
            }
            else if (wlIndex == 1) {
                stream << "green" << std::endl;
            }
            else {
                stream << "blue" << std::endl;
            }
        }
        else {
            stream << "wl " << ss->getWavelength(wlIndex) << std::endl;
        }

        stream << " kbdf" << std::endl;
        stream << " ";
        for (int i = 0; i < brdf.getNumInTheta() * brdf.getNumInPhi(); ++i) {
            stream << " 1.0";
        }

        stream << "\n def" << std::endl;

        for (int inPhIndex = 0; inPhIndex < brdf.getNumInPhi(); ++inPhIndex) {
            stream << ";; Psi = " << toDegree(brdf.getInPhi(inPhIndex)) << std::endl;

            for (int inThIndex = 0; inThIndex < brdf.getNumInTheta(); ++inThIndex) {
                stream << ";; Sigma = " << toDegree(brdf.getInTheta(inThIndex)) << std::endl;

                for (int spPhIndex = 0; spPhIndex < brdf.getNumSpecPhi(); ++spPhIndex) {
                    for (int spThIndex = 0; spThIndex < brdf.getNumSpecTheta(); ++spThIndex) {
                        const Spectrum& sp = brdf.getSpectrum(inThIndex, inPhIndex, spThIndex, spPhIndex);

                        if (ss->getColorModel() == XYZ_MODEL) {
                            Spectrum rgb = xyzToSrgb(sp);
                            stream << " " << rgb[wlIndex] * PI_F;
                        }
                        else {
                            stream << " " << sp[wlIndex] * PI_F;
                        }
                    }

                    stream << std::endl;
                }
            }
        }

        stream << " enddef" << std::endl;
    }

    return true;
}
