//
//    AGAST, an adaptive and generic corner detector based on the
//              accelerated segment test for a 8 pixel mask
//
//    Copyright (C) 2010  Elmar Mair
//    All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//        * Redistributions of source code must retain the above copyright
//          notice, this list of conditions and the following disclaimer.
//        * Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the distribution.
//        * Neither the name of the <organization> nor the
//          names of its contributors may be used to endorse or promote products
//          derived from this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
//    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdlib.h>
#include <agast/ast-detector.h>

namespace agast {
void AstDetector::NonMaximumSuppression(const std::vector<agast::KeyPoint>& corners_all,
                                        std::vector<agast::KeyPoint>& corners_nms) {
  int currCorner_ind;
  int lastRow = 0, next_lastRow = 0;
  std::vector<agast::KeyPoint>::const_iterator currCorner;
  int lastRowCorner_ind = 0, next_lastRowCorner_ind = 0;
  std::vector<int>::iterator nmsFlags_p;
  std::vector<agast::KeyPoint>::iterator currCorner_nms;
  int j;
  int numCorners_all = corners_all.size();
  int nMaxCorners = corners_nms.capacity();

  currCorner = corners_all.begin();

  if (numCorners_all > nMaxCorners) {
    if (nMaxCorners == 0) {
      nMaxCorners = 512 > numCorners_all ? 512 : numCorners_all;
      corners_nms.reserve(nMaxCorners);
      nmsFlags_.reserve(nMaxCorners);
    } else {
      nMaxCorners *= 2;
      if (numCorners_all > nMaxCorners)
        nMaxCorners = numCorners_all;
      corners_nms.reserve(nMaxCorners);
      nmsFlags_.reserve(nMaxCorners);
    }
  }
  corners_nms.resize(numCorners_all);
  nmsFlags_.resize(numCorners_all);

  nmsFlags_p = nmsFlags_.begin();
  currCorner_nms = corners_nms.begin();

  // Set all flags to MAXIMUM.
  for (j = numCorners_all; j > 0; j--)
    *nmsFlags_p++ = -1;
  nmsFlags_p = nmsFlags_.begin();

  for (currCorner_ind = 0; currCorner_ind < numCorners_all; currCorner_ind++) {
    int t;

    // Check above.
    if (lastRow + 1 < KeyPointY(*currCorner)) {
      lastRow = next_lastRow;
      lastRowCorner_ind = next_lastRowCorner_ind;
    }
    if (next_lastRow != KeyPointY(*currCorner)) {
      next_lastRow = KeyPointY(*currCorner);
      next_lastRowCorner_ind = currCorner_ind;
    }
    if (lastRow + 1 == KeyPointY(*currCorner)) {
      // Find the corner above the current one.
      while ((KeyPointX(corners_all[lastRowCorner_ind]) <
          KeyPointX(*currCorner))
          && (KeyPointY(corners_all[lastRowCorner_ind]) == lastRow))
        lastRowCorner_ind++;

      if ((KeyPointX(corners_all[lastRowCorner_ind]) ==
          KeyPointX(*currCorner))
          && (lastRowCorner_ind != currCorner_ind)) {
        int t = lastRowCorner_ind;
        while (nmsFlags_[t] != -1)  // Find the maximum in this block.
          t = nmsFlags_[t];

        if (scores_[currCorner_ind] < scores_[t]) {
          nmsFlags_[currCorner_ind] = t;
        } else
          nmsFlags_[t] = currCorner_ind;
      }
    }

    // Check left.
    t = currCorner_ind - 1;
    if ((currCorner_ind != 0) &&
        (KeyPointY(corners_all[t]) == KeyPointY(*currCorner))
        && (KeyPointX(corners_all[t]) + 1 == KeyPointX(*currCorner))) {
      int currCornerMaxAbove_ind = nmsFlags_[currCorner_ind];

      while (nmsFlags_[t] != -1)  // Find the maximum in that area.
        t = nmsFlags_[t];

      if (currCornerMaxAbove_ind == -1)  // No maximum above.
          {
        if (t != currCorner_ind) {
          if (scores_[currCorner_ind] < scores_[t])
            nmsFlags_[currCorner_ind] = t;
          else
            nmsFlags_[t] = currCorner_ind;
        }
      } else	// Maximum above.
      {
        if (t != currCornerMaxAbove_ind) {
          if (scores_[currCornerMaxAbove_ind] < scores_[t]) {
            nmsFlags_[currCornerMaxAbove_ind] = t;
            nmsFlags_[currCorner_ind] = t;
          } else {
            nmsFlags_[t] = currCornerMaxAbove_ind;
            nmsFlags_[currCorner_ind] = currCornerMaxAbove_ind;
          }
        }
      }
    }

    currCorner++;
  }

  // Collecting maximum corners.
  corners_nms.resize(0);
  for (currCorner_ind = 0; currCorner_ind < numCorners_all; currCorner_ind++) {
    if (*nmsFlags_p++ == -1)
      corners_nms.push_back(corners_all[currCorner_ind]);
  }
}
}  // namespace agast
