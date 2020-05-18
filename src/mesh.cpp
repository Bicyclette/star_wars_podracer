/**
 * \file
 * Point by point
 * \author Mathias Velo
 */

#include "mesh.hpp"
#include <glm/gtx/string_cast.hpp>

Mesh::Mesh(std::vector<Vertex> vertices_list, std::vector<int> indices_list, Material m, std::string p_name, bool p_drawable, bool p_dynamic_draw, bool p_lap) :
	vertices(vertices_list),
	indices(indices_list),
	material(m),
	name(p_name),
    drawable(p_drawable),
    dynamic_draw(p_dynamic_draw),
    lap(p_lap)
{
	// VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if(!dynamic_draw)
	    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    else
	    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoords)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bonesID)));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bonesWeight)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	// EBO
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);
}

void Mesh::recreate(std::vector<Vertex> vertices_list, std::vector<int> indices_list)
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    // VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if(!dynamic_draw)
        glBufferData(GL_ARRAY_BUFFER, vertices_list.size() * sizeof(Vertex) * 20, vertices_list.data(), GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, vertices_list.size() * sizeof(Vertex) * 20, vertices_list.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoords)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bonesID)));
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bonesWeight)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    vertices.clear();
    vertices = vertices_list;
    indices.clear();
    indices = indices_list;

    // EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    
    // Unbind VAO
    glBindVertexArray(0);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

std::string Mesh::get_name()
{
	return name;
}

void Mesh::draw(Shader& s, DRAWING_MODE mode)
{
	// bind VAO
	glBindVertexArray(VAO);

	// use shader and sets its texture maps location
	s.use();
	int diffuse_tex_number = 0;
	int specular_tex_number = 0;
	int texture_count = material.textures.size();

    if(texture_count == 0)
    {
        s.set_int("material.has_textures", 0);
        s.set_vec3f("material.base_color", material.base_color);
        s.set_float("material.shininess", material.shininess);
    }
    else
    {
        s.set_int("material.has_textures", 1);
        s.set_vec3f("material.base_color", material.base_color);
        s.set_float("material.shininess", material.shininess);
	    for(int i = 0; i < texture_count; i++)
	    {
		    glActiveTexture(GL_TEXTURE0 + i);
		    if(material.textures.at(i).type == DIFFUSE_TEXTURE)
		    {
			    diffuse_tex_number++;
			    s.set_int("material.diffuse_" + diffuse_tex_number, i);
		    }
		    if(material.textures.at(i).type == SPECULAR_TEXTURE)
		    {
			    specular_tex_number++;
			    s.set_int("material.specular_" + specular_tex_number, i);
		    }
		    glBindTexture(GL_TEXTURE_2D, material.textures.at(i).id);
	    }
    }
	glActiveTexture(GL_TEXTURE0);

	// declare that no animation is playing to the vertex shader
	s.set_int("animation", 0);

	// final step
	if(mode == SOLID)
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	else if(mode == WIREFRAME)
		glDrawArrays(GL_LINES, 0, vertices.size());
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::draw(Shader& s, std::map<std::string, Joint*> bones_ptr_list, Animation* anim, int frame, Joint* skeleton)
{
	// bind VAO
	glBindVertexArray(VAO);

	// use shader and sets its texture maps location
	s.use();
	int diffuse_tex_number = 0;
	int specular_tex_number = 0;
	int texture_count = material.textures.size();
    
    if(texture_count == 0)
    {
        s.set_int("material.has_textures", 0);
        s.set_vec3f("material.base_color", material.base_color);
        s.set_float("material.shininess", material.shininess);
    }
    else
    {
        s.set_int("material.has_textures", 1);
        s.set_vec3f("material.base_color", material.base_color);
        s.set_float("material.shininess", material.shininess);
	    for(int i = 0; i < texture_count; i++)
	    {
		    glActiveTexture(GL_TEXTURE0 + i);
		    if(material.textures.at(i).type == DIFFUSE_TEXTURE)
		    {
			    diffuse_tex_number++;
			    s.set_int("material.diffuse_" + diffuse_tex_number, i);
		    }
		    if(material.textures.at(i).type == SPECULAR_TEXTURE)
		    {
			    specular_tex_number++;
			    s.set_int("material.specular_" + specular_tex_number, i);
		    }
		    glBindTexture(GL_TEXTURE_2D, material.textures.at(i).id);
	    }
    }
	glActiveTexture(GL_TEXTURE0);

	// if anim not null => means we play an animation
	// set the joints orientation based on an animation keyframe
	if(anim != nullptr)
	{
		std::vector<Keyframe*>& keys = anim->get_keyframes();
		int kFrame_count = keys.size();
		int k = frame;
		while(k < kFrame_count && keys.at(k)->get_frame() != k)
		{
			k++;
		}
		Keyframe* kFrame = keys.at(k);
		std::vector<JointPose*>& jPoseList = kFrame->get_joint_pose_list();
		int nbJointPose = jPoseList.size();

		glm::mat4 pose[35];
		glm::mat4 tr[35];
		for(int i = 0; i < 35; i++)
		{
			pose[i] = glm::mat4(1.0f);
			tr[i] = glm::mat4(1.0f);
		}

		for(int b = 0; b < nbJointPose; b++)
		{
			std::string jPoseName = jPoseList.at(b)->name;
			Joint* currentJoint = bones_ptr_list[jPoseName];

			int jointID = currentJoint->get_id();
			glm::mat4 poseMatrix = jPoseList.at(b)->poseMatrix;
			glm::mat4 transform = currentJoint->get_transform();

			pose[jointID - 1] = poseMatrix;
			tr[jointID - 1] = transform;
		}

		// declare that an animation is playing to the vertex shader
		s.set_int("animation", 1);

		// and send joints pose to an array in the vertex shader
		for(int m = 0; m < 35; m++)
		{
			std::string arg1("transform[");
			arg1 += std::to_string(m);
			arg1 += "]";

			std::string arg2("bones_pose[");
			arg2 += std::to_string(m);
			arg2 += "]";

			s.set_Matrix(arg1, tr[m]);
			s.set_Matrix(arg2, pose[m]);
		}
		glm::mat4 inv_tr = skeleton->get_inverse_transform();
		s.set_Matrix("inv_transform", inv_tr);
	}
	else
	{
		// declare that no animation is playing to the vertex shader
		s.set_int("animation", 0);
	}

	// final step
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

std::vector<Vertex> const& Mesh::get_vertex_list() const
{
	return vertices;
}

std::vector<int> const& Mesh::get_index_list() const
{
	return indices;
}

bool Mesh::is_drawable()
{
    return drawable;
}

void Mesh::reset_drawable()
{
    drawable = false;
}

bool Mesh::is_lap_building() const
{
    return lap;
}

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

DrawMaster::DrawMaster()
{
    root = nullptr;
}

DrawMaster::~DrawMaster()
{
    destroy(root);
}

void DrawMaster::destroy(struct QuadTree* node)
{
    if(node != nullptr)
    {
        if(node->bottom_left != nullptr)
        {
            destroy(node->bottom_left);
        }
        if(node->bottom_right != nullptr)
        {
            destroy(node->bottom_right);
        }
        if(node->top_right != nullptr)
        {
            destroy(node->top_right);
        }
        if(node->top_left != nullptr)
        {
            destroy(node->top_left);
        }
        node->drawable_meshes.clear();
        delete(node);
    }
}

void DrawMaster::build_tree(std::vector<Mesh*> m)
{
    std::vector<AABB> env_AABB;
    AABB top_level;
    top_level.tri_mesh = nullptr;
    top_level.name = "top_level";
    top_level.min_x = 0.0f;
    top_level.max_x = 0.0f;
    top_level.min_z = 0.0f;
    top_level.max_z = 0.0f;
    top_level.min_y = 0.0f;
    top_level.max_y = 0.0f;

    bool first_loop = true;

    // process AABB for each mesh
    for(int i = 0; i < m.size(); i++)
    {
        // get vertices
        std::vector<Vertex> const & vertices = m[i]->get_vertex_list();
        
        // get indices
	    std::vector<int> const & indices = m[i]->get_index_list();

        // create new AABB for the current mesh
        AABB meshAABB;
        meshAABB.min_x = 0.0f;
        meshAABB.max_x = 0.0f;
        meshAABB.min_y = 0.0f;
        meshAABB.max_y = 0.0f;
        meshAABB.min_z = 0.0f;
        meshAABB.max_z = 0.0f;
        meshAABB.tri_mesh = nullptr;
        meshAABB.name = m[i]->get_name();

        for(int j = 0; j < indices.size(); j = j + 3)
        {
			if((j + 1) >= indices.size() || (j + 2) >= indices.size()) { break; }
			Vertex vertex1 = vertices[indices[j]];
			Vertex vertex2 = vertices[indices[j + 1]];
			Vertex vertex3 = vertices[indices[j + 2]];

			glm::vec4 v1(vertex1.position.x, vertex1.position.y, vertex1.position.z, 1.0f);
			glm::vec4 v2(vertex2.position.x, vertex2.position.y, vertex2.position.z, 1.0f);
			glm::vec4 v3(vertex3.position.x, vertex3.position.y, vertex3.position.z, 1.0f);
                
            float x_max = std::max(std::max(v1.x, v2.x), v3.x);
            float x_min = std::min(std::min(v1.x, v2.x), v3.x);
            float y_max = std::max(std::max(v1.y, v2.y), v3.y);
            float y_min = std::min(std::min(v1.y, v2.y), v3.y);
            float z_max = std::max(std::max(v1.z, v2.z), v3.z);
            float z_min = std::min(std::min(v1.z, v2.z), v3.z);

            if(first_loop)
            {
                first_loop = false;
                meshAABB.min_x = x_min;
                meshAABB.max_x = x_max;
                meshAABB.min_y = y_min;
                meshAABB.max_y = y_max;
                meshAABB.min_z = z_min;
                meshAABB.max_z = z_max;
            }
            else
            {
                if(meshAABB.min_x > x_min)
                    meshAABB.min_x = x_min;
                if(meshAABB.max_x < x_max)
                    meshAABB.max_x = x_max;
                if(meshAABB.min_y > y_min)
                    meshAABB.min_y = y_min;
                if(meshAABB.max_y < y_max)
                    meshAABB.max_y = y_max;
                if(meshAABB.min_z > z_min)
                    meshAABB.min_z = z_min;
                if(meshAABB.max_z < z_max)
                    meshAABB.max_z = z_max;
            }
        }

        // reset first loop
        first_loop = true;

        // add AABB to the env AABB array
        env_AABB.push_back(meshAABB);
    }

    // create top_level AABB
    process_top_level_AABB(top_level, env_AABB);

    // build tree (5 levels)
    root = new QuadTree(top_level.min_x, top_level.max_x, top_level.min_z, top_level.max_z);

    process_QuadTree_subLevels(root, 5);
    fill_tree(root, env_AABB, m);
    //print_tree(root);
}

void DrawMaster::merge_AABB(const AABB & a, const AABB & b, AABB & res)
{
    if(a.min_x > b.min_x && res.min_x > b.min_x)
        res.min_x = b.min_x;
    else if(res.min_x > a.min_x)
        res.min_x = a.min_x;

    if(a.max_x < b.max_x && res.max_x < b.max_x)
        res.max_x = b.max_x;
    else if(res.max_x < a.max_x)
        res.max_x = a.max_x;

    if(a.min_z > b.min_z && res.min_z > b.min_z)
        res.min_z = b.min_z;
    else if(res.min_z > a.min_z)
        res.min_z = a.min_z;

    if(a.max_z < b.max_z && res.max_z < b.max_z)
        res.max_z = b.max_z;
    else if(res.max_z < a.max_z)
        res.max_z = a.max_z;

    if(a.min_y > b.min_y && res.min_y > b.min_y)
        res.min_y = b.min_y;
    else if(res.min_y > a.min_y)
        res.min_y = a.min_y;

    if(a.max_y < b.max_y && res.max_y < b.max_y)
        res.max_y = b.max_y;
    else if(res.max_y < a.max_y)
        res.max_y = a.max_y;
}

void DrawMaster::process_top_level_AABB(AABB & top_level, std::vector<AABB> list)
{
    if(list.size() == 1)
    {
        if(top_level.min_x > list[0].min_x)
            top_level.min_x = list[0].min_x;
        if(top_level.max_x < list[0].max_x)
            top_level.max_x = list[0].max_x;
        if(top_level.min_z > list[0].min_z)
            top_level.min_z = list[0].min_z;
        if(top_level.max_z < list[0].max_z)
            top_level.max_z = list[0].max_z;
        if(top_level.min_y > list[0].min_y)
            top_level.min_y = list[0].min_y;
        if(top_level.max_y < list[0].max_y)
            top_level.max_y < list[0].max_y;
    }
    else if(list.size() != 0)
    {
        while(list.size() > 0 && list.size() >= 2)
        {
            merge_AABB(list[0], list[1], top_level);
            list.erase(list.begin(), list.begin() + 1);
        }

        if(list.size() == 1)
        {
            if(top_level.min_x > list[0].min_x)
                top_level.min_x = list[0].min_x;

            if(top_level.max_x < list[0].max_x)
                top_level.max_x = list[0].max_x;

            if(top_level.min_z > list[0].min_z)
                top_level.min_z = list[0].min_z;

            if(top_level.max_z < list[0].max_z)
                top_level.max_z = list[0].max_z;

            if(top_level.min_y > list[0].min_y)
                top_level.min_y = list[0].min_y;

            if(top_level.max_y < list[0].max_y)
                top_level.max_y = list[0].max_y;
        }
    }
    /*
    std::cout << "world top_level boundaries:" << std::endl;
    std::cout << "min_x : " << top_level.min_x << std::endl;
    std::cout << "max_x : " << top_level.max_x << std::endl;
    std::cout << "min_z : " << top_level.min_z << std::endl;
    std::cout << "max_z : " << top_level.max_z << std::endl;
    std::cout << "min_y : " << top_level.min_y << std::endl;
    std::cout << "max_y : " << top_level.max_y << std::endl;
    */
}

void DrawMaster::process_QuadTree_subLevels(struct QuadTree * node, int lvl)
{
    if(lvl > 0)
    {
        float mid_x = (node->min_x + node->max_x) / 2.0f;
        float mid_z = (node->min_z + node->max_z) / 2.0f;

        node->bottom_left = new QuadTree(node->min_x, mid_x, node->min_z, mid_z);
        node->bottom_right = new QuadTree(mid_x, node->max_x, node->min_z, mid_z);
        node->top_right = new QuadTree(mid_x, node->max_x, mid_z, node->max_z);
        node->top_left = new QuadTree(node->min_x, mid_x, mid_z, node->max_z);

        //std::cout << "---------- lvl = " << lvl << std::endl;
        //std::cout << "min_x = " << node->min_x << " | max_x = " << node->max_x << std::endl;
        //std::cout << "min_z = " << node->min_z << " | max_z = " << node->max_z << std::endl;

        process_QuadTree_subLevels(node->bottom_left, lvl - 1);
        process_QuadTree_subLevels(node->bottom_right, lvl - 1);
        process_QuadTree_subLevels(node->top_right, lvl - 1);
        process_QuadTree_subLevels(node->top_left, lvl - 1);
    }
}

void DrawMaster::fill_tree(struct QuadTree * node, const std::vector<AABB> & AABB_list, std::vector<Mesh*> m)
{
   if(node->bottom_left == nullptr && node->bottom_right == nullptr && node->top_right == nullptr && node->top_left == nullptr)
   {
        int AABB_count = AABB_list.size();
        for(int i = 0; i < AABB_count; i++)
        {
            bool mesh_node_overlap = overlap(AABB_list[i], node);
            if(mesh_node_overlap)
            {
                for(int j = 0; j < m.size(); j++)
                {
                    if(std::strcmp(m.at(j)->get_name().c_str(), AABB_list[i].name.c_str()) == 0)
                    {
                        if(std::find(node->drawable_meshes.begin(), node->drawable_meshes.end(), m.at(j)) == node->drawable_meshes.end())
                            node->drawable_meshes.push_back(m.at(j));
                    }
                }
            }
        }
   }
   else
   {
       fill_tree(node->bottom_left, AABB_list, m);
       fill_tree(node->bottom_right, AABB_list, m);
       fill_tree(node->top_right, AABB_list, m);
       fill_tree(node->top_left, AABB_list, m);
   }
}

bool DrawMaster::overlap(const AABB & mesh_AABB, struct QuadTree * node)
{
    if((mesh_AABB.min_x >= node->min_x && mesh_AABB.min_x <= node->max_x) || (mesh_AABB.max_x >= node->min_x && mesh_AABB.max_x <= node->max_x) || (mesh_AABB.min_x <= node->min_x && mesh_AABB.max_x >= node->max_x))
    {
        if((mesh_AABB.min_z >= node->min_z && mesh_AABB.min_z <= node->max_z) || (mesh_AABB.max_z >= node->min_z && mesh_AABB.max_z <= node->max_z) || (mesh_AABB.min_z <= node->min_z && mesh_AABB.max_z >= node->max_z))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void DrawMaster::print_tree(struct QuadTree * node)
{
    if(node != nullptr)
    {
        if(node->drawable_meshes.size() != 0)
        {
            std::cout << "--------------------" << std::endl;
            std::cout << "node min_x = " << node->min_x << " max_x = " << node->max_x << std::endl;
            std::cout << "node min_z = " << node->min_z << " max_z = " << node->max_z << std::endl;
            
            for(int i = 0; i < node->drawable_meshes.size(); i++)
            {
                std::cout << "mesh => " << node->drawable_meshes[i]->get_name() << std::endl;
            }
            
        }
        else
        {
            print_tree(node->bottom_left);
            print_tree(node->bottom_right);
            print_tree(node->top_right);
            print_tree(node->top_left);
        }
    }
}

void DrawMaster::process_drawable_meshes_list(glm::vec3 cam_pos, glm::vec3 cam_view_dir, glm::vec3 cam_right, glm::vec3 cam_up)
{
    // compute frustum bounding box
    float cam_near = -225.0f;
    float cam_far = 4500.0f;
    float cam_fov = (60.0f * PI)/180.0f;

    glm::vec3 up = ((sin(cam_fov) * cam_far) * cam_up) * (780.0f / 1560.0f);
    glm::vec3 right = ((sin(cam_fov) * cam_far) * cam_right);
    
    glm::vec3 frustum_far_top_right = (cam_pos + (cam_far * cam_view_dir)) + ((sin(cam_fov) * cam_far) * cam_up) + ((sin(cam_fov) * cam_far) * cam_right);
    glm::vec3 frustum_far_bottom_right = (cam_pos + (cam_far * cam_view_dir)) + ((sin(cam_fov) * cam_far) * -cam_up) + ((sin(cam_fov) * cam_far) * cam_right);
    glm::vec3 frustum_far_bottom_left = (cam_pos + (cam_far * cam_view_dir)) + ((sin(cam_fov) * cam_far) * -cam_up) + ((sin(cam_fov) * cam_far) * -cam_right);
    glm::vec3 frustum_far_top_left = (cam_pos + (cam_far * cam_view_dir)) + ((sin(cam_fov) * cam_far) * cam_up) + ((sin(cam_fov) * cam_far) * -cam_right);
    
    glm::vec3 frustum_near_top_right = (cam_pos + (cam_near * cam_view_dir)) + ((sin(cam_fov) * cam_near) * cam_up) + ((sin(cam_fov) * cam_near) * cam_right);
    glm::vec3 frustum_near_bottom_right = (cam_pos + (cam_near * cam_view_dir)) + ((sin(cam_fov) * cam_near) * -cam_up) + ((sin(cam_fov) * cam_near) * cam_right);
    glm::vec3 frustum_near_bottom_left = (cam_pos + (cam_near * cam_view_dir)) + ((sin(cam_fov) * cam_near) * -cam_up) + ((sin(cam_fov) * cam_near) * -cam_right);
    glm::vec3 frustum_near_top_left = (cam_pos + (cam_near * cam_view_dir)) + ((sin(cam_fov) * cam_near) * cam_up) + ((sin(cam_fov) * cam_near) * -cam_right);

    float frustum_far_min_x = std::min(frustum_far_bottom_left.x, std::min(frustum_far_bottom_right.x, std::min(frustum_far_top_right.x, frustum_far_top_left.x)));
    float frustum_far_min_z = std::min(frustum_far_bottom_left.z, std::min(frustum_far_bottom_right.z, std::min(frustum_far_top_right.z, frustum_far_top_left.z)));
    float frustum_far_max_x = std::max(frustum_far_bottom_left.x, std::max(frustum_far_bottom_right.x, std::max(frustum_far_top_right.x, frustum_far_top_left.x)));
    float frustum_far_max_z = std::max(frustum_far_bottom_left.z, std::max(frustum_far_bottom_right.z, std::max(frustum_far_top_right.z, frustum_far_top_left.z)));
    
    float frustum_near_min_x = std::min(frustum_near_bottom_left.x, std::min(frustum_near_bottom_right.x, std::min(frustum_near_top_right.x, frustum_near_top_left.x)));
    float frustum_near_min_z = std::min(frustum_near_bottom_left.z, std::min(frustum_near_bottom_right.z, std::min(frustum_near_top_right.z, frustum_near_top_left.z)));
    float frustum_near_max_x = std::max(frustum_near_bottom_left.x, std::max(frustum_near_bottom_right.x, std::max(frustum_near_top_right.x, frustum_near_top_left.x)));
    float frustum_near_max_z = std::max(frustum_near_bottom_left.z, std::max(frustum_near_bottom_right.z, std::max(frustum_near_top_right.z, frustum_near_top_left.z)));

    float frustum_min_x = std::min(frustum_far_min_x, frustum_near_min_x);
    float frustum_min_z = std::min(frustum_far_min_z, frustum_near_min_z);
    float frustum_max_x = std::max(frustum_far_max_x, frustum_near_max_x);
    float frustum_max_z = std::max(frustum_far_max_z, frustum_near_max_z);
/*
    std::cout << "cam pos = (" << cam_pos.x << ", " << cam_pos.y << ", " << cam_pos.z << ")" << std::endl;
    std::cout << "far top right = (" << frustum_far_top_right.x << ", " << frustum_near_top_right.y << ", " << frustum_far_top_right.z << ")" << std::endl;
    std::cout << "far top left = (" << frustum_far_top_left.x << ", " << frustum_near_top_left.y << ", " << frustum_far_top_left.z << ")" << std::endl;
    std::cout << "far bottom right = (" << frustum_far_bottom_right.x << ", " << frustum_near_bottom_right.y << ", " << frustum_far_bottom_right.z << ")" << std::endl;
    std::cout << "far bottom left = (" << frustum_far_bottom_left.x << ", " << frustum_near_bottom_left.y << ", " << frustum_far_bottom_left.z << ")" << std::endl;

    std::cout << "near top right = (" << frustum_near_top_right.x << ", " << frustum_near_top_right.y << ", " << frustum_near_top_right.z << ")" << std::endl;
    std::cout << "near top left = (" << frustum_near_top_left.x << ", " << frustum_near_top_left.y << ", " << frustum_near_top_left.z << ")" << std::endl;
    std::cout << "near bottom right = (" << frustum_near_bottom_right.x << ", " << frustum_near_bottom_right.y << ", " << frustum_near_bottom_right.z << ")" << std::endl;
    std::cout << "near bottom left = (" << frustum_near_bottom_left.x << ", " << frustum_near_bottom_left.y << ", " << frustum_near_bottom_left.z << ")" << std::endl;

    std::cout << "FRUSTUM BOUNDING BOX = min_x => " << frustum_min_x << ", max_x => " << frustum_max_x << ", min_z => " << frustum_min_z << ", max_z => " << frustum_max_z << std::endl;
*/
    struct AABB frustum_AABB;
    frustum_AABB.tri_mesh = nullptr;
    frustum_AABB.name = "frustum AABB";
    frustum_AABB.min_x = frustum_min_x;
    frustum_AABB.max_x = frustum_max_x;
    frustum_AABB.min_z = frustum_min_z;
    frustum_AABB.max_z = frustum_max_z;
    frustum_AABB.min_y = 0.0f;
    frustum_AABB.max_y = 0.0f;

    // update drawable status for env meshes
    update_drawable(frustum_AABB, root);
}

void DrawMaster::update_drawable(const struct AABB & f_AABB, struct QuadTree * node)
{
    if(node->bottom_left == nullptr && node->bottom_right == nullptr && node->top_right == nullptr && node->top_left == nullptr)
    {
        for(int i = 0; i < node->drawable_meshes.size(); i++)
        {
            if(!node->drawable_meshes.at(i)->is_lap_building())
                node->drawable_meshes.at(i)->drawable = true;
        }
    }
    else
    {
        if(overlap(f_AABB, node->bottom_left))
        {
            update_drawable(f_AABB, node->bottom_left);
        }
        if(overlap(f_AABB, node->bottom_right))
        {
            update_drawable(f_AABB, node->bottom_right);
        }
        if(overlap(f_AABB, node->top_right))
        {
            update_drawable(f_AABB, node->top_right);
        }
        if(overlap(f_AABB, node->top_left))
        {
            update_drawable(f_AABB, node->top_left);
        }
    }
}

void Mesh::update_VBO(std::vector<Vertex> const & updated_vertices)
{
    indices.clear();
    for(int i = 0; i < updated_vertices.size(); i++)
    {
        indices.push_back(i);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    void * VBO_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    void * EBO_ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(VBO_ptr, updated_vertices.data(), updated_vertices.size() * sizeof(Vertex));
    memcpy(EBO_ptr, indices.data(), indices.size() * sizeof(int));

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    vertices.clear();
    vertices = updated_vertices;
}
