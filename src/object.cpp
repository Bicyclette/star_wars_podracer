/**
 * \file
 * Shape by shape
 * \author Mathias Velo
 */

#include "object.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.hpp"

#include <glm/gtx/string_cast.hpp>

glm::mat4 assimpMat4_to_glmMat4(aiMatrix4x4& m)
{
	glm::mat4 matrix;
	matrix[0][0] = m.a1; matrix[1][0] = m.a2; matrix[2][0] = m.a3; matrix[3][0] = m.a4;
	matrix[0][1] = m.b1; matrix[1][1] = m.b2; matrix[2][1] = m.b3; matrix[3][1] = m.b4;
	matrix[0][2] = m.c1; matrix[1][2] = m.c2; matrix[2][2] = m.c3; matrix[3][2] = m.c4;
	matrix[0][3] = m.d1; matrix[1][3] = m.d2; matrix[2][3] = m.d3; matrix[3][3] = m.d4;
	return matrix;
}

glm::mat3 assimpMat3_to_glmMat3(aiMatrix3x3& m)
{
	glm::mat3 matrix;
	matrix[0][0] = m.a1; matrix[1][0] = m.a2; matrix[2][0] = m.a3; 
	matrix[0][1] = m.b1; matrix[1][1] = m.b2; matrix[2][1] = m.b3; 
	matrix[0][2] = m.c1; matrix[1][2] = m.c2; matrix[2][2] = m.c3; 
	return matrix;
}

Object::Object(const std::string& obj_path, bool drawable, bool p_lap, bool p_dynamic)
{
	skeleton = nullptr;
	load(obj_path, drawable, p_lap, p_dynamic);
}

Object::~Object()
{
	delete skeleton;
	int nb_animations = animations.size();
	for(int i = 0; i < nb_animations; i++)
	{
		delete animations.at(i);
	}
}

Joint* Object::get_skeleton()
{
	return skeleton;
}

void Object::draw(Shader& shader, DRAWING_MODE mode)
{
	int mesh_count = mesh_collection.size();

	for(int i = 0; i < mesh_count; i++)
	{
        if(mesh_collection.at(i)->is_drawable())
        {
		    mesh_collection.at(i)->draw(shader, mode);
        }
	}
}

void Object::draw(Shader& shader, Animation* anim, int frame)
{
	int mesh_count = mesh_collection.size();

	for(int i = 0; i < mesh_count; i++)
	{
		mesh_collection.at(i)->draw(shader, joints_ptr_list, anim, frame, skeleton);
	}
}

std::vector<Animation*> Object::get_animations()
{
	return animations;
}

std::map<std::string, Joint*> Object::get_joints_ptr_list()
{
	return joints_ptr_list;
}

std::vector<Mesh*> Object::get_mesh_collection()
{
    return mesh_collection;
}

std::vector<glm::vec3> Object::get_left_sources() const {return sources_left;}

std::vector<glm::vec3> Object::get_right_sources() const {return sources_right;}

std::vector<glm::vec3> Object::get_connectors(CONNECTOR_SIDE side)
{
	if(side == LEFT)
		return connectors_left;
	else
		return connectors_right;
}

glm::vec3 Object::get_smoke_direction(SMOKE_DIR dir) const
{
	if(dir == SMOKE_LEFT)
	{
		return smoke_left_dir;
	}
	else if(dir == SMOKE_RIGHT)
	{
		return smoke_right_dir;
	}
}

void Object::load(const std::string& file, bool drawable, bool p_lap, bool p_dynamic)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || ! scene->mRootNode)
	{
		std::cerr << importer.GetErrorString() << std::endl;
		return;
	}

	// calculate loading time
	double load_start = omp_get_wtime();
	double load_end;

	if(scene->HasAnimations())
	{
		create_joint_hierarchy(scene);
		std::cout << "##### JOINT HIERARCHY CREATED ! #####" << std::endl;
		std::cout << skeleton->get_name() << std::endl;
		skeleton->print_hierarchy(skeleton, 4);
		int nb_animations = scene->mNumAnimations;
		aiAnimation** anims = scene->mAnimations;
		
		// for each animation
		for(int j = 0; j < nb_animations; j++)
		{
			std::string anim_name(anims[j]->mName.C_Str());
			std::cout << "	- LOADING ANIMATION: " << anim_name << std::endl;
			//double anim_duration = anims[j]->mDuration;
			double anim_duration = anims[j]->mChannels[0]->mNumPositionKeys;
			std::cout << "	anim_duration = " << anim_duration << std::endl;
			Animation* currentAnimation = new Animation(anim_name, anim_duration);
			
			// create keyframes for the current animation for each joint
			for(int k = 0; k < anim_duration; k++)
			{
				Keyframe* kFrame = new Keyframe(k);
				int joints_involved_count = anims[j]->mNumChannels;

				for(int i = 0; i < joints_involved_count; i++)
				{
					aiNodeAnim* animJoint = anims[j]->mChannels[i];
					if(std::strcmp(animJoint->mNodeName.C_Str(), "Armature") == 0)
						continue;
					int posKeysCount = animJoint->mNumPositionKeys;
					std::vector<double> keyFrameTime;
					for(int i = 0; i < posKeysCount; i++)
					{
						keyFrameTime.push_back(animJoint->mPositionKeys[i].mTime);
					}
					std::string joint_name(animJoint->mNodeName.C_Str());
					Joint* animJointSame = joints_ptr_list[joint_name];
					std::vector<aiNodeAnim*> parentAnimJoint_list = get_parent_joints_list(animJointSame, anims[j]);
					
					// now chain compute pose Matrix
					glm::mat4 poseMatrix = glm::mat4(1.0f);
					poseMatrix = chainComputePoseMatrix(poseMatrix, parentAnimJoint_list, k, animJoint, keyFrameTime);

					// create JointPose and add it to the current keyframe
					JointPose* jPose = new JointPose(joint_name, poseMatrix);
					kFrame->add_joint_pose(jPose);
				}
				currentAnimation->add_keyframe(kFrame);
			}
			
			// add current animation to the animations collection
			animations.push_back(currentAnimation);
		}
	}
	
	std::cout << std::endl;
	explore_node(scene->mRootNode, scene, drawable, p_lap, p_dynamic);
	
	// loading time
	load_end = omp_get_wtime();
	std::cout << "LOADING TIME = " << load_end - load_start << " seconds." << std::endl << std::endl;
}

void Object::explore_node(aiNode* node, const aiScene* scene, bool drawable, bool p_lap, bool p_dynamic)
{
	int nb_meshes = node->mNumMeshes;
	int nb_children = node->mNumChildren;
	for(int i = 0; i < nb_meshes; i++)
	{
		aiMesh* currMesh = scene->mMeshes[node->mMeshes[i]];
		std::cout << "	- LOADING MESH: " << currMesh->mName.C_Str() << std::endl;
		Mesh* mesh = get_mesh(currMesh, scene, drawable, p_lap, p_dynamic);
		mesh_collection.push_back(mesh);
	}

	for(int i = 0; i < nb_children; i++)
	{
		explore_node(node->mChildren[i], scene, drawable, p_lap, p_dynamic);
	}
}

Mesh* Object::get_mesh(aiMesh* mesh, const aiScene* scene, bool drawable, bool p_lap, bool p_dynamic)
{
	// vertices
	std::string mesh_name(mesh->mName.C_Str());
	aiBone** bones_tab= mesh->mBones;
	int bones_count = mesh->mNumBones;
	
	int nb_vertices = mesh->mNumVertices;
	std::vector<Vertex> vertices;
	glm::vec3 v_pos;
	glm::vec3 v_norm;
	glm::vec2 v_tex_coords;
	glm::vec2 v_bonesID = glm::vec2(-1.0f, -1.0f);
	glm::vec2 v_bonesWeight = glm::vec2(-1.0f, -1.0f);
	
	aiVector3D vertex_pos;
	aiVector3D vertex_norm;
	
	#pragma omp for
	for(int i = 0; i < nb_vertices; i++)
	{
		vertex_pos = mesh->mVertices[i];
		vertex_norm = mesh->mNormals[i];

		v_pos = glm::vec3(vertex_pos.x, vertex_pos.y, vertex_pos.z);
		v_norm = glm::vec3(vertex_norm.x, vertex_norm.y, vertex_norm.z);
		
		if(mesh->mTextureCoords[0])
		{
			float u, v;
			u = mesh->mTextureCoords[0][i].x;
			v = mesh->mTextureCoords[0][i].y;
			v_tex_coords = glm::vec2(u, v);
		}
		else
			v_tex_coords = glm::vec2(0.0f, 0.0f);

		for(int b = 0; b < bones_count && (v_bonesID.x == -1 || v_bonesID.y == -1); b++)
		{
			aiBone* currentBone = bones_tab[b];
			int vertices_affected_by_currentBone = currentBone->mNumWeights;
			
			for(int v = 0; v < vertices_affected_by_currentBone && (v_bonesID.x == -1 || v_bonesID.y == -1); v++)
			{
				// if the current vertex [i] is affected by the current bone
				if(currentBone->mWeights[v].mVertexId == i)
				{
					// get the bone ID
					const char* current_bone_name = currentBone->mName.C_Str();
					Joint* current_joint = joints_ptr_list[current_bone_name];
					int boneID = current_joint->get_id();

					// and the weight applied to the affected vertex
					float weight = currentBone->mWeights[v].mWeight;

					// finally, push those data into v_bonesID and v_bonesWeight
					if(v_bonesID.x == -1)
						v_bonesID.x = static_cast<float>(boneID);
					else if(v_bonesID.y == -1)
						v_bonesID.y = static_cast<float>(boneID);

					if(v_bonesWeight.x == -1.0f)
						v_bonesWeight.x = weight;
					else if(v_bonesWeight.y == -1.0f)
						v_bonesWeight.y = weight;
				}
			}
		}

		Vertex v(v_pos, v_norm, v_tex_coords, v_bonesID, v_bonesWeight);
		if(mesh_name == "smoke")
		{
            if(v_pos.x > 0.0f)
            {
			    sources_left.push_back(v_pos);
			    smoke_left_dir = glm::normalize(v_norm);
            }
			else
            {
                sources_right.push_back(v_pos);
			    smoke_right_dir = glm::normalize(v_norm);
            }
		}
		else if(mesh_name == "connector")
		{
            if(v_pos.x > 0.0f)
			    connectors_left.push_back(v_pos);
            else
			    connectors_right.push_back(v_pos);
		}

		v_bonesID.x = -1.0f; v_bonesID.y = -1.0f;
		v_bonesWeight.x = -1.0f; v_bonesWeight.y = -1.0f;
		vertices.push_back(v);
	}

	// indices
	int nb_faces = mesh->mNumFaces;
	int nb_indices_face = 0;
	std::vector<int> indices;

	for(int i = 0; i < nb_faces; i++)
	{
		aiFace face = mesh->mFaces[i];
		nb_indices_face = face.mNumIndices;

		for(int j = 0; j < nb_indices_face; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// material
	aiMaterial* mesh_material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> mesh_tex_set;
    aiColor3D base_color;
	float mesh_shininess;

	int nb_diffuse_tex = mesh_material->GetTextureCount(aiTextureType_DIFFUSE);
	int nb_specular_tex = mesh_material->GetTextureCount(aiTextureType_SPECULAR);
	
	for(int i = 0; i < nb_diffuse_tex; i++)
	{
		aiString texture_path;
		mesh_material->GetTexture(aiTextureType_DIFFUSE, i, &texture_path);
		std::string tex_path(texture_path.C_Str());
		
        int lastSlashPos = tex_path.find_last_of("/");
		tex_path = tex_path.substr(lastSlashPos + 1);
		tex_path = "../assets/textures/podracer/" + tex_path;

		int tex_index = texture_already_loaded(tex_path);
		if(tex_index == -1)
		{
			GLuint tex_id = create_texture(tex_path);

			Texture tex(tex_id, TextureType::DIFFUSE_TEXTURE, tex_path);
			mesh_tex_set.push_back(tex);
			texture_collection.push_back(tex);
		}
		else
		{
			mesh_tex_set.push_back(texture_collection.at(tex_index));
		}
	}
	
	for(int i = 0; i < nb_specular_tex; i++)
	{
		aiString texture_path;
		mesh_material->GetTexture(aiTextureType_SPECULAR, i, &texture_path);
		std::string tex_path(texture_path.C_Str());
		
		int lastSlashPos = tex_path.find_last_of("/");
		tex_path = tex_path.substr(lastSlashPos + 1);
		tex_path = "../assets/textures/podracer/" + tex_path;
		
		int tex_index = texture_already_loaded(tex_path);
		if(tex_index == -1)
		{
			GLuint tex_id = create_texture(tex_path);
		
			Texture tex(tex_id, TextureType::SPECULAR_TEXTURE, tex_path);
			mesh_tex_set.push_back(tex);
		}
		else
		{
			mesh_tex_set.push_back(texture_collection.at(tex_index));
		}
	}

	mesh_material->Get(AI_MATKEY_COLOR_DIFFUSE, base_color);
	mesh_material->Get(AI_MATKEY_SHININESS, mesh_shininess);

	Material m;
	m.textures = mesh_tex_set;
    m.base_color = glm::vec3(base_color.r, base_color.g, base_color.b);
	m.shininess = mesh_shininess / 8.0f;

	// pack everything
    Mesh * retrieved_mesh;
    if(p_lap)
	    retrieved_mesh = new Mesh(vertices, indices, m, mesh_name, drawable, p_dynamic, p_lap);
    else
	    retrieved_mesh = new Mesh(vertices, indices, m, mesh_name, drawable, p_dynamic);

	// final step
	return retrieved_mesh;
}

GLuint Object::create_texture(std::string tex_path, bool flip)
{
	int width, height, channels;

	GLuint tex;
	glGenTextures(1, &tex);
	stbi_set_flip_vertically_on_load(flip);
	std::cout << "texture_path = " << tex_path << std::endl;
	unsigned char* img_data = stbi_load(tex_path.c_str(), &width, &height, &channels, 0);
	if(img_data != nullptr)
	{
		GLenum format_src;
		GLenum format_dest;
		if(channels == 3)
		{
			format_src = GL_SRGB;
			format_dest = GL_RGB;
		}
		else if(channels == 4)
		{
			format_src = GL_SRGB_ALPHA;
			format_dest = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, format_src, width, height, 0, format_dest, GL_UNSIGNED_BYTE, img_data);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(img_data);
	}
	else
	{
		std::cout << "Error while trying to load texture !" << std::endl;
		stbi_image_free(img_data);
	}

	return tex;
}

int Object::texture_already_loaded(std::string texture_path)
{
	int tex_count = texture_collection.size();

	for(int i = 0; i < tex_count; i++)
	{
		if(texture_collection.at(i).texture_file_path == texture_path)
			return i;
	}
	return -1;
}

void Object::create_joint_hierarchy(const aiScene* scene)
{
	// find the Armature node
	aiNode* root = scene->mRootNode;
	aiNode* Armature_node = root->FindNode("Armature");
	
	// construct joint hierarchy
	int currentID = 1;
	aiNode* root_bone = Armature_node->mChildren[0];
	skeleton = new Joint(currentID, root_bone->mName.C_Str(), nullptr);
	create_joint_hierarchy_aux(root_bone, skeleton, currentID);

	// set initial transform matrix for each joint
	aiMesh** meshes = scene->mMeshes;
	int mesh_count = scene->mNumMeshes;
	
	for(int i = 0; i < mesh_count; i++)
	{
		aiMesh* m = meshes[i];
		if(m->HasBones())
		{
			int bone_count = m->mNumBones;

			for(int b = 0; b < bone_count; b++)
			{
				aiBone* currentBone = m->mBones[b];
				Joint* currentJoint = joints_ptr_list[currentBone->mName.C_Str()];
				
				aiMatrix4x4 mat4 = currentBone->mOffsetMatrix;
				glm::mat4 matrix = assimpMat4_to_glmMat4(mat4);
				
				currentJoint->set_transform(matrix);
				currentJoint->set_inverse_transform(glm::inverse(matrix));
			}
		}
	}
}

void Object::create_joint_hierarchy_aux(aiNode* bone, Joint* j, int& id)
{
	joints_ptr_list[j->get_name()] = j;
	int children_count = bone->mNumChildren;

	for(int i = 0; i < children_count; i++)
	{
		std::string bone_name(bone->mChildren[i]->mName.C_Str());
		if(bone_name.find("_end") == std::string::npos)
		{
			id++;
			Joint* child_joint = new Joint(id, bone->mChildren[i]->mName.C_Str(), j);
			j->add_child(child_joint);
			create_joint_hierarchy_aux(bone->mChildren[i], child_joint, id);
		}
	}
}

std::vector<aiNodeAnim*> Object::get_parent_joints_list(Joint* j, aiAnimation* currAnim)
{
	std::vector<aiNodeAnim*> parent_joints_list;
	while(j->get_parent() != nullptr)
	{
		int nodeAnimCount = currAnim->mNumChannels;
		aiNodeAnim* nodeAnim;
		for(int i = 0; i < nodeAnimCount; i++)
		{
			if(std::strcmp(currAnim->mChannels[i]->mNodeName.C_Str(), j->get_parent()->get_name().c_str()) == 0)
			{
				nodeAnim = currAnim->mChannels[i];
				parent_joints_list.push_back(nodeAnim);
				j = j->get_parent();
				break;
			}
		}
	}
	return parent_joints_list;
}

glm::mat4 Object::chainComputePoseMatrix(glm::mat4 matrix, std::vector<aiNodeAnim*> parentAnimJointList, int frame, aiNodeAnim* animJoint, std::vector<double> kFrameTime)
{
	int parent_count = parentAnimJointList.size();
	aiNodeAnim* currentParent;
	
	if(parent_count == 0)
	{
		// if current animJoint contains the frame
		//if(std::find(kFrameTime.begin(), kFrameTime.end(), frame) != kFrameTime.end())
		//{
			matrix = get_joint_poseMatrix_atFrame(animJoint, frame, matrix);
		//}
		/*else // else take last frame pos, rot, scale data into poseMatrix
		{
			int lastFrame = frame;
			while(lastFrame >= 0 && std::find(kFrameTime.begin(), kFrameTime.end(), lastFrame) == kFrameTime.end())
			{
				lastFrame--;
			}
			matrix = get_joint_poseMatrix_atFrame(animJoint, lastFrame, matrix);
		}*/
	}
	else
	{
		// if current animJoint contains the frame
		//if(std::find(kFrameTime.begin(), kFrameTime.end(), frame) != kFrameTime.end())
		//{
			matrix = get_joint_poseMatrix_atFrame(animJoint, frame, matrix);
		//}
		/*else // else take last frame pos, rot, scale data into poseMatrix
		{
			int lastFrame = frame;
			while(lastFrame >= 0 && std::find(kFrameTime.begin(), kFrameTime.end(), lastFrame) == kFrameTime.end())
			{
				lastFrame--;
			}
			matrix = get_joint_poseMatrix_atFrame(animJoint, lastFrame, matrix);
		}*/
		for(int p = 0; p < parent_count; p++)
		{
			currentParent = parentAnimJointList.at(p);
			int posKeysParentCount = currentParent->mNumPositionKeys;
			std::vector<double> keyFrameTimeParent;
			for(int i = 0; i < posKeysParentCount; i++)
			{
				keyFrameTimeParent.push_back(currentParent->mPositionKeys[i].mTime);
			}

			// if current parent animJoint contains the frame
			//if(std::find(keyFrameTimeParent.begin(), keyFrameTimeParent.end(), frame) != keyFrameTimeParent.end())
			//{
				matrix = get_joint_poseMatrix_atFrame(currentParent, frame, matrix);
			//}
			/*else // else take last frame pos, rot, scale data into poseMatrix
			{
				int lastFrame = frame;
				while(lastFrame >= 0 && std::find(keyFrameTimeParent.begin(), keyFrameTimeParent.end(), lastFrame) == keyFrameTimeParent.end())
				{
					lastFrame--;
				}
				matrix = get_joint_poseMatrix_atFrame(currentParent, lastFrame, matrix);
			}*/
		}
		
	}

	return matrix;
}

glm::mat4 Object::get_joint_poseMatrix_atFrame(aiNodeAnim* joint, int frame, glm::mat4 matrix)
{
	// translation matrix
	aiVector3D posData = joint->mPositionKeys[frame].mValue;
	glm::vec3 posKey(posData.x, posData.y, posData.z);
	glm::mat4 translate(1.0f);
	translate = glm::translate(translate, posKey);

	// rotation matrix
	aiQuaternion rotData = joint->mRotationKeys[frame].mValue;
	aiMatrix3x3 assimpRotMatrix = rotData.GetMatrix();
	glm::mat3 tmpRot = assimpMat3_to_glmMat3(assimpRotMatrix);
	glm::mat4 rotation(tmpRot);
	
	// scale matrix
	aiVector3D scaleData = joint->mScalingKeys[frame].mValue;
	glm::vec3 scaleKey(scaleData.x, scaleData.y, scaleData.z);
	glm::mat4 scale(1.0f);
	scale = glm::scale(scale, scaleKey);

	// final step
	matrix = translate * rotation * scale * matrix;
	return matrix;
}

void Object::reset_drawable()
{
    for(int i = 0; i < mesh_collection.size(); i++)
    {
        mesh_collection.at(i)->reset_drawable();
    }
}
