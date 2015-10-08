#ifndef DWL_TERRAIN__REWARD_MAP_SERVER___H
#define DWL_TERRAIN__REWARD_MAP_SERVER___H

#include <ros/ros.h>
#include <octomap_msgs/Octomap.h>
#include <octomap/octomap.h>
#include <octomap_msgs/conversions.h>
#include <octomap/math/Utils.h>

#include <environment/RewardMap.h>
#include <environment/SlopeFeature.h>
#include <environment/HeightDeviationFeature.h>
#include <environment/CurvatureFeature.h>
#include <utils/Orientation.h>

#include <Eigen/Dense>
#include <vector>
#include <geometry_msgs/PoseArray.h>
#include <dwl_terrain/RewardMap.h>
#include <dwl_terrain/RewardCell.h>
#include <std_srvs/Empty.h>

#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <tf/message_filter.h>
#include <message_filters/subscriber.h>


namespace dwl_terrain
{

/**
 * @class RewardMapServer
 * @brief Class for building reward map of the environment
 */
class RewardMapServer
{
	public:
		/** @brief Constructor function */
		RewardMapServer(ros::NodeHandle node = ros::NodeHandle("~"));

		/** @brief Destructor function */
		~RewardMapServer();

		/** @brief Initialization of the reward map server */
		bool init();

		/**
		 * @brief Callback function when it arrives a octomap message
		 * @param const octomap_msgs::Octomap::ConstPtr& msg Octomap message
		 */
		void octomapCallback(const octomap_msgs::Octomap::ConstPtr& msg);

		/** @brief Resets the reward map */
		bool reset(std_srvs::Empty::Request& req, std_srvs::Empty::Response& resp);

		/** @brief Publishes a reward map */
		void publishRewardMap();


	private:
		/** @brief ROS node handle */
		ros::NodeHandle node_;

		/** @brief Private ROS node handle */
		ros::NodeHandle private_node_;

		/** @brief Pointer to the reward map class */
		dwl::environment::RewardMap reward_map_;

		/** @brief Reward map publisher */
		ros::Publisher reward_pub_;

		/** @brief Octomap subcriber */
		message_filters::Subscriber<octomap_msgs::Octomap>* octomap_sub_;

		/** @brief TF and octomap subscriber */
		tf::MessageFilter<octomap_msgs::Octomap>* tf_octomap_sub_;

		/** @brief Reset service */
		ros::ServiceServer reset_srv_;

		/** @brief Reward map message */
		dwl_terrain::RewardMap reward_map_msg_;

		/** @brief TF listener */
		tf::TransformListener tf_listener_;

		/** @brief Base frame */
		std::string base_frame_;

		/** @brief World frame */
		std::string world_frame_;

		/** @brief Indicates if it was computed new information of the reward map */
		bool new_information_;
};

} //@namespace dwl_terrain
#endif
