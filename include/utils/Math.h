#ifndef DWL_Math_H
#define DWL_Math_H

#include <utils/macros.h>
#include <Eigen/Dense>
#include <vector>


namespace dwl
{

enum AngleRepresentation {ZeroTo2Pi, MinusPiToPi};

namespace utils
{

/**
 * @class Math
 * @brief Class for defining mathematical methods
 */
class Math
{
	public:
		/** @brief Constructor function */
		Math();

		/** @brief Destructor function */
		~Math();

		void normalizeAngle(double& angle, AngleRepresentation angle_notation);

		/**
		 * @brief Computes the mean and covariance of the cloud of 3D points
		 * @param std::vector<Eigen::Vector3f> cloud Cloud of 3D points
		 * @param Eigen::Matrix3d& covariance_matrix Matrix of covariance
		 * @param Eigen::Vector3d& Mean vector
		 * @return unsigned int Return the size of the cloud
		 */
		unsigned int computeMeanAndCovarianceMatrix(std::vector<Eigen::Vector3f> cloud,
														 Eigen::Matrix3d& covariance_matrix, Eigen::Vector3d& mean);

		/**
		 * @brief Solves the plane parameters
		 * @param Eigen::Matrix3d& covariance_matrix Matrix of covariance
		 * @param Eigen::Vector3d& normal_vector Normal vector of the plane
		 * @param double& curvature Curvature of the plane
		 */
		void solvePlaneParameters(const Eigen::Matrix3d& covariance_matrix,	Eigen::Vector3d& normal_vector, double& curvature);

		/**
		 * @brief Computes the roots of a third order equation
		 * @param Eigen::Matrix3d& m Matrix
		 * @param Eigen::Vector3d& roots Roots
		 */
		void computeRoots(const Eigen::Matrix3d& m, Eigen::Vector3d& roots);

	private:
		/**
		 * @brief Complementary function of the computeRoots method
		 * @param const Eigen::Matrix3d::Scalar& b
		 * @param const Eigen::Matrix3d::Scalar& c
		 * @param Eigen::Vector3d& roots
		 */
		void computeRoots2(const Eigen::Matrix3d::Scalar& b, const Eigen::Matrix3d::Scalar& c, Eigen::Vector3d& roots);
};

} //@namespace utils
} //@namespace dwl


#endif
