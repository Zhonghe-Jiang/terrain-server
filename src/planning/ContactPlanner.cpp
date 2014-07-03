#include <planning/ContactPlanner.h>


namespace dwl
{

namespace planning
{

ContactPlanner::ContactPlanner() : environment_(NULL)
{

}


ContactPlanner::~ContactPlanner()
{

}


void ContactPlanner::reset(environment::EnvironmentInformation* environment)
{
	printf(BLUE "Setting the environment information in the contact planner\n" COLOR_RESET);
	environment_ = environment;
}

bool ContactPlanner::computeFootholds(std::vector<Contact>& footholds, Pose current_pose)
{
	// Converting quaternion to roll, pitch and yaw angles
	double roll, pitch, yaw;
	Orientation orientation(current_pose.orientation);
	orientation.getRPY(roll, pitch, yaw);

	// Getting the vertex position
	Eigen::Vector2d vertex_position = current_pose.position.head(2);

	std::cout << "Current position = " << current_pose.position(0) << " " << current_pose.position(1) << " " << current_pose.position(2) << std::endl;
	for (int i = 0; i < robot_.getNumberOfLegs(); i++) {
		int current_leg_id = robot_.getNextLeg(i);
		CostMap terrain_costmap;
		environment_->getTerrainCostMap(terrain_costmap);

		double body_cost;
		// Computing the boundary of stance area
		Eigen::Vector2d boundary_min, boundary_max;
		boundary_min(0) = robot_.getStanceAreas()[current_leg_id].min_x + vertex_position(0);
		boundary_min(1) = robot_.getStanceAreas()[current_leg_id].min_y + vertex_position(1);
		boundary_max(0) = robot_.getStanceAreas()[current_leg_id].max_x + vertex_position(0);
		boundary_max(1) = robot_.getStanceAreas()[current_leg_id].max_y + vertex_position(1);

		std::set< std::pair<Weight, Vertex>, pair_first_less<Weight, Vertex> > stance_cost_queue;
		double stance_cost = 0;
		for (double y = boundary_min(1); y < boundary_max(1); y += robot_.getStanceAreas()[current_leg_id].grid_resolution) {
			for (double x = boundary_min(0); x < boundary_max(0); x += robot_.getStanceAreas()[current_leg_id].grid_resolution) {
				// Computing the rotated coordinate of the point inside the search area
				Eigen::Vector2d point_position;
				point_position(0) = (x - vertex_position(0)) * cos(yaw) - (y - vertex_position(1)) * sin(yaw) + vertex_position(0);
				point_position(1) = (x - vertex_position(0)) * sin(yaw) + (y - vertex_position(1)) * cos(yaw) + vertex_position(1);

				Vertex point = environment_->getGridModel().coordToVertex(point_position);

				// Inserts the element in an organized vertex queue, according to the maximun value
				if (terrain_costmap.find(point)->first == point)
					stance_cost_queue.insert(std::pair<Weight, Vertex>(terrain_costmap.find(point)->second, point));
			}
		}

		Contact foothold;
		foothold.end_effector = current_leg_id;
		if (stance_cost_queue.size() > 0)
			foothold.position << environment_->getGridModel().vertexToCoord(stance_cost_queue.begin()->second), 0.0;
		else
			foothold.position << robot_.getStancePosition()[current_leg_id] + vertex_position, 0.0;

		footholds.push_back(foothold);
		std::cout << "Footholds: leg = " << foothold.end_effector << " and position = " << foothold.position(0) << " " << foothold.position(1) << " " << foothold.position(2) << std::endl;
	}

	return true;
}

} //@namespace planning
} //@namespace dwl