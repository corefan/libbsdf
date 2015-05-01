// =================================================================== //
// Copyright (C) 2014-2015 Kimura Ryo                                  //
//                                                                     //
// This Source Code Form is subject to the terms of the Mozilla Public //
// License, v. 2.0. If a copy of the MPL was not distributed with this //
// file, You can obtain one at http://mozilla.org/MPL/2.0/.            //
// =================================================================== //

/*!
 * \file    Vector.h
 * \brief   The Vector.h header file includes the vector declarations and functions.
 */

#ifndef LIBBSDF_VECTOR_H
#define LIBBSDF_VECTOR_H

#include <vector>

#include <Eigen/Core>

#include <libbsdf/Common/AlignedVec3f.h>

namespace lb {

typedef Eigen::Vector2d Vec2;
typedef Eigen::Vector2f Vec2f;
typedef Eigen::Vector2i Vec2i;

typedef AlignedVec3f    Vec3;
typedef Eigen::Vector3f Vec3f;
typedef Eigen::Vector3d Vec3d;

typedef Eigen::Vector4f Vec4;
typedef Eigen::Vector4i Vec4i;

typedef Eigen::ArrayXf Arrayf;
typedef Eigen::ArrayXd Arrayd;

/*! \brief Converts from a vector to lb::Vec3. */
template <typename Vec3T>
Vec3 toVec3(const Vec3T& vec3);

/*! \brief Appends an element to the end of an array. */
template <typename ArrayT, typename ScalarT>
void append(ArrayT& arrayf, ScalarT value);

/* Implementation */

template <typename Vec3T>
inline Vec3 toVec3(const Vec3T& vec3)
{
    return Vec3(static_cast<Vec3::Scalar>(vec3[0]),
                static_cast<Vec3::Scalar>(vec3[1]),
                static_cast<Vec3::Scalar>(vec3[2]));
}

template <typename ArrayT, typename ScalarT>
inline void append(ArrayT& arrayf, ScalarT value)
{
    std::vector<ScalarT> orig(arrayf.data(), arrayf.data() + arrayf.size());
    orig.push_back(value);
    arrayf.resize(arrayf.size() + 1);

#if (_MSC_VER >= 1600) // Visual Studio 2010
    std::copy(orig.begin(), orig.end(),
              stdext::checked_array_iterator<ArrayT::Scalar*>(arrayf.data(), arrayf.size()));
#else
    std::copy(orig.begin(), orig.end(), arrayf.data());
#endif
}

} // namespace lb

#endif // LIBBSDF_VECTOR_H
