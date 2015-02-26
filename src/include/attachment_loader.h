//
// Copyright 2012-2013, Syoyo Fujita.
//
// Licensed under 2-clause BSD liecense.
//
#ifndef __ATTACHMENT_LOADER_H__
#define __ATTACHMENT_LOADER_H__

#include <string>
#include <vector>

void PIN_loadWeights(std::vector<float>& boneWeights,
                     int& numBones,
                     const char * path);

void PIN_loadSkeleton(std::vector<float>& frames,
                      std::vector<float>& bindPose,
                      int& numBones,
                      const char * path);

#endif  // _ATTACHMENT_LOADER_H
