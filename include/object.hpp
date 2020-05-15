#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <vector>
#include <string>
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <omp.h>
#include "mesh.hpp"
#include "joint.hpp"
#include "animation.hpp"

enum SMOKE_DIR
{
	SMOKE_LEFT,
	SMOKE_RIGHT
};

enum CONNECTOR_SIDE
{
	LEFT,
	RIGHT
};

class Object
{
	public:

		Object(const std::string& obj_path, bool drawable = false, bool p_lap = false, bool p_dynamic = false);
		~Object();
		Joint* get_skeleton();
		void draw(Shader& shader, DRAWING_MODE mode = SOLID);
		void draw(Shader& shader, Animation* anim, int frame);
		std::vector<Animation*> get_animations();
		std::map<std::string, Joint*> get_joints_ptr_list();
        std::vector<Mesh*> get_mesh_collection();
		static GLuint create_texture(std::string tex_path, bool flip = false);
        void reset_drawable();
		
		// smoke data
		std::vector<glm::vec3> get_left_sources() const;
		std::vector<glm::vec3> get_right_sources() const;
		glm::vec3 get_smoke_direction(SMOKE_DIR dir) const;

		// power coupling data
		std::vector<glm::vec3> get_connectors(CONNECTOR_SIDE side);

	private:

		void load(const std::string& file, bool drawable, bool p_lap, bool p_dynamic);
		void explore_node(aiNode* node, const aiScene* scene, bool drawable, bool p_lap, bool p_dynamic);
		Mesh* get_mesh(aiMesh* mesh, const aiScene* scene, bool drawable, bool p_lap, bool p_dynamic);
		int texture_already_loaded(std::string texture_path);
		
		// Animation related methods
		void create_joint_hierarchy(const aiScene* scene);
		void create_joint_hierarchy_aux(aiNode* bone, Joint* j, int& id);
		std::vector<aiNodeAnim*> get_parent_joints_list(Joint* j, aiAnimation* currAnim);
		glm::mat4 chainComputePoseMatrix(glm::mat4 matrix, std::vector<aiNodeAnim*> parentAnimJointList, int frame, aiNodeAnim* animJoint, std::vector<double> kFrameTime);
		glm::mat4 get_joint_poseMatrix_atFrame(aiNodeAnim* joint, int frame, glm::mat4 matrix);

		std::vector<Mesh*> mesh_collection;
		std::vector<Texture> texture_collection;
		Joint* skeleton;
		std::map<std::string, Joint*> joints_ptr_list;
		std::vector<Animation*> animations;

		// smoke emission sources
		glm::vec3 smoke_left_dir;
		glm::vec3 smoke_right_dir;
		std::vector<glm::vec3> sources_left;
		std::vector<glm::vec3> sources_right;

		// power coupling
		std::vector<glm::vec3> connectors_left;
		std::vector<glm::vec3> connectors_right;

		friend class Environment;
		friend class Podracer;
        friend class WorldPhysics;
		friend class Smoke;
		friend class Power;
};

#endif
