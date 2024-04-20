#ifndef DT285_PROJECTION_H
#define DT285_PROJECTION_H

#include "Camera.h"


Affine WorldToCamera(const Camera& cam);

Matrix CameraToNDC(const Camera& cam);


#endif

