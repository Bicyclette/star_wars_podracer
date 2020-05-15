#include "joint.hpp"

Joint::Joint(int p_id, std::string p_name, Joint* j_parent) :
	id(p_id),
	name(p_name),
	parent(j_parent)
{}

void Joint::add_child(Joint* j)
{
	children.push_back(j);
}

int Joint::get_id() const
{
	return id;
}

// always + 1 joint for the root bone
int Joint::get_nb_joints() const
{
	int direct_children_count = children.size();
	int sum = 0;
	for(int i = 0; i < direct_children_count; i++)
	{
		sum += children.at(i)->get_nb_joints();
	}
	return direct_children_count + sum;
}

std::string Joint::get_name() const
{
	return name;
}

Joint* Joint::get_parent()
{
	return parent;
}

std::vector<Joint*> Joint::get_direct_children()
{
	return children;
}

glm::mat4 Joint::get_transform()
{
	return transform;
}

glm::mat4 Joint::get_inverse_transform()
{
	return inv_transform;
}

void Joint::print_hierarchy(Joint* j, int nb_space) const
{
	if(nb_space == 0)
	{
		std::cout << j->name << " ID = " << j->get_id() << std::endl;
		nb_space += 4;
	}
	int nb_children = j->children.size();
	for(int i = 0; i < nb_children; i++)
	{
		for(int space = 0; space < nb_space; space++)
			std::cout << " ";
		std::cout << j->children.at(i)->name << " ID = " << j->children.at(i)->get_id() << std::endl;
		print_hierarchy(j->children.at(i), nb_space + 4);
	}
}

void Joint::set_transform(glm::mat4 t)
{
	transform = t;
}

void Joint::set_inverse_transform(glm::mat4 t)
{
	inv_transform = t;
}

Joint* Joint::find_by_name(Joint* joint, const char* joint_name)
{
	if(std::strcmp(joint->name.c_str(), joint_name) == 0)
	{
		return joint;
	}
	else
	{
		int direct_children_count = joint->get_direct_children().size();
		if(direct_children_count == 0)
			return nullptr;
		else
		{
			Joint* res = nullptr;
			for(int i = 0; i < direct_children_count; i++)
			{
				res = find_by_name(joint->children[i], joint_name);
				if(res != nullptr)
					return res;
			}
			return res;
		}
	}
}
