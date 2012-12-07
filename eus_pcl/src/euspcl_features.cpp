#include "eus_pcl/euspcl.h"
#include "eus_pcl/euspcl_features.h"

#define ADD_NORMAL_(PTYPE, radius)                                      \
  pcl::PointCloud< PTYPE >::Ptr pcl_cloud =                             \
    make_pcl_pointcloud< PTYPE > (ctx, points, colors, NULL, NULL, width, height); \
  pcl::NormalEstimation< PTYPE, PNormal > no_est;                       \
  no_est.setInputCloud (pcl_cloud);                                     \
  //                                                                    \
  pcl::search::KdTree<PTYPE>::Ptr tree (new pcl::search::KdTree<PTYPE> ()); \
  no_est.setSearchMethod (tree);                                        \
  if (radius > 1000.0) {                                                \
    no_est.setKSearch (round(radius/1000.0));                           \
  } else {                                                              \
    no_est.setRadiusSearch (radius/1000.0);                             \
  }                                                                     \
  Normals::Ptr cloud_nm (new Normals);                                  \
  // Compute the features                                               \
  no_est.compute (*cloud_nm);                                           \
  if (create) {                                                         \
    ret = make_pointcloud_from_pcl (ctx, pcl_cloud, *cloud_nm);         \
    vpush(ret); pc++;                                                   \
  } else {                                                              \
    pointer nom = makematrix (ctx, len, 3);                             \
    vpush (nom); pc++;                                                  \
    eusfloat_t *fv = nom->c.ary.entity->c.fvec.fv;                      \
    for (Normals::const_iterator it = cloud_nm->begin();                \
         it != cloud_nm->end(); it++) {                                 \
      *fv++ = it->normal_x;                                             \
      *fv++ = it->normal_y;                                             \
      *fv++ = it->normal_z;                                             \
    }                                                                   \
    ret = set_to_pointcloud(ctx, in_cloud, K_EUSPCL_NORMALS, nom);      \
  }

pointer PCL_ADD_NORMAL (register context *ctx, int n, pointer *argv) {
  /* ( pointcloud &optional (radius 30.0) (create nil) ) */
  pointer in_cloud;
  pointer points, colors, normals;
  pointer ret = NIL;
  numunion nu;
  int pc = 0;
  bool create_pc = false;
  eusfloat_t arg_rad = 30.0;

  ckarg2(1, 3);
  if (!isPointCloud (argv[0])) {
    error(E_TYPEMISMATCH);
    return ret;
  }
  in_cloud = argv[0];
  if (n > 1) {
    arg_rad = fltval (argv[1]);
  }
  if (n > 2) {
    create_pc = true;
  }

  int width = intval (get_from_pointcloud (ctx, in_cloud, K_EUSPCL_WIDTH));
  int height = intval (get_from_pointcloud (ctx, in_cloud, K_EUSPCL_HEIGHT));
  points = get_from_pointcloud (ctx, in_cloud, K_EUSPCL_POINTS);
  colors = get_from_pointcloud (ctx, in_cloud, K_EUSPCL_COLORS);
  normals = get_from_pointcloud (ctx, in_cloud, K_EUSPCL_NORMALS);

  if (points != NIL && colors != NIL) {
    ADD_NORMAL_(PointC, arg_rad);
  } else if (points != NIL) {
    ADD_NORMAL_(Point, arg_rad);
  } else {
    // warning there is no points.
  }

  while (pc-- > 0) vpop();
  return ret;
}
