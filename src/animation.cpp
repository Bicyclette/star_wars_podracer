#include "animation.hpp"

Keyframe::Keyframe(int p_frame)
{
	frame = p_frame;
}

Keyframe::~Keyframe()
{
	int joint_pose_count = joint_pose_list.size();
	for(int i = 0; i < joint_pose_count; i++)
	{
		delete joint_pose_list.at(i);
	}
}

int Keyframe::get_frame()
{
	return frame;
}

void Keyframe::add_joint_pose(JointPose* jPose)
{
	joint_pose_list.push_back(jPose);
}

JointPose* Keyframe::get_joint_pose_by_name(const char* joint_name)
{
	int joint_pose_count = joint_pose_list.size();
	for(int i = 0; i < joint_pose_count; i++)
	{
		if(std::strcmp(joint_pose_list.at(i)->name.c_str(), joint_name) == 0)
			return joint_pose_list.at(i);
	}
	return nullptr;
}

std::vector<JointPose*>& Keyframe::get_joint_pose_list()
{
	return joint_pose_list;
}

Animation::Animation(std::string p_name, double p_duration) :
	name(p_name),
	duration(p_duration)
{}

std::string Animation::get_name() const
{
	return name;
}

void Animation::add_keyframe(Keyframe* key)
{
	keys.push_back(key);
}

std::vector<Keyframe*>& Animation::get_keyframes()
{
	return keys;
}
