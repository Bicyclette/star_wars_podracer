#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <string>
#include <omp.h>
#include <math.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include "shader.hpp"
#include "color.hpp"
#include "object.hpp"
#include "skybox.hpp"
#include "smoke.hpp"
#include "power.hpp"
#include "audio.hpp"

#define WIDTH 1560
#define HEIGHT 780
#define MATRIX_SCALAR_COUNT 16
#define MINIMAP_DIMENSIONS 512

class Camera;
class Podracer;
class Environment;
class Minimap;
class WorldPhysics;
class DrawMaster;

enum PAGE
{
	MAIN,
	INFOS,
	MAP,
	CONTROLS,
	POD_SPECS,
	TUNING,
    END_GAME_STATS,
	RACE
};

struct UserActions
{
	bool key_up;
	bool key_down;
	bool key_right;
	bool key_left;
	bool key_c;
	bool key_v;
	bool key_space;
	bool key_shift;

	bool mouse_scroll;
	int scroll_direction;
	bool mouse_middle;
	bool mouse_left;
	bool mouse_left_down;
	bool slide_volume;
	float xRel;
	float yRel;
	int mouseX;
	int mouseY;

	bool clicked_play;
	bool clicked_infos;
	bool clicked_pod_specs;
    bool clicked_main_menu;
	bool clicked_quit;

	bool clicked_map;
	bool clicked_controls;

	bool clicked_back;
};

struct BoundingBox
{
	int top_left_x;
	int top_left_y;
	int bottom_right_x;
	int bottom_right_y;
};

struct HUD_chrono
{
	GLuint min_d0;
	GLuint min_d1;
	GLuint dots2;
	GLuint sec_d0;
	GLuint sec_d1;
	GLuint dot;
	GLuint ms_d0;
	GLuint ms_d1;
};

struct HUD_pos
{
	GLuint pos;
	GLuint slash;
	GLuint max;
};

struct HUD_lap
{
	GLuint current;
	GLuint slash;
	GLuint max;
};

struct HUD_speed
{
	GLuint speed_bar;
	GLuint speed_bar_VBO;
	GLuint layout;
	GLuint d0;
	GLuint d1;
	GLuint d2;
};

class Game
{
	public:

		Game(const std::string& title);
		~Game();
		void start(); // shows main menu
        static void loading_screen();
		void quit();

	private:

		SDL_Window* createWindow(int w, int h, const std::string& title);
		void set_menu_textures();
		void set_framebuffers();
        void update_framebuffers();
		void tuning();
		void gameInfo(); // go to the rules/game presentation page
		void map();
		void podracer_controls();
		void pod_specs();
        void end_game_stats();
		
		void play(); // starts the actual racing game
		void prepare_print_quit_game(GLuint VAO, GLuint VAO1, Shader* grey_shader);
		void quit_game(HUD_speed& hud_speed, GLuint topBarVAO, HUD_lap& hud_lap, HUD_pos& hud_pos, HUD_chrono& chrono);
		void set_view_matrix(bool & first_loop);
		void render_env_texture();
		void render_smoke_texture();
		void render_depth_texture(bool podracerDepth = false);
		void render_env_motion_blur_texture(GLuint VAO, Shader& motionBlur_shader);
		void render_smoke_motion_blur_texture(GLuint VAO, Shader& motionBlur_shader);
		void render_podracer();
		void render_gaussian_blur_bright_colors(GLuint VAO, Shader& gaussianBlurShader);
		void render_color(GLuint VAO, Shader& color_shader);
		void render_HUD(Shader& final_shader, GLuint VAO, HUD_speed& hud_speed, GLuint topBarVAO, HUD_lap& hud_lap, HUD_pos& hud_pos, HUD_chrono& chrono, bool disable_HUD = false);
		void view_render_pass(GLuint VAO, Shader& color_shader);
		void process_countdown(GLuint countdownVAO, float delta, Shader& countdown_shader);
		
		void draw_speed_HUD(HUD_speed& hud_speed);
		void draw_topBar_HUD(GLuint topBarVAO, HUD_lap& hud_lap, HUD_pos& hud_pos, HUD_chrono& chrono);
		void draw_map_HUD(bool quit_game = false);
        void draw_lap_time(HUD_chrono& lap_time);
        void play_end_game_anim();
		
		void check_events(); // check for user input each render loop
		void update_menu_bb(int win_max_width, int win_max_height);
		void render_to_shadowMap(bool podShadowMap = false);
		void reset();

		void sound_system();
	
		// menu bounding boxes
		struct BoundingBox bb_play;
		struct BoundingBox bb_infos;
		struct BoundingBox bb_pod_specs;
		struct BoundingBox bb_quit;
		struct BoundingBox bb_tuning;
		struct BoundingBox bb_back;
		struct BoundingBox bb_map;
		struct BoundingBox bb_controls;
		struct BoundingBox bb_cast_shadows;
		struct BoundingBox bb_close_tuning_window;
		struct BoundingBox bb_sound_slider;
		struct BoundingBox bb_quit_no;
		struct BoundingBox bb_quit_yes;
		struct BoundingBox bb_main_menu;

		// game timer
		double timer;
        double timer1;
        double timer2;
        double timer3;

		int timer_min_d0;
		int timer_min_d1;
		int timer_sec_d0;
		int timer_sec_d1;
		int timer_ms_d0;
		int timer_ms_d1;

		int lap1_min_d0;
		int lap1_min_d1;
		int lap1_sec_d0;
		int lap1_sec_d1;
		int lap1_ms_d0;
		int lap1_ms_d1;
		
        int lap2_min_d0;
		int lap2_min_d1;
		int lap2_sec_d0;
		int lap2_sec_d1;
		int lap2_ms_d0;
		int lap2_ms_d1;
		
        int lap3_min_d0;
		int lap3_min_d1;
		int lap3_sec_d0;
		int lap3_sec_d1;
		int lap3_ms_d0;
		int lap3_ms_d1;
        
        int total_min_d0;
		int total_min_d1;
		int total_sec_d0;
		int total_sec_d1;
		int total_ms_d0;
		int total_ms_d1;

        // podracer average speed
        int avg_speed;
        unsigned long long int nb_frames;

		// game countdown
		int countdown_timer;

		// game top bar HUD
		HUD_chrono chrono;
		HUD_lap hud_lap;
		HUD_pos hud_pos;

        // game lap_time HUD
        HUD_chrono lap_time;
        bool hit_count_lap_wall;
        int lap_iterate;
        double display_lap_timer;
        float delta_anim;

		// game speed HUD
		HUD_speed hud_speed;

		SDL_Window* window;
		bool show_menu;
		bool show_info_menu;
		bool show_map;
		bool show_controls;
		bool show_pod_specs;
		bool show_tuning;
        bool show_end_game_stats;
		SDL_GLContext glContext;
		SDL_Event event;
		const Uint8* keyboardState; // snapshot of the keyboard state
		Uint32 mouseButtonBitMask;
		int mouseX_rel;
		int mouseY_rel;
		GLenum err;

		// game tuning parameters
		bool cast_shadows;
		int sound_volume;
		bool exit_game;
		bool print_quit_game;
		
		// window width & height
		int width;
        int menu_width;
		int height;
        int menu_height;
        bool menu_fullscreen;

		// menu textures
		std::vector<std::string> tex_path;
		std::vector<bool> flip;
		std::vector<Texture> menu_textures;

		bool in_racing_game;
		UserActions user_actions;
		Camera* editor_cam;
		Camera* racing_cam;
		Camera* pod_specs_cam;
		Camera* minimap_cam;
		Camera* cam;

		Skybox* sky;
		Podracer* pod;
		Environment* platform;
		Environment* env;
		Minimap* minimap;

		// navigation
		PAGE current_page;
		std::vector<float> tuning_button;
		GLuint tuningVAO, tuningVBO, tuningEBO;
		std::vector<float> back_button;
		GLuint backVAO, backVBO, backEBO;
		Shader* ui_shader;

		// render loop attributes
		double lastFrame;
		double currentFrame;
		double delta;
		double accuDelta;
		double animRate;
		double fps;

		//sounds
		Audio* sounds;
		Source* main_menu_source;
		Source* pod_fire_power_coupling;
		Source* pod_power_coupling;
		Source* pod_start_electric_engine;
		Source* pod_electric_engine;
		Source* pod_fire_engine;
		Source* pod_base_engine;
		Source* pod_break;
		Source* pod_afterburn;
		Source* fodesinbeed;
		Source* tatooine_wind;
		Source* countdown_sounds;

		// shadows framebuffer
		GLuint shadowMapFBO;
		int shadowMapWidth;
		int shadowMapHeight;
		GLuint shadowMap;
		
        GLuint shadowMap2FBO;
		GLuint shadowMap2;

        bool process_pod_shadowPass;
		
		// depth framebuffer
		GLuint depthFBO;
		GLuint depthTexture;
		
		// depth bis framebuffer
		GLuint depthBisFBO;
		GLuint depthBisTexture;
		
		// podracer depth framebuffer
		GLuint depthPodFBO;
		GLuint depthPodTexture;

		// env framebuffer
		GLuint envFBO;
		GLuint envRBO;
		GLuint envTexture;
		
		// smoke framebuffer
		GLuint smokeFBO;
		GLuint smokeRBO;
		GLuint smokeTexture;
		
		// env motionBlur framebuffer
		GLuint envMotionBlurFBO;
		GLuint envMotionBlurRBO;
		GLuint envMotionBlurTexture;
		
		// smoke motionBlur framebuffer
		GLuint smokeMotionBlurFBO;
		GLuint smokeMotionBlurRBO;
		GLuint smokeMotionBlurTexture;
		GLuint smokeBrightMotionBlurTexture;
		
		// motionBlur framebuffer
		GLuint motionBlurFBO;
		GLuint motionBlurTexture;
		
		// podracer framebuffer
		GLuint podracerFBO;
		GLuint podRBO;
		GLuint podracerTexture;
		GLuint podracerBrightTexture;

		// pingpong framebuffers
		GLuint pingFBO;
		GLuint pongFBO;
		GLuint pingRBO;
		GLuint pongRBO;
		GLuint pingTexture;
		GLuint pongTexture;
		
		GLuint ping2FBO;
		GLuint pong2FBO;
		GLuint ping2RBO;
		GLuint pong2RBO;
		GLuint ping2Texture;
		GLuint pong2Texture;
		
		// final color framebuffer
		GLuint colorFBO;
		GLuint colorRBO;
		GLuint colorTexture;

		// render pass visualization
		bool check_render_pass;
		int render_pass;

		// previous camera view matrix
		glm::mat4 prev_camera_view;

        // World physics
        WorldPhysics* tatooine;

        // Draw master
        DrawMaster* draw_master;
		
		friend class Camera;
		friend class Skybox;
		friend class Podracer;
		friend class Smoke;
		friend class Environment;
		friend class Minimap;
        friend class WorldPhysics;
};

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

class Camera
{
	public:

		static const int EDITOR;
		static const int PODRACER_PILOT;
		static const int POD_SPECS;
		static const int MINIMAP;
		static const float ROLL_ANGLE_RATE;
		static const float TURN_ANGLE_RATE;
		static const float KMH_TO_MS;

		Camera(int p_type, int screen_w, int screen_h, float p_fov);
		int get_type() const;
		glm::vec3 get_position() const;
		glm::vec3 get_vector_up() const;
		glm::vec3 get_vector_right() const;
		glm::vec3 get_center_ray() const;
		glm::vec3 get_direction() const;
		glm::mat4 get_view() const;
		glm::mat4 get_projection() const;
		glm::mat4 get_model() const;
		float get_yaw() const;
		void update_view(Game* g, float delta);
        void update_view(glm::mat4 & transform, glm::vec3 pod_direction, Game* g);
        void turn_view(float p_yaw, float p_pitch);
		void reset();
        void update_roll(float v, Game* g, bool reset = false);

	private:

		void update_yaw_and_pitch(float xDif, float yDif, float delta);

		int type;
		float fov;
		glm::vec3 position;
		float yaw;
		float pitch;
		float roll;
		float move_sensitivity;
		glm::vec3 target;
		float target_eye_length;
		glm::vec3 direction;
		glm::vec3 up;
		glm::vec3 right;
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 model;
        bool cam_move;
};

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

class Podracer
{
	public:

		/***** METHODS *****/

		Podracer(std::string p_name, Game* p_g);
		~Podracer();
		void draw(bool shadowPass = false, bool depthPass = false, bool smokePass = false);
		inline void draw_aux(bool depthPass = false, bool smokePass = false);
		void reset();

		/***** ATTRIBUTES *****/

		// podracer's engine
		std::string name;
		bool power_coupling_on;
		bool electric_engine_on;
		bool afterburn_on;
		bool turbojet_on;
		float fuel_power;
		float speed;
		float heat;
		float overheat;
		float max_speed;
        float max_roll_angle;
        glm::vec3 rotor_left_translate;
        glm::vec3 rotor_right_translate;
	
		// camera + game ptr
		Camera* cam;
		Game* g;

		// pod objects + objects_shader + smoke + smoke_shader + power + power_shader
		Object* chariot;
        Object* dir_left;
        Object* dir_right;
        Object* cable_left;
        Object* cable_right;
        Object* reactor_left;
        Object* reactor_right;
        Object* rotor_left;
        Object* rotor_right;
        Object* air_scoops_left1;
        Object* air_scoops_left2;
        Object* air_scoops_left3;
        Object* air_scoops_left_hinge1;
        Object* air_scoops_left_hinge2;
        Object* air_scoops_left_hinge3;
        Object* air_scoops_right1;
        Object* air_scoops_right2;
        Object* air_scoops_right3;
        Object* air_scoops_right_hinge1;
        Object* air_scoops_right_hinge2;
        Object* air_scoops_right_hinge3;
        Shader* pod_shader;

		Smoke* smoke_left;
		Smoke* smoke_right;
        Shader* smoke_shader;

		Power* power;
        Shader* power_shader;

        friend class WorldPhysics;
};

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

class Environment
{
	public:

		Environment(std::string p_name, Game* p_g);
		Environment(std::string p_name, std::string file_path, Game* p_g);
		~Environment();
		glm::vec3 get_sun_color() const;
		glm::vec3 get_sun_direction() const;
		glm::mat4 get_sun_spaceMatrix(bool pod) const;
		void draw(bool shadowPass, bool depthPass = false);
		void reset();
        std::vector<Mesh*> get_mesh_collection(bool mos_espa = false, int index = 0);
        void reset_drawable();
        glm::mat4 get_model_env();

	private:

		std::string name;
        std::vector<Object*> env;
        std::vector<Mesh*> collection;
		Shader* env_shader;

		// model
		glm::mat4 model_env;

		// light sources
		glm::vec3 sun_color;
		glm::vec3 sun_direction;
		glm::mat4 sunlightProj;
		glm::mat4 sunlightView;
		glm::mat4 sunlightSpaceMatrix;

		// Game ptr
		Game* g;

		// camera ptr
		Camera* cam;
};

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

class Minimap
{
	public:

		Minimap(const std::string & file_path, Game* p_g);
		~Minimap();
        void update_framebuffer(int w, int h);
		void draw(bool quit_game = false);

	private:

		glm::mat4 update_dot_model();
		
		// red dot position and rotation
		glm::vec3 dot_last_pos;
		glm::vec3 dot_new_pos;
		glm::vec3 dot_translate;

		float dot_last_rotation;
		float dot_current_rotation;
		float dot_rotation;

		glm::mat4 trans_accu;
		glm::mat4 rot_accu;

		glm::mat4 model_dot;

		Game* g;
		Object* minimap;
		Object* red_dot;
		Shader* shader;
		glm::vec3 sun_pos;
		glm::vec3 sun_dir;
		glm::vec3 sun_color;

		GLuint minimapFBO;
		GLuint minimapTexture;
        GLuint RBO;

		GLuint minimapVAO;
		GLuint minimapVBO;
		GLuint minimapEBO;
};

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

enum COLLISION_GROUP
{
    NONE,
    POD,
    ENV,
    LAP_COUNT
};

int cmp_vertex(const void * a, const void * b);

class WorldPhysics
{
    public:

        WorldPhysics(Game* p_g);
        ~WorldPhysics();
        void update_dynamics();
        void reset();
        btRaycastVehicle* get_vehicle();
        glm::vec3 get_pod_direction();
        
        //***** podracer model matrices *****
        glm::mat4 chariot_model;
        glm::mat4 dir_left_model;
        glm::mat4 dir_right_model;
        glm::mat4 reactors_model;
        glm::mat4 rotor_left_model;
        glm::mat4 rotor_right_model;
        glm::mat4 air_scoops_left1_model;
        glm::mat4 air_scoops_left2_model;
        glm::mat4 air_scoops_left3_model;
        glm::mat4 air_scoops_left_hinge1_model;
        glm::mat4 air_scoops_left_hinge2_model;
        glm::mat4 air_scoops_left_hinge3_model;
        glm::mat4 air_scoops_right1_model;
        glm::mat4 air_scoops_right2_model;
        glm::mat4 air_scoops_right3_model;
        glm::mat4 air_scoops_right_hinge1_model;
        glm::mat4 air_scoops_right_hinge2_model;
        glm::mat4 air_scoops_right_hinge3_model;

    private:

        /***** ATTRIBUTES *****/
        Game* g;
        btAlignedObjectArray<btCollisionShape*> collisionShapes;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        btBroadphaseInterface* overlappingPairCache;
        btSequentialImpulseConstraintSolver* solver;
        btSoftRigidDynamicsWorld* dynamicsWorld;
        btSoftBodyWorldInfo * softBody_worldInfo;

        // vehicle data
        float engineForce;
        float engineForceIncrement;
        float maxEngineForce;

        float breakingForce;
        float maxBreakingForce;

        float vehicleSteering;
        float steeringIncrement;
        float steeringClamp;

        btRaycastVehicle::btVehicleTuning tuning;
        btVehicleRaycaster* raycaster;
        btRaycastVehicle* vehicle;

        // control individual parts
        btRigidBody * chariot_body;
        btRigidBody * reactors_body;
        btTransform reactors_initialTransform;
        btTransform chariot_initialTransform;
        btCollisionObject * reactors_colObj;
        btSoftBody * cable_left;
        btSoftBody * cable_right;
        std::vector<Vertex> initial_c_left_vertices;
        std::vector<int> initial_c_left_indices;
        std::vector<Vertex> initial_c_right_vertices;
        std::vector<int> initial_c_right_indices;
        std::vector<Vertex> prev_c_left_vertices;
        std::vector<Vertex> prev_c_right_vertices;
        std::vector<Vertex> updated_c_left_vertices;
        std::vector<Vertex> updated_c_right_vertices;

        /***** internal methods *****/
        btScalar * vertex_list_2_btScalarArray(std::vector<Vertex> const & vertices);
        int get_vertex_defined_index(glm::vec3 ref_pos, int ref_index, const std::vector<std::pair<glm::vec3, int>> & couples);
        int retrieve_correct_index(glm::vec3 ref_pos, const std::vector<std::pair<glm::vec3, int>> & couple);
        std::vector<Vertex> init_previous_vertices(std::vector<Vertex> ref_vertices, std::vector<Vertex> v);
        Vertex get_vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 prev_pos, const std::vector<Vertex> & prev_vertices);
};

#endif
