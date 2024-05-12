

#ifndef DT285_CLIP_H
#define DT285_CLIP_H

#include <vector>
#include "Affine.h"
#include "HalfSpace.h"

class Clip {
  public:
    Clip(void);
    Clip(int n, const Hcoords*);
    bool operator()(Point&, Point&);
    bool operator()(std::vector<Point>& verts);
  private:
    std::vector<Hcoords> hspaces;
    std::vector<Point> temp_points;
};


#endif

