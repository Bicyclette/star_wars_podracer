#ifndef _JOINT_HPP_
#define _JOINT_HPP_

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <vector>
#include <string>

class Joint
{
	public:

		Joint(int p_id, std::string p_name, Joint* j_parent);
		void add_child(Joint* j);
		int get_id() const;
		std::string get_name() const;
		Joint* get_parent();
		std::vector<Joint*> get_direct_children();
		glm::mat4 get_transform();
		glm::mat4 get_inverse_transform();
		int get_nb_joints() const;
		void set_transform(glm::mat4 t);
		void set_inverse_transform(glm::mat4 t);
		void print_hierarchy(Joint* j, int nb_space) const;
		Joint* find_by_name(Joint* joint, const char* joint_name);

	private:

		int id;
		std::string name;
		Joint* parent;
		std::vector<Joint*> children;
		glm::mat4 transform; // from mesh to bone space
		glm::mat4 inv_transform; // from bone space to mesh space
};

#endif
