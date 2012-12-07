#include "eus_pcl/euspcl.h"
#include "eus_pcl/euspcl_surface.h"

#if 0
int concave_plane(eusfloat_t *src, int ssize,
                  eusfloat_t *coeff, eusfloat_t alpha, eusfloat_t *ret) {

  typedef pcl::PointXYZ Point;
  pcl::PointCloud<Point>::Ptr cloud_projected (new pcl::PointCloud<Point>);
  pcl::PointCloud<Point>::Ptr cloud_filtered (new pcl::PointCloud<Point>);
  floatvector2pointcloud(src, ssize, 1, *cloud_filtered);

  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
  coefficients->values.resize(4);
  coefficients->values[0] = coeff[0];
  coefficients->values[1] = coeff[1];
  coefficients->values[2] = coeff[2];
  coefficients->values[3] = coeff[3]/1000.0;

  // Project the model inliers
  pcl::ProjectInliers<Point> proj;
  proj.setModelType (pcl::SACMODEL_PLANE);
  proj.setInputCloud (cloud_filtered);
  proj.setModelCoefficients (coefficients);
  proj.filter (*cloud_projected);

  // Create a Concave Hull representation of the projected inliers
  pcl::PointCloud<Point>::Ptr cloud_hull (new pcl::PointCloud<Point>);
  pcl::ConcaveHull<Point> chull;

  chull.setInputCloud (cloud_projected);
  chull.setAlpha (alpha);
  chull.reconstruct (*cloud_hull);

  for(int i = 0; i < cloud_hull->points.size(); i++) {
    *ret++ = cloud_hull->points[i].x * 1000.0;
    *ret++ = cloud_hull->points[i].y * 1000.0;
    *ret++ = cloud_hull->points[i].z * 1000.0;
  }

  return cloud_hull->points.size();
}

int convex_plane(eusfloat_t *src, int ssize,
                 eusfloat_t *coeff, eusfloat_t *ret) {

  typedef pcl::PointXYZ Point;
  pcl::PointCloud<Point>::Ptr cloud_projected (new pcl::PointCloud<Point>);
  pcl::PointCloud<Point>::Ptr cloud_filtered (new pcl::PointCloud<Point>);
  floatvector2pointcloud(src, ssize, 1, *cloud_filtered);

  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
  coefficients->values.resize(4);
  coefficients->values[0] = coeff[0];
  coefficients->values[1] = coeff[1];
  coefficients->values[2] = coeff[2];
  coefficients->values[3] = coeff[3] / 1000.0;

  // Project the model inliers
  pcl::ProjectInliers<Point> proj;
  proj.setModelType (pcl::SACMODEL_PLANE);
  proj.setInputCloud (cloud_filtered);
  proj.setModelCoefficients (coefficients);
  proj.filter (*cloud_projected);

  // Create a Concave Hull representation of the projected inliers
  pcl::PointCloud<Point>::Ptr cloud_hull (new pcl::PointCloud<Point>);
  pcl::ConvexHull<Point> chull;

  chull.setInputCloud (cloud_projected);
  //chull.setAlpha (alpha);
  chull.reconstruct (*cloud_hull);

  for(int i = 0; i < cloud_hull->points.size(); i++) {
    *ret++ = cloud_hull->points[i].x * 1000.0;
    *ret++ = cloud_hull->points[i].y * 1000.0;
    *ret++ = cloud_hull->points[i].z * 1000.0;
  }

  return cloud_hull->points.size();
}
#endif

pointer PCL_CONVEX_HULL (register context *ctx, int n, pointer *argv) {
  pointer in_cloud = argv[0];

  int width = intval(get_from_pointcloud(ctx, in_cloud, K_EUSPCL_WIDTH));
  int height = intval(get_from_pointcloud(ctx, in_cloud, K_EUSPCL_HEIGHT));
  pointer points = get_from_pointcloud(ctx, in_cloud, K_EUSPCL_POINTS);

  pcl::PointCloud< Point >::Ptr ptr =
    make_pcl_pointcloud< Point > (ctx, points, NULL, NULL, width, height);

  pcl::PointCloud< Point >::Ptr cloud_hull (new pcl::PointCloud<Point>);
  pcl::ConvexHull< Point > chull;

  chull.setInputCloud (ptr);
  chull.reconstruct (*cloud_hull);

  return make_pointcloud_from_pcl (ctx, *cloud_hull);
}

pointer PCL_CONVEX_HULL_PLANE
(register context *ctx, int n, pointer *argv) {

}

pointer PCL_CONCAVE_HULL (register context *ctx, int n, pointer *argv) { }

pointer PCL_CONCAVE_HULL_PLANE (register context *ctx, int n, pointer *argv) { }
