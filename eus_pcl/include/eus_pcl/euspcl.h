#ifndef __EUSPCL_H__
#define __EUSPCL_H__

//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <setjmp.h>
#include <errno.h>

#include <list>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <cstdio>

// For PCL
#include <pcl/pcl_base.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <boost/make_shared.hpp>

// euspcl_io.h
#include <pcl/io/pcd_io.h>

// euspcl_filters.h
#include <pcl/filters/voxel_grid.h>

// euspcl_segmentation.h
#include <pcl/segmentation/extract_clusters.h>

// for eus.h
#define class   eus_class
#define throw   eus_throw
#define export  eus_export
#define vector  eus_vector
#define string  eus_string
#define iostream eus_iostream
#define complex eus_complex

#include "eus.h"

#undef class
#undef throw
#undef export
#undef vector
#undef string
#undef iostream
#undef complex

typedef pcl::PointXYZ    Point;
typedef pcl::Normal      PNormal;
typedef pcl::PointXYZRGB PointC;
typedef pcl::PointNormal PointN;
typedef pcl::PointXYZRGBNormal PointCN;

typedef pcl::PointCloud< Point >   Points;
typedef pcl::PointCloud< PNormal > Normals;
typedef pcl::PointCloud< PointN >  PointsN;
typedef pcl::PointCloud< PointC >  PointsC;
typedef pcl::PointCloud< PointCN > PointsCN;

extern pointer K_EUSPCL_INIT, K_EUSPCL_POINTS, K_EUSPCL_COLORS, K_EUSPCL_NORMALS, K_EUSPCL_WIDTH, K_EUSPCL_HEIGHT;

static inline void fvector2pcl_pointcloud(eusfloat_t *src, eusfloat_t *rgb, eusfloat_t *nm,
                                          int width, int height, Points &pt) {
  pt.width    = width;
  pt.height   = height;
  pt.is_dense = false;
  pt.points.resize (width*height);

  if ( src != NULL ) {
    for (size_t i=0; i < pt.points.size(); i++) {
      pt.points[i].x = (*src++)/1000.0;
      pt.points[i].y = (*src++)/1000.0;
      pt.points[i].z = (*src++)/1000.0;
    }
  }
}

static inline void fvector2pcl_pointcloud(eusfloat_t *src, eusfloat_t *rgb, eusfloat_t *nm,
                                          int width, int height, Normals &pt) {
  pt.width    = width;
  pt.height   = height;
  pt.is_dense = false;
  pt.points.resize (width*height);

  if ( nm != NULL ) {
    for (size_t i=0; i < pt.points.size(); i++) {
      pt.points[i].normal[0] = *src++;
      pt.points[i].normal[1] = *src++;
      pt.points[i].normal[2] = *src++;
    }
  }
}

static inline void fvector2pcl_pointcloud(eusfloat_t *src, eusfloat_t *rgb, eusfloat_t *nm,
                                          int width, int height, PointsN &pt) {
  pt.width    = width;
  pt.height   = height;
  pt.is_dense = false;
  pt.points.resize (width*height);

  for (size_t i=0; i < pt.points.size(); i++) {
    if ( src != NULL ) {
      pt.points[i].x = (*src++)/1000.0;
      pt.points[i].y = (*src++)/1000.0;
      pt.points[i].z = (*src++)/1000.0;
    }

    if ( nm != NULL ) {
      pt.points[i].normal[0] = *nm++;
      pt.points[i].normal[1] = *nm++;
      pt.points[i].normal[2] = *nm++;
      //if (cvt != NULL)  pt.points[i].curvature = *cvt++;
    }
  }
}

static inline void fvector2pcl_pointcloud(eusfloat_t *src, eusfloat_t *rgb, eusfloat_t *nm,
                                          int width, int height, PointsC &pt) {
  pt.width    = width;
  pt.height   = height;
  pt.is_dense = false;
  pt.points.resize (width*height);

  unsigned char r, g, b;
  unsigned int int_rgb;

  for (size_t i=0; i < pt.points.size(); i++) {
    if ( src != NULL ) {
      pt.points[i].x = (*src++)/1000.0;
      pt.points[i].y = (*src++)/1000.0;
      pt.points[i].z = (*src++)/1000.0;
    }
    if ( rgb != NULL ) {
      r = round(255.0 * (*rgb++));
      g = round(255.0 * (*rgb++));
      b = round(255.0 * (*rgb++));
      int_rgb = (r << 16) | (g << 8) | b;
      pt.points[i].rgb = *(float *)(&int_rgb);
    }
  }
}
static inline void fvector2pcl_pointcloud(eusfloat_t *src, eusfloat_t *rgb, eusfloat_t *nm,
                                          int width, int height, PointsCN &pt) {
  pt.width    = width;
  pt.height   = height;
  pt.is_dense = false;
  pt.points.resize (width*height);

  unsigned char r, g, b;
  unsigned int int_rgb;

  for (size_t i=0; i < pt.points.size(); i++) {
    if ( src != NULL ) {
      pt.points[i].x = (*src++)/1000.0;
      pt.points[i].y = (*src++)/1000.0;
      pt.points[i].z = (*src++)/1000.0;
    }

    if ( rgb != NULL ) {
      r = round(255.0 * (*rgb++));
      g = round(255.0 * (*rgb++));
      b = round(255.0 * (*rgb++));
      int_rgb = (r << 16) | (g << 8) | b;
      pt.points[i].rgb = *(float *)(&int_rgb);
    }

    if ( nm != NULL ) {
      pt.points[i].normal[0] = *nm++;
      pt.points[i].normal[1] = *nm++;
      pt.points[i].normal[2] = *nm++;
    }
  }
}

template < typename PTS >
inline typename pcl::PointCloud<PTS>::Ptr
make_pcl_pointcloud (register context *ctx,
                     pointer points, pointer colors, pointer normals,
                     int width, int height) {

  typename pcl::PointCloud< PTS >::Ptr pcl_cloud ( new  pcl::PointCloud< PTS > );

  fvector2pcl_pointcloud(points == NIL ? NULL : points->c.ary.entity->c.fvec.fv,
                         colors == NIL ? NULL : colors->c.ary.entity->c.fvec.fv,
                         normals == NIL ? NULL : normals->c.ary.entity->c.fvec.fv,
                         width, height, *pcl_cloud );

  return pcl_cloud;
}

inline pointer get_from_pointcloud(register context *ctx,
                                   pointer pointcloud,
                                   pointer key) {
  register pointer *local=ctx->vsp, w;

  local[0] = pointcloud;
  local[1] = key;
  ctx->vsp = local + 2;
  w=(pointer)SEND(ctx, 2, local);  /*instantiate*/
  ctx->vsp = local;

  return w;
}

extern pointer eval_c_string(register context *ctx, const char *strings);
extern pointer make_eus_pointcloud(register context *ctx, pointer pos, pointer col, pointer nom);

extern pointer make_pointcloud_from_pcl ( register context *ctx, Points pt );
extern pointer make_pointcloud_from_pcl ( register context *ctx, PointsC pt );
extern pointer make_pointcloud_from_pcl ( register context *ctx, PointsN pt );
extern pointer make_pointcloud_from_pcl ( register context *ctx, PointsCN pt );

#endif
