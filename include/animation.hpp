#ifndef _ANIMATION_HPP_
#define _ANIMATION_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

struct JointPose
{
	std::string name;
	glm::mat4 poseMatrix;
	
	JointPose(std::string p_name, glm::mat4 pMatrix)
	{
		name = p_name;
		poseMatrix = pMatrix;
	}
};

class Keyframe
{
	public:

		Keyframe(int p_frame);
		~Keyframe();
		int get_frame();
		void add_joint_pose(JointPose* jPose);
		JointPose* get_joint_pose_by_name(const char* joint_name);
		std::vector<JointPose*>& get_joint_pose_list();

	private:

		int frame;
		std::vector<JointPose*> joint_pose_list;
};

class Animation
{
	public:

		Animation(std::string p_name, double p_duration);
		std::string get_name() const;
		void add_keyframe(Keyframe* key);
		std::vector<Keyframe*>& get_keyframes();

	private:

		std::string name;
		double duration;
		std::vector<Keyframe*> keys;
};

#endif
