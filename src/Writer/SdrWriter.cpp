// =================================================================== //
// Copyright (C) 2015-2016 Kimura Ryo                                  //
//                                                                     //
// This Source Code Form is subject to the terms of the Mozilla Public //
// License, v. 2.0. If a copy of the MPL was not distributed with this //
// file, You can obtain one at http://mozilla.org/MPL/2.0/.            //
// =================================================================== //

#include <libbsdf/Writer/SdrWriter.h>

#include <fstream>
#include <iostream>

#include <libbsdf/Common/SpectrumUtility.h>
#include <libbsdf/Common/Version.h>

using namespace lb;

bool SdrWriter::write(const std::string& fileName, const SampleSet2D& samples2D)
{
    std::ofstream fout(fileName.c_str());
    if (fout.fail()) {
        std::cerr << "[SdrReader::write] Could not open: " << fileName << std::endl;
        return false;
    }

    return output(samples2D, fout);
}

bool SdrWriter::output(const SampleSet2D& samples2D, std::ostream& stream)
{
    stream << ";; This file is generated by libbsdf-" << getVersion() << ".\n" << std::endl;

    SourceType sourceType = samples2D.getSourceType();
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

    ColorModel colorModel;
    stream << "TypeColorModel ";
    if (samples2D.getNumWavelengths() == 1) {
        colorModel = MONOCHROMATIC_MODEL;
        stream << "BW" << std::endl;
    }
    else if (samples2D.getColorModel() == RGB_MODEL ||
             samples2D.getColorModel() == XYZ_MODEL) {
        colorModel = RGB_MODEL;
        stream << "RGB" << std::endl;
    }
    else {
        colorModel = SPECTRAL_MODEL;
        stream << "spectral " << samples2D.getNumWavelengths() << std::endl;
    }

    stream << "sigma " << samples2D.getNumTheta() << std::endl;
    for (int i = 0; i < samples2D.getNumTheta(); ++i) {
        stream << " " << toDegree(samples2D.getTheta(i));
    }
    stream << std::endl;

    for (int wlIndex = 0; wlIndex < samples2D.getNumWavelengths(); ++wlIndex) {
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
            stream << "wl " << samples2D.getWavelength(wlIndex) << std::endl;
        }

        stream << " def" << std::endl;

        for (int inThIndex = 0; inThIndex < samples2D.getNumTheta(); ++inThIndex) {
            const Spectrum& sp = samples2D.getSpectrum(inThIndex);

            if (samples2D.getColorModel() == XYZ_MODEL) {
                Spectrum rgb = xyzToSrgb(sp);
                stream << " " << rgb[wlIndex];
            }
            else {
                stream << " " << sp[wlIndex];
            }
        }

        stream << std::endl;
        stream << " enddef" << std::endl;
    }

    return true;
}
