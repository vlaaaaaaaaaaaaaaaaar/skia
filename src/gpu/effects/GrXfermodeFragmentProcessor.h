/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXfermodeFragmentProcessor_DEFINED
#define GrXfermodeFragmentProcessor_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"

class GrFragmentProcessor;

namespace GrXfermodeFragmentProcessor {

/** Blends src and dst inputs according to the blend mode.
 *  If either input is null, the input color (sk_InColor) is used instead.
 */
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                          std::unique_ptr<GrFragmentProcessor> dst,
                                          SkBlendMode mode);

};

#endif
