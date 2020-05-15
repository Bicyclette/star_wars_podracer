#ifndef _MESH_HPP_
#define _MESH_HPP_

#include <GL/glew.h>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <algorithm>
#include "shader.hpp"
#include "animation.hpp"
#include "joint.hpp"

#define PI 3.14159265

enum DRAWING_MODE
{
	SOLID,
	WIREFRAME
};

enum TextureType
{
	DIFFUSE_TEXTURE,
	SPECULAR_TEXTURE
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec2 bonesID;
	glm::vec2 bonesWeight;

	Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 tex, glm::vec2 bID, glm::vec2 bWeight)
	{
		position = pos;
		normal = norm;
		texCoords = tex;
		bonesID = bID;
		bonesWeight = bWeight;
	}
};

struct Texture
{
	GLuint id;
	TextureType type;
	std::string texture_file_path;

	Texture(GLuint tex_id, TextureType t, std::string tex_file_path)
	{
		id = tex_id;
		type = t;
		texture_file_path = tex_file_path;
	}
};

struct Material
{
	std::vector<Texture> textures;
    glm::vec3 base_color;
	float shininess;
};

struct Point
{
    float x;
    float y;
    float z;
};

struct TRIANGLE
{
    Point vertex[3];
};

struct TRIANGLE_MESH
{
    std::vector<TRIANGLE> shape;
};

struct AABB
{
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    float min_z;
    float max_z;
    TRIANGLE_MESH* tri_mesh;
    std::string name;
};

class Mesh
{
	public:

		Mesh(std::vector<Vertex> vertices_list, std::vector<int> indices_list, Material m, std::string p_name, bool p_drawable = false, bool p_dynamic_draw = false, bool p_lap = false);
		void recreate(std::vector<Vertex> vertices_list, std::vector<int> indices_list);
        ~Mesh();
        bool is_drawable();
		std::string get_name();
		void draw(Shader& s, DRAWING_MODE mode = SOLID);
		void draw(Shader& s, std::map<std::string, Joint*> bones_ptr_list, Animation* anim, int frame, Joint* skeleton);
		std::vector<Vertex> const& get_vertex_list() const;
		std::vector<int> const& get_index_list() const;
        void update_VBO(std::vector<Vertex> const & updated_vertices);
        void reset_drawable();
        bool is_lap_building() const;

	private:

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;

		std::vector<Vertex> vertices;
		std::vector<int> indices;
		Material material;
		std::string name;
        bool drawable;
        bool dynamic_draw;
        bool lap;

		friend class Object;
        friend class DrawMaster;
};

struct QuadTree
{
    struct QuadTree* bottom_left;
    struct QuadTree* bottom_right;
    struct QuadTree* top_right;
    struct QuadTree* top_left;
    
    float min_x;
    float max_x;
    float min_z;
    float max_z;
    
    std::vector<Mesh*> drawable_meshes;

    QuadTree(float p_min_x = 0.0f, float p_max_x = 0.0f, float p_min_z = 0.0f, float p_max_z = 0.0f)
    {
        min_x = p_min_x;
        max_x = p_max_x;
        min_z = p_min_z;
        max_z = p_max_z;

        bottom_left = nullptr;
        bottom_right = nullptr;
        top_right = nullptr;
        top_left = nullptr;
    }
};

class DrawMaster
{
    public:
        DrawMaster();
        ~DrawMaster();
        void destroy(struct QuadTree* node);
        void build_tree(std::vector<Mesh*> m);
        void process_drawable_meshes_list(glm::vec3 cam_pos, glm::vec3 cam_view_dir, glm::vec3 cam_right, glm::vec3 cam_up);
        void print_tree(struct QuadTree * node = nullptr);
        std::vector<AABB> get_drawable_meshes_AABB();

    private:
        /* ---------- METHODS ---------- */

        void merge_AABB(const AABB & a, const AABB & b, AABB & res);
        void process_top_level_AABB(AABB & top_level, std::vector<AABB> list);
        void process_QuadTree_subLevels(struct QuadTree * node, int lvl);
        void fill_tree(struct QuadTree * node, const std::vector<AABB> & AABB_list, std::vector<Mesh*> m);
        bool overlap(const AABB & mesh_AABB, struct QuadTree * node);
        void update_drawable(const struct AABB & f_AABB, struct QuadTree * node);

        /* ---------- PROPERTIES ---------- */
        struct QuadTree* root;
};

#endif
