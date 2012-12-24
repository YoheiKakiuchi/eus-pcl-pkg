#include "eus_pcl/euspcl.h"
#include "eus_pcl/euspcl_recognition.h"

#include <pcl/features/impl/fpfh.hpp>
#include <pcl/recognition/impl/implicit_shape_model.hpp>

// applications
pointer PCL_ISM_TRAINING (register context *ctx, int n, pointer *argv) {
  //
  unsigned int number_of_training_clouds = 1;
  eusfloat_t estimate_normal_radius = 25.0;

  pcl::NormalEstimation< Point, PNormal > normal_estimator;
  normal_estimator.setRadiusSearch (estimate_normal_radius);

  std::vector< Points::Ptr >  training_clouds;
  std::vector< Normals::Ptr > training_normals;
  std::vector<unsigned int>   training_classes;

  for (int i = 0; i < n - 1; i++) {
    if (!isPointCloud (argv[i])) {
      error(E_TYPEMISMATCH);
    }
    pointer eus_in_cloud = argv[i];

    int width = intval (get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_WIDTH));
    int height = intval (get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_HEIGHT));
    pointer points = get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_POINTS);

    Points::Ptr tr_points =
      make_pcl_pointcloud< Point > (ctx, points, NULL, NULL, NULL, width, height);

    Normals::Ptr tr_normals (new Normals);
    normal_estimator.setInputCloud (tr_points);
    normal_estimator.compute (*tr_normals);

    std::cerr << ";; read points and add normal " << i << std::endl;
    training_clouds.push_back (tr_points);
    training_normals.push_back (tr_normals);
    training_classes.push_back (i);
  }

  pcl::FPFHEstimation< Point, PNormal, pcl::Histogram<153> >::Ptr fpfh
    (new pcl::FPFHEstimation < Point, PNormal, pcl::Histogram<153> >);
  fpfh->setRadiusSearch (30.0);
  pcl::Feature< Point, pcl::Histogram<153> >::Ptr feature_estimator (fpfh);

  pcl::ism::ImplicitShapeModelEstimation<153, Point, PNormal> ism;
  ism.setFeatureEstimator (feature_estimator);
  ism.setTrainingClouds  (training_clouds);
  ism.setTrainingNormals (training_normals);
  ism.setTrainingClasses (training_classes);
  ism.setSamplingSize (2.0f);

  pcl::ism::ImplicitShapeModelEstimation<153, Point, PNormal>::ISMModelPtr model
    = boost::shared_ptr<pcl::features::ISMModel> (new pcl::features::ISMModel);

  std::cerr << ";; start training" << std::endl;
  ism.trainISM (model);
  std::cerr << ";; done training" << std::endl;

  std::string file ("trained_ism_model.txt"); // filename
  model->saveModelToFile (file);

#if 1
  Points::Ptr testing_points;
  Normals::Ptr testing_normals
    = (new Normals)->makeShared ();
  {
    if (!isPointCloud (argv[n-1])) {
      error(E_TYPEMISMATCH);
    }
    pointer eus_in_cloud = argv[n-1];

    int width = intval (get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_WIDTH));
    int height = intval (get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_HEIGHT));
    pointer points = get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_POINTS);

    testing_points = make_pcl_pointcloud< Point > (ctx, points, NULL, NULL, NULL, width, height);

    normal_estimator.setInputCloud (testing_points);
    normal_estimator.compute (*testing_normals);
    std::cerr << ";; read points and add normal testing" << std::endl;
  }
  model->loadModelFromfile (file);
  int testing_class = 0;

  std::cerr << ";; start find objects" << std::endl;
  boost::shared_ptr< pcl::features::ISMVoteList< Point > > vote_list =
    ism.findObjects (model, testing_points, testing_normals, testing_class);
  std::cerr << ";; done find objects" << std::endl;

  double radius = model->sigmas_[testing_class] * 10.0;
  double sigma = model->sigmas_[testing_class];

  std::cerr << "moge" << std::endl;

  std::vector< pcl::ISMPeak, Eigen::aligned_allocator<pcl::ISMPeak> > strongest_peaks;
  std::cerr << "fuga" << std::endl;
  vote_list->findStrongestPeaks (strongest_peaks, testing_class, radius, sigma);
  std::cerr << "higa" << std::endl;

  std::cerr << "num = " << vote_list->getNumberOfVotes() << std::endl;
  std::cerr << "error size = " << strongest_peaks.size() << std::endl;
  for (size_t i_vote = 0;
       i_vote < strongest_peaks.size ();
       i_vote++)
    {
      std::cerr << "density = " << strongest_peaks[i_vote].density << " / class = ";
      std::cerr << strongest_peaks[i_vote].class_id << std::endl;
      std::cerr << strongest_peaks[i_vote].x << " ";
      std::cerr << strongest_peaks[i_vote].y << " ";
      std::cerr << strongest_peaks[i_vote].z << std::endl;
    }
#endif
  return NIL;
}

pointer PCL_ISM_DETECTION (register context *ctx, int n, pointer *argv) {
  pointer eus_in_cloud;
  pointer ret = NIL;
  int pc = 0;
  numunion nu;
  eusfloat_t estimate_normal_radius = 25.0;
  std::string fname;

  ckarg(3);
  fname.assign ((const char *)argv[0]->c.str.chars);
  unsigned int testing_class = intval (argv[1]);

  if (!isPointCloud (argv[2])) {
    error(E_TYPEMISMATCH);
  }
  eus_in_cloud = argv[2];

  int width = intval (get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_WIDTH));
  int height = intval (get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_HEIGHT));
  pointer points = get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_POINTS);
  //pointer colors = get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_COLORS);
  //pointer normals = get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_NORMALS);
  //pointer curvatures = get_from_pointcloud (ctx, eus_in_cloud, K_EUSPCL_CURVATURES);
  Points::Ptr testing_points =
    make_pcl_pointcloud< Point > (ctx, points, NULL, NULL, NULL, width, height);

  pcl::NormalEstimation< Point, PNormal > normal_estimator;
  normal_estimator.setRadiusSearch (estimate_normal_radius);

  Normals::Ptr testing_normals
    = (new Normals)->makeShared ();
  normal_estimator.setInputCloud (testing_points);
  normal_estimator.compute (*testing_normals);

  pcl::ism::ImplicitShapeModelEstimation<153, Point, PNormal>::ISMModelPtr model
    = boost::shared_ptr<pcl::features::ISMModel> (new pcl::features::ISMModel);

  model->loadModelFromfile (fname);

  pcl::ism::ImplicitShapeModelEstimation<153, Point, PNormal> ism;
  boost::shared_ptr< pcl::features::ISMVoteList< Point > > vote_list =
    ism.findObjects (model, testing_points, testing_normals, testing_class);

  double radius = model->sigmas_[testing_class] * 10.0;
  double sigma = model->sigmas_[testing_class];

  std::vector< pcl::ISMPeak, Eigen::aligned_allocator<pcl::ISMPeak> > strongest_peaks;
  vote_list->findStrongestPeaks (strongest_peaks, testing_class, radius, sigma);

  std::cerr << "num = " << vote_list->getNumberOfVotes() << std::endl;
  std::cerr << "error size = " << strongest_peaks.size() << std::endl;
  for (size_t i_vote = 0;
       i_vote < strongest_peaks.size ();
       i_vote++)
    {
      std::cerr << "density = " << strongest_peaks[i_vote].density << " / class = ";
      std::cerr << strongest_peaks[i_vote].class_id << std::endl;
      std::cerr << strongest_peaks[i_vote].x << " ";
      std::cerr << strongest_peaks[i_vote].y << " ";
      std::cerr << strongest_peaks[i_vote].z << std::endl;
    }
#if 0
  PointsC::Ptr colored_cloud = (new PointsC)->makeShared ();
  colored_cloud->height = 0;
  colored_cloud->width = 1;

  pcl::PointXYZRGB point;
  point.r = 255;
  point.g = 255;
  point.b = 255;

  for (size_t i_point = 0;
       i_point < testing_cloud->points.size ();
       i_point++)
    {
      point.x = testing_cloud->points[i_point].x;
      point.y = testing_cloud->points[i_point].y;
      point.z = testing_cloud->points[i_point].z;
      colored_cloud->points.push_back (point);
    }
  colored_cloud->height += testing_cloud->points.size ();

  point.r = 255;
  point.g = 0;
  point.b = 0;
  for (size_t i_vote = 0;
       i_vote < strongest_peaks.size ();
       i_vote++)
    {
      point.x = strongest_peaks[i_vote].x;
      point.y = strongest_peaks[i_vote].y;
      point.z = strongest_peaks[i_vote].z;
      colored_cloud->points.push_back (point);
    }
  colored_cloud->height += strongest_peaks.size ();
#endif
  return NIL;
}


