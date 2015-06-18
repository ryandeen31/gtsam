/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file   SmartProjectionPoseFactor.h
 * @brief  Produces an Hessian factors on POSES from monocular measurements of a single landmark
 * @author Luca Carlone
 * @author Chris Beall
 * @author Zsolt Kira
 */

#pragma once

#include <gtsam/slam/SmartProjectionFactor.h>

namespace gtsam {
/**
 *
 * @addtogroup SLAM
 *
 * If you are using the factor, please cite:
 * L. Carlone, Z. Kira, C. Beall, V. Indelman, F. Dellaert, Eliminating conditionally
 * independent sets in factor graphs: a unifying perspective based on smart factors,
 * Int. Conf. on Robotics and Automation (ICRA), 2014.
 *
 */

/**
 * The calibration is known here. The factor only constraints poses (variable dimension is 6)
 * @addtogroup SLAM
 */
template<class CALIBRATION>
class SmartProjectionPoseFactor: public SmartProjectionFactor<
    PinholePose<CALIBRATION> > {

private:
  typedef PinholePose<CALIBRATION> Camera;
  typedef SmartProjectionFactor<Camera> Base;
  typedef SmartProjectionPoseFactor<CALIBRATION> This;

protected:

  boost::optional<Pose3> body_P_sensor_; ///< Pose of the camera in the body frame
  std::vector<boost::shared_ptr<CALIBRATION> > sharedKs_; ///< shared pointer to calibration object (one for each camera)

public:

  /// shorthand for a smart pointer to a factor
  typedef boost::shared_ptr<This> shared_ptr;

  /**
   * Constructor
   * @param rankTol tolerance used to check if point triangulation is degenerate
   * @param linThreshold threshold on relative pose changes used to decide whether to relinearize (selective relinearization)
   * @param manageDegeneracy is true, in presence of degenerate triangulation, the factor is converted to a rotation-only constraint,
   * otherwise the factor is simply neglected (this functionality is deprecated)
   * @param enableEPI if set to true linear triangulation is refined with embedded LM iterations
   * @param body_P_sensor is the transform from sensor to body frame (default identity)
   */
  SmartProjectionPoseFactor(const double rankTol = 1,
      const double linThreshold = -1, const DegeneracyMode manageDegeneracy = IGNORE_DEGENERACY,
      const bool enableEPI = false, boost::optional<Pose3> body_P_sensor = boost::none,
      LinearizationMode linearizeTo = HESSIAN, double landmarkDistanceThreshold = 1e10,
      double dynamicOutlierRejectionThreshold = -1) :
        Base(linearizeTo, rankTol, manageDegeneracy, enableEPI,
            landmarkDistanceThreshold, dynamicOutlierRejectionThreshold), body_P_sensor_(body_P_sensor) {}

  /** Virtual destructor */
  virtual ~SmartProjectionPoseFactor() {}

  /**
   * print
   * @param s optional string naming the factor
   * @param keyFormatter optional formatter useful for printing Symbols
   */
  void print(const std::string& s = "", const KeyFormatter& keyFormatter =
      DefaultKeyFormatter) const {
    std::cout << s << "SmartProjectionPoseFactor, z = \n ";
    if(body_P_sensor_)
      body_P_sensor_->print("body_P_sensor_:\n");
    Base::print("", keyFormatter);
  }

  /// equals
  virtual bool equals(const NonlinearFactor& p, double tol = 1e-9) const {
    const This *e = dynamic_cast<const This*>(&p);
    return e && Base::equals(p, tol);
  }

  /**
   * Linearize to Gaussian Factor
   * @param values Values structure which must contain camera poses for this factor
   * @return
   */
  virtual boost::shared_ptr<GaussianFactor> linearize(
      const Values& values) const {
    // depending on flag set on construction we may linearize to different linear factors
//    switch(linearizeTo_){
//    case JACOBIAN_SVD :
//      return this->createJacobianSVDFactor(Base::cameras(values), 0.0);
//      break;
//    case JACOBIAN_Q :
//      return this->createJacobianQFactor(Base::cameras(values), 0.0);
//      break;
//    default:
      return this->createHessianFactor(Base::cameras(values));
//      break;
//    }
  }

  /**
   * error calculates the error of the factor.
   */
  virtual double error(const Values& values) const {
    if (this->active(values)) {
      return this->totalReprojectionError(Base::cameras(values));
    } else { // else of active flag
      return 0.0;
    }
  }

  /** return calibration shared pointers */
  inline const std::vector<boost::shared_ptr<CALIBRATION> > calibration() const {
    return sharedKs_;
  }

  Pose3 body_P_sensor() const{
    if(body_P_sensor_)
      return *body_P_sensor_;
    else
      return Pose3(); // if unspecified, the transformation is the identity
  }

private:

  /// Serialization function
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
    ar & BOOST_SERIALIZATION_NVP(sharedKs_);
  }

}; // end of class declaration

/// traits
template<class CALIBRATION>
struct traits<SmartProjectionPoseFactor<CALIBRATION> > : public Testable<
    SmartProjectionPoseFactor<CALIBRATION> > {
};

} // \ namespace gtsam
