#include <dwl_planners/ConstrainedWholeBodyPlanner.h>


namespace dwl_planners
{

ConstrainedWholeBodyPlanner::ConstrainedWholeBodyPlanner(ros::NodeHandle node) : privated_node_(node),
		interpolation_time_(0.), computation_time_(0.), new_current_state_(false)
{
	current_state_.joint_pos = Eigen::VectorXd::Zero(2);
	current_state_.joint_pos << 0.6, -1.5;
	current_state_.joint_vel = Eigen::VectorXd::Zero(2);
	current_state_.joint_acc = Eigen::VectorXd::Zero(2);
	current_state_.joint_eff = Eigen::VectorXd::Zero(2);
	current_state_.joint_eff << 9.33031, 27.6003;
}


ConstrainedWholeBodyPlanner::~ConstrainedWholeBodyPlanner()
{

}


void ConstrainedWholeBodyPlanner::init()
{
	// Setting publishers and subscribers
	motion_plan_pub_ = node_.advertise<dwl_msgs::WholeBodyTrajectory>("/hyl/constrained_operational_controller/plan", 1);
	robot_state_sub_ = node_.subscribe<sensor_msgs::JointState>("/hyl/joint_states", 1,
			&ConstrainedWholeBodyPlanner::jointStateCallback, this);

	// Initializing the planning optimizer
	dwl::solver::IpoptNLP* ipopt_solver = new dwl::solver::IpoptNLP();
	dwl::solver::OptimizationSolver* solver = ipopt_solver;
	planning_.init(solver);


	// Initializing the dynamical system constraint
	std::string model_file = "/home/cmastalli/ros_workspace/src/dwl/thirdparty/rbdl/hyl.urdf";
	dwl::model::ConstrainedDynamicalSystem* system_constraint = new dwl::model::ConstrainedDynamicalSystem();
	dwl::model::DynamicalSystem* dynamical_system = system_constraint;

	dwl::rbd::BodySelector active_contact;
	active_contact.push_back("foot");
	system_constraint->setActiveEndEffectors(active_contact);
	dynamical_system->modelFromURDFFile(model_file, true);

	// Adding the dynamical system
	planning_.addDynamicalSystem(dynamical_system);
	dwl::model::FloatingBaseSystem system = planning_.getDynamicalSystem()->getFloatingBaseSystem();


	// Reading the desired position states
	desired_state_.setJointDoF(system.getJointDoF());
	double linear_x, linear_y, linear_z, angular_x, angular_y, angular_z;
	privated_node_.param("desired_state/position/LX", linear_x, 0.0);
	privated_node_.param("desired_state/position/LY", linear_y, 0.0);
	privated_node_.param("desired_state/position/LZ", linear_z, 0.0);
	privated_node_.param("desired_state/position/AX", angular_x, 0.0);
	privated_node_.param("desired_state/position/AY", angular_y, 0.0);
	privated_node_.param("desired_state/position/AZ", angular_z, 0.0);
	desired_state_.base_pos(dwl::rbd::LX) = linear_x;
	desired_state_.base_pos(dwl::rbd::LY) = linear_y;
	desired_state_.base_pos(dwl::rbd::LZ) = linear_z;
	desired_state_.base_pos(dwl::rbd::AX) = angular_x;
	desired_state_.base_pos(dwl::rbd::AY) = angular_y;
	desired_state_.base_pos(dwl::rbd::AZ) = angular_z;

	// Reading the desired velocity states
	privated_node_.param("desired_state/velocity/LX", linear_x, 0.0);
	privated_node_.param("desired_state/velocity/LY", linear_y, 0.0);
	privated_node_.param("desired_state/velocity/LZ", linear_z, 0.0);
	privated_node_.param("desired_state/velocity/AX", angular_x, 0.0);
	privated_node_.param("desired_state/velocity/AY", angular_y, 0.0);
	privated_node_.param("desired_state/velocity/AZ", angular_z, 0.0);
	desired_state_.base_vel(dwl::rbd::LX) = linear_x;
	desired_state_.base_vel(dwl::rbd::LY) = linear_y;
	desired_state_.base_vel(dwl::rbd::LZ) = linear_z;
	desired_state_.base_vel(dwl::rbd::AX) = angular_x;
	desired_state_.base_vel(dwl::rbd::AY) = angular_y;
	desired_state_.base_vel(dwl::rbd::AZ) = angular_z;

	// Reading the desired acceleration states
	privated_node_.param("desired_state/acceleration/LX", linear_x, 0.0);
	privated_node_.param("desired_state/acceleration/LY", linear_y, 0.0);
	privated_node_.param("desired_state/acceleration/LZ", linear_z, 0.0);
	privated_node_.param("desired_state/acceleration/AX", angular_x, 0.0);
	privated_node_.param("desired_state/acceleration/AY", angular_y, 0.0);
	privated_node_.param("desired_state/acceleration/AZ", angular_z, 0.0);
	desired_state_.base_acc(dwl::rbd::LX) = linear_x;
	desired_state_.base_acc(dwl::rbd::LY) = linear_y;
	desired_state_.base_acc(dwl::rbd::LZ) = linear_z;
	desired_state_.base_acc(dwl::rbd::AX) = angular_x;
	desired_state_.base_acc(dwl::rbd::AY) = angular_y;
	desired_state_.base_acc(dwl::rbd::AZ) = angular_z;


	// Reading the cost weights
	dwl::LocomotionState weights(system.getJointDoF());

	// Base position weights
	privated_node_.param("cost/state_tracking_energy/position/base/LX", linear_x, 0.0);
	privated_node_.param("cost/state_tracking_energy/position/base/LY", linear_y, 0.0);
	privated_node_.param("cost/state_tracking_energy/position/base/LZ", linear_z, 0.0);
	privated_node_.param("cost/state_tracking_energy/position/base/AX", angular_x, 0.0);
	privated_node_.param("cost/state_tracking_energy/position/base/AY", angular_y, 0.0);
	privated_node_.param("cost/state_tracking_energy/position/base/AZ", angular_z, 0.0);
	weights.base_pos(dwl::rbd::LX) = linear_x;
	weights.base_pos(dwl::rbd::LY) = linear_y;
	weights.base_pos(dwl::rbd::LZ) = linear_z;
	weights.base_pos(dwl::rbd::AX) = angular_x;
	weights.base_pos(dwl::rbd::AY) = angular_y;
	weights.base_pos(dwl::rbd::AZ) = angular_z;

	// Base velocity weights
	privated_node_.param("cost/state_tracking_energy/velocity/base/LX", linear_x, 0.0);
	privated_node_.param("cost/state_tracking_energy/velocity/base/LY", linear_y, 0.0);
	privated_node_.param("cost/state_tracking_energy/velocity/base/LZ", linear_z, 0.0);
	privated_node_.param("cost/state_tracking_energy/velocity/base/AX", angular_x, 0.0);
	privated_node_.param("cost/state_tracking_energy/velocity/base/AY", angular_y, 0.0);
	privated_node_.param("cost/state_tracking_energy/velocity/base/AZ", angular_z, 0.0);
	weights.base_vel(dwl::rbd::LX) = linear_x;
	weights.base_vel(dwl::rbd::LY) = linear_y;
	weights.base_vel(dwl::rbd::LZ) = linear_z;
	weights.base_vel(dwl::rbd::AX) = angular_x;
	weights.base_vel(dwl::rbd::AY) = angular_y;
	weights.base_vel(dwl::rbd::AZ) = angular_z;

	// Base acceleration weights
	privated_node_.param("cost/state_tracking_energy/acceleration/base/LX", linear_x, 0.0);
	privated_node_.param("cost/state_tracking_energy/acceleration/base/LY", linear_y, 0.0);
	privated_node_.param("cost/state_tracking_energy/acceleration/base/LZ", linear_z, 0.0);
	privated_node_.param("cost/state_tracking_energy/acceleration/base/AX", angular_x, 0.0);
	privated_node_.param("cost/state_tracking_energy/acceleration/base/AY", angular_y, 0.0);
	privated_node_.param("cost/state_tracking_energy/acceleration/base/AZ", angular_z, 0.0);
	weights.base_acc(dwl::rbd::LX) = linear_x;
	weights.base_acc(dwl::rbd::LY) = linear_y;
	weights.base_acc(dwl::rbd::LZ) = linear_z;
	weights.base_acc(dwl::rbd::AX) = angular_x;
	weights.base_acc(dwl::rbd::AY) = angular_y;
	weights.base_acc(dwl::rbd::AZ) = angular_z;


	// Joint weights
	for (dwl::urdf_model::JointID::const_iterator jnt_it = system.getJoints().begin();
			jnt_it != system.getJoints().end(); jnt_it++) {
		unsigned int joint_idx =  jnt_it->second - system.getFloatingBaseDoF();
		std::string joint_name = jnt_it->first;
		double weight_value;

		// Position weights
		if (!privated_node_.getParam("cost/state_tracking_energy/position/joints/" + joint_name,
				weight_value))
			ROS_WARN("The position weight of the %s joint is not defined", joint_name.c_str());
		else
			weights.joint_pos(joint_idx) = weight_value;

		// Velocity weights
		if (!privated_node_.getParam("cost/state_tracking_energy/velocity/joints/" + joint_name,
				weight_value))
			ROS_WARN("The velocity weight of the %s joint is not defined", joint_name.c_str());
		else
			weights.joint_vel(joint_idx) = weight_value;

		// Acceleration weights
		if (!privated_node_.getParam("cost/state_tracking_energy/acceleration/joints/" + joint_name,
				weight_value))
			ROS_WARN("The acceleration weight of the %s joint is not defined", joint_name.c_str());
		else
			weights.joint_acc(joint_idx) = weight_value;

		// Control weights
		if (!privated_node_.getParam("cost/control_energy/" + joint_name, weight_value))
			ROS_WARN("The control weight of the %s joint is not defined", joint_name.c_str());
		else
			weights.joint_eff(joint_idx) = weight_value;
	}

	// Setting the cost functions
	dwl::model::Cost* state_tracking_cost = new dwl::model::StateTrackingEnergyCost();
	state_tracking_cost->setWeights(weights);
	dwl::model::Cost* control_cost = new dwl::model::ControlEnergyCost();
	control_cost->setWeights(weights);


	// Adding the cost functions
	planning_.addCost(state_tracking_cost);
	planning_.addCost(control_cost);


	// Reading the interpolation time
	privated_node_.param("interpolation_time", interpolation_time_, -1.);

	// Reading and setting the horizon value
	int horizon;
	privated_node_.param("horizon", horizon, 1);
	planning_.setHorizon(horizon);

	// Reading the allowed computation time
	privated_node_.param("computation_time", computation_time_, std::numeric_limits<double>::max());
}


bool ConstrainedWholeBodyPlanner::compute()
{
	if (new_current_state_) {
		new_current_state_ = false;
		return planning_.compute(current_state_, desired_state_, computation_time_);
	} else
		return false;
}


void ConstrainedWholeBodyPlanner::publishWholeBodyTrajectory()
{
	// Publishing the motion plan if there is at least one subscriber
	if (motion_plan_pub_.getNumSubscribers() > 0) {
		robot_trajectory_msg_.header.stamp = ros::Time::now();

		// Filling the current state
		writeWholeBodyStateMessage(robot_trajectory_msg_.actual,
								   current_state_);

		// Filling the trajectory message
		dwl::locomotion::LocomotionTrajectory trajectory;
		if (interpolation_time_ <= 0.)
			trajectory = planning_.getWholeBodyTrajectory();
		else
			trajectory = planning_.getInterpolatedWholeBodyTrajectory(interpolation_time_);

		robot_trajectory_msg_.trajectory.resize(trajectory.size());
		for (unsigned int i = 0; i < trajectory.size(); i++)
			writeWholeBodyStateMessage(robot_trajectory_msg_.trajectory[i], trajectory[i]);

		// Publishing the motion plan
		motion_plan_pub_.publish(robot_trajectory_msg_);
	}
}



void ConstrainedWholeBodyPlanner::writeWholeBodyStateMessage(dwl_msgs::WholeBodyState& msg,
															 const dwl::LocomotionState& state)
{
	// Getting the floating-base system information
	dwl::model::FloatingBaseSystem system = planning_.getDynamicalSystem()->getFloatingBaseSystem();

	// Filling the time information
	msg.time = state.time;

	// Filling the base state
	msg.base.resize(system.getFloatingBaseDoF());
	unsigned int counter = 0;
	for (unsigned int base_idx = 0; base_idx < 6; base_idx++) {
		dwl::rbd::Coords6d base_coord = dwl::rbd::Coords6d(base_idx);
		dwl::model::FloatingBaseJoint base_joint = system.getFloatingBaseJoint(base_coord);

		if (base_joint.active) {
			msg.base[counter].id = base_idx;
			msg.base[counter].name = base_joint.name;

			msg.base[counter].position = state.base_pos(base_idx);
			msg.base[counter].velocity = state.base_vel(base_idx);
			msg.base[counter].acceleration = state.base_acc(base_idx);

			counter++;
		}
	}

	// Filling the joint state
	msg.joints.resize(system.getJointDoF());
	for (dwl::urdf_model::JointID::const_iterator jnt_it = system.getJoints().begin();
			jnt_it != system.getJoints().end(); jnt_it++) {
		unsigned int joint_idx =  jnt_it->second - system.getFloatingBaseDoF();
		std::string joint_name = jnt_it->first;

		msg.joints[joint_idx].name = joint_name;
		msg.joints[joint_idx].position = state.joint_pos(joint_idx);
		msg.joints[joint_idx].velocity = state.joint_vel(joint_idx);
		msg.joints[joint_idx].acceleration = state.joint_acc(joint_idx);
		msg.joints[joint_idx].effort = state.joint_eff(joint_idx);
	}
}


void ConstrainedWholeBodyPlanner::jointStateCallback(const sensor_msgs::JointStateConstPtr& msg)
{
	// Getting the floating system information
	dwl::model::FloatingBaseSystem system = planning_.getDynamicalSystem()->getFloatingBaseSystem();

	// Setting the base and joint information
	current_state_.base_pos.setZero();
	current_state_.base_vel.setZero();
	current_state_.base_acc.setZero();
	dwl::urdf_model::JointID joints = system.getJoints();
	for (unsigned int jnt_idx = 0; jnt_idx < system.getSystemDoF(); jnt_idx++) {
		std::string joint_name = msg->name[jnt_idx];

		dwl::urdf_model::JointID::iterator joint_it = joints.find(joint_name);
		if (joint_it != joints.end()) {
			unsigned int joint_id = joint_it->second - system.getFloatingBaseDoF();

			current_state_.joint_pos(joint_id) = msg->position[jnt_idx];
			current_state_.joint_vel(joint_id) = msg->velocity[jnt_idx];
			current_state_.joint_acc(joint_id) = 0.;
			current_state_.joint_eff(joint_id) = msg->effort[jnt_idx];
		} else {
			for (unsigned int base_idx = 0; base_idx < 6; base_idx++) {
				dwl::rbd::Coords6d base_coord = dwl::rbd::Coords6d(base_idx);
				dwl::model::FloatingBaseJoint base_joint = system.getFloatingBaseJoint(base_coord);

				if (base_joint.active) {
					if (base_joint.name == joint_name) {
						current_state_.base_pos(base_coord) = msg->position[jnt_idx];
						current_state_.base_vel(base_coord) = msg->velocity[jnt_idx];
						current_state_.base_acc(base_coord) = 0.;
					}
				}
			}
		}
	}

	new_current_state_ = true;
}

} //@namespace dwl_planners




int main(int argc, char **argv)
{
	ros::init(argc, argv, "constrained_whole_body_planner");

	dwl_planners::ConstrainedWholeBodyPlanner planner;

	planner.init();
	ros::spinOnce();

	try {
		ros::Rate loop_rate(0.25);

		while (ros::ok()) {
			if (planner.compute()) {
				planner.publishWholeBodyTrajectory();
				return 0;
			}
			ros::spinOnce();
			loop_rate.sleep();
		}
	} catch (std::runtime_error& e) {
		ROS_ERROR("constrained_whole_body_planner exception: %s", e.what());
		return -1;
	}

	return 0;
}

