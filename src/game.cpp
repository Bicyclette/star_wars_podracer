/**
 * \file
 * It all happens here
 * \author Mathias Velo
 */

#include "game.hpp"
#include <glm/gtx/string_cast.hpp>
#define CHECK_RENDER_PASS 0

Game::Game(const std::string& title) :
	tuning_button({0.925f, 0.9f, -0.15f, 0.0f, 0.0f,
		1.0f, 0.9f, -0.15f, 1.0f, 0.0f,
		0.925f, 1.0f, -0.15f, 0.0f, 1.0f,
		1.0f, 1.0f, -0.15f, 1.0f, 1.0f}),
	back_button({-1.0f, 0.9f, -0.15f, 0.0f, 0.0f,
		-0.925f, 0.9f, -0.15f, 1.0f, 0.0f,
		-1.0f, 1.0f, -0.15f, 0.0f, 1.0f,
		-0.925f, 1.0f, -0.15f, 1.0f, 1.0f}),
    process_pod_shadowPass(false),
    hit_count_lap_wall(false),
    lap_iterate(-1),
    lap1_min_d0(0),
    lap1_min_d1(0),
    lap1_sec_d0(0),
    lap1_sec_d1(0),
    lap1_ms_d0(0),
    lap1_ms_d1(0),
    lap2_min_d0(0),
    lap2_min_d1(0),
    lap2_sec_d0(0),
    lap2_sec_d1(0),
    lap2_ms_d0(0),
    lap2_ms_d1(0),
    lap3_min_d0(0),
    lap3_min_d1(0),
    lap3_sec_d0(0),
    lap3_sec_d1(0),
    lap3_ms_d0(0),
    lap3_ms_d1(0),
    total_min_d0(0),
    total_min_d1(0),
    total_sec_d0(0),
    total_sec_d1(0),
    total_ms_d0(0),
    total_ms_d1(0),
    avg_speed(0),
    nb_frames(0),
    display_lap_timer(2.0),
    delta_anim(0.0f),
    menu_fullscreen(false)
{
	width = WIDTH;
	height = HEIGHT;
	menu_width = WIDTH;
	menu_height = HEIGHT;
	window = createWindow(WIDTH, HEIGHT, title);
	if(window == nullptr)
	{
		std::cerr << SDL_GetError() << std::endl;
		SDL_Quit();
		std::exit(-1);
	}

	// boolean fiesta
	show_menu = true;
	show_info_menu = false;
	show_map = false;
	show_controls = false;
	show_pod_specs = false;
	show_tuning = false;
	show_end_game_stats = false;

	// game tuning parameters
	cast_shadows = true;
	sound_volume = 45; // [0,100]
	exit_game = false;
	print_quit_game = false;

	// OPENGL CONTEXT FOR THE WINDOW
	glContext = SDL_GL_CreateContext(window);

	// GLEW INIT
	glewExperimental = true;
	err = glewInit();
	if(err != GLEW_OK)
	{
		std::cerr << glewGetErrorString(err) << std::endl;
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(window);
		SDL_Quit();
		std::exit(-1);
	}
	
	in_racing_game = true;

	// EVERYTHING'S FINE : TIME TO SET SOME OPENGL STATES
	glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);
	glClearDepthf(2000.0f);
	SDL_GL_SetSwapInterval(1);
	
	// INIT CAMERAS
	editor_cam = new Camera(Camera::EDITOR, WIDTH, HEIGHT, 60.0f);
	racing_cam = new Camera(Camera::PODRACER_PILOT, WIDTH, HEIGHT, 60.0f);
	pod_specs_cam = new Camera(Camera::POD_SPECS, WIDTH, HEIGHT, 60.0f);
	minimap_cam = new Camera(Camera::MINIMAP, WIDTH, HEIGHT, 60.0f);
	cam = racing_cam;
	prev_camera_view = cam->get_view();
	
	// podracer
	pod = nullptr;

	// Create Skybox
	sky = nullptr;
	std::vector<std::string> cubemap_textures = {"../assets/textures/skybox/back.tga",
		"../assets/textures/skybox/front.tga",
		"../assets/textures/skybox/top.tga",
		"../assets/textures/skybox/top.tga",
		"../assets/textures/skybox/left.tga",
		"../assets/textures/skybox/right.tga"};
	sky = new Skybox("../shaders/env/skybox/vertex.glsl", "../shaders/env/skybox/fragment.glsl", "../shaders/env/skybox/geometry.glsl", cubemap_textures);
	
	// USER ACTIONS
	user_actions.key_up = false;
	user_actions.key_down = false;
	user_actions.key_right = false;
	user_actions.key_left = false;
	user_actions.key_c = false;
	user_actions.key_v = false;
	user_actions.key_space = false;
	user_actions.key_shift = false;
	user_actions.mouse_scroll = false;
	user_actions.scroll_direction = 0;
	user_actions.mouse_middle = false;
	user_actions.mouse_left = false;
	user_actions.mouse_left_down = false;
	user_actions.slide_volume = false;
	user_actions.xRel = 0.0f;
	user_actions.yRel = 0.0f;
	user_actions.mouseX = 0;
	user_actions.mouseY = 0;
	user_actions.clicked_play = false;
	user_actions.clicked_infos = false;
	user_actions.clicked_pod_specs = false;
	user_actions.clicked_main_menu = false;
	user_actions.clicked_quit = false;
	user_actions.clicked_map = false;
	user_actions.clicked_controls = false;
	user_actions.clicked_back = false;
	
	// menu navigation
	current_page = MAIN;
	int indices[] = {0, 1, 2, 2, 1, 3};
	
	glGenVertexArrays(1, &tuningVAO);
	glBindVertexArray(tuningVAO);

	glGenBuffers(1, &tuningVBO);
	glBindBuffer(GL_ARRAY_BUFFER, tuningVBO);
	glBufferData(GL_ARRAY_BUFFER, tuning_button.size() * sizeof(float), tuning_button.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &tuningEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tuningEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	glGenVertexArrays(1, &backVAO);
	glBindVertexArray(backVAO);

	glGenBuffers(1, &backVBO);
	glBindBuffer(GL_ARRAY_BUFFER, backVBO);
	glBufferData(GL_ARRAY_BUFFER, back_button.size() * sizeof(float), back_button.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &backEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// UI shader
	ui_shader = new Shader("../shaders/menu/vertex.glsl", "../shaders/menu/fragment.glsl", "../shaders/menu/geometry.glsl");
	ui_shader->use();
	ui_shader->set_int("img", 0);
	ui_shader->set_float("alpha", 1.0f);
    ui_shader->set_int("lap_timer_anim", 0);
    ui_shader->set_float("delta_anim", delta_anim);
	glActiveTexture(GL_TEXTURE0);

	// MENU BOUNDING BOXES
	bb_play.top_left_x = (WIDTH / 8) * 3;
	bb_play.top_left_y = (HEIGHT / 2) * 0.334;
	bb_play.bottom_right_x = (WIDTH / 8) * 5;
	bb_play.bottom_right_y = (HEIGHT / 2) * 0.5;
	
	bb_infos.top_left_x = (WIDTH / 8) * 3;
	bb_infos.top_left_y = (HEIGHT / 2) * 0.656;
	bb_infos.bottom_right_x = (WIDTH / 8) * 5;
	bb_infos.bottom_right_y = (HEIGHT / 2) * 0.826;

	bb_pod_specs.top_left_x = (WIDTH / 16) * 5;
	bb_pod_specs.top_left_y = (HEIGHT / 2);
	bb_pod_specs.bottom_right_x = (WIDTH / 16) * 11;
	bb_pod_specs.bottom_right_y = (HEIGHT / 2) * 1.166;
	
	bb_quit.top_left_x = (WIDTH / 8) * 3;
	bb_quit.top_left_y = (HEIGHT / 2) * 1.333;
	bb_quit.bottom_right_x = (WIDTH / 8) * 5;
	bb_quit.bottom_right_y = (HEIGHT / 2) * 1.5;

	bb_tuning.top_left_x = (WIDTH / 30) * 29;
	bb_tuning.top_left_y = 0;
	bb_tuning.bottom_right_x = WIDTH;
	bb_tuning.bottom_right_y = (WIDTH / 40);
	
	bb_back.top_left_x = 0;
	bb_back.top_left_y = 0;
	bb_back.bottom_right_x = (WIDTH / 30);
	bb_back.bottom_right_y = (WIDTH / 40);
	
	bb_map.top_left_x = (WIDTH / 8) * 2;
	bb_map.top_left_y = (HEIGHT / 8) * 3;
	bb_map.bottom_right_x = (WIDTH / 8) * 3;
	bb_map.bottom_right_y = (HEIGHT / 8) * 5;
	
	bb_controls.top_left_x = (WIDTH / 8) * 5;
	bb_controls.top_left_y = (HEIGHT / 8) * 3;
	bb_controls.bottom_right_x = (WIDTH / 8) * 6;
	bb_controls.bottom_right_y = (HEIGHT / 8) * 5;
	
	bb_cast_shadows.top_left_x = WIDTH * 0.43;
	bb_cast_shadows.top_left_y = HEIGHT * 0.34;
	bb_cast_shadows.bottom_right_x = WIDTH * 0.47;
	bb_cast_shadows.bottom_right_y = HEIGHT * 0.391;
	
	bb_close_tuning_window.top_left_x = (WIDTH / 4) * 2.875;
	bb_close_tuning_window.top_left_y = (HEIGHT / 10) * 2;
	bb_close_tuning_window.bottom_right_x = (WIDTH / 4) * 3;
	bb_close_tuning_window.bottom_right_y = (HEIGHT / 10) * 2.55;
	
	bb_sound_slider.top_left_x = WIDTH * 0.615;
	bb_sound_slider.top_left_y = HEIGHT * 0.353;
	bb_sound_slider.bottom_right_x = WIDTH * 0.715;
	bb_sound_slider.bottom_right_y = HEIGHT * 0.38;
	
	bb_quit_no.top_left_x = WIDTH * 0.496;
	bb_quit_no.top_left_y = HEIGHT * 0.375;
	bb_quit_no.bottom_right_x = WIDTH * 0.555;
	bb_quit_no.bottom_right_y = HEIGHT * 0.45;
	
	bb_quit_yes.top_left_x = WIDTH * 0.417;
	bb_quit_yes.top_left_y = HEIGHT * 0.375;
	bb_quit_yes.bottom_right_x = WIDTH * 0.48;
	bb_quit_yes.bottom_right_y = HEIGHT * 0.45;
	
    bb_main_menu.top_left_x = WIDTH * 0.406;
	bb_main_menu.top_left_y = HEIGHT * 0.859;
	bb_main_menu.bottom_right_x = WIDTH * 0.595;
	bb_main_menu.bottom_right_y = HEIGHT * 0.955;
	
	// path to all menu textures
	tex_path.push_back(std::string("../assets/textures/menu/menu_background.png")); // 0
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/play_button.png")); // 1
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/hover_play_button.png")); // 2
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/infos_button.png")); // 3
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/hover_infos_button.png")); // 4
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/pod_specs_button.png")); // 5
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/hover_pod_specs_button.png")); // 6
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/stats_button.png")); // 7
	flip.push_back(false);
	tex_path.push_back(std::string("../assets/textures/menu/hover_stats_button.png")); // 8
	flip.push_back(false);
	tex_path.push_back(std::string("../assets/textures/menu/quit_button.png")); // 9
	flip.push_back(false);
	tex_path.push_back(std::string("../assets/textures/menu/hover_quit_button.png")); // 10
	flip.push_back(false);
	tex_path.push_back(std::string("../assets/textures/menu/tuning.png")); // 11
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/hover_tuning.png")); // 12
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/back_button.png")); // 13
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/back_button_hover.png")); // 14
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/mos_espa_circuit.jpg")); // 15
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/map_button.png")); // 16
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/map_button_hover.png")); // 17
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/controls_button.png")); // 18
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/controls_button_hover.png")); // 19
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/back_button_white.png")); // 20
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/tuning_white.png")); // 21
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/podracer_anakin.jpg")); // 22
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/pod_specs_title.png")); // 23
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/pod_specs.png")); // 24
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/control_your_pod.png")); // 25
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/default_tuning_window.png")); // 26
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/default_tuning_window_hover_cross.png")); // 27
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/no_shadows_tuning_window_hover_cross.png")); // 28
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/no_shadows_tuning_window.png")); // 29
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/quit_game.png")); // 30
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/quit_game_no.png")); // 31
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/quit_game_yes.png")); // 32
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/HUD/HUD_speed.png")); // 33
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/HUD/HUD_speed_bar.png")); // 34
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/0.png")); // 35
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/1.png")); // 36
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/2.png")); // 37
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/3.png")); // 38
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/4.png")); // 39
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/5.png")); // 40
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/6.png")); // 41
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/7.png")); // 42
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/8.png")); // 43
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/9.png")); // 44
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/sound_volume_button.png")); // 45
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/HUD/HUD_top_bar.png")); // 46
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/0w.png")); // 47
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/1w.png")); // 48
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/2w.png")); // 49
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/3w.png")); // 50
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/4w.png")); // 51
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/5w.png")); // 52
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/6w.png")); // 53
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/7w.png")); // 54
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/8w.png")); // 55
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/9w.png")); // 56
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/dotsw.png")); // 57
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/slashw.png")); // 58
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/dotw.png")); // 59
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/timer_3.png")); // 60
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/timer_2.png")); // 61
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/timer_1.png")); // 62
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/digits/go.png")); // 63
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/end_game_stats.png")); // 64
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/end_game_stats_hover.png")); // 65
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/min.png")); // 66
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/sec.png")); // 67
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/ms.png")); // 68
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/kmh.png")); // 69
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/loading_background.png")); // 70
	flip.push_back(true);
	tex_path.push_back(std::string("../assets/textures/menu/loading_screen_assets.png")); // 71
	flip.push_back(false);
	set_menu_textures();
    
    // ########## start loading screen geometry ##########
	GLuint VAO1, VBO1, EBO1;
	GLuint VAO2, VBO2, EBO2;
	
	float load_background[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(load_background), load_background, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
    float load[] = {
		-0.2f, -0.8f, -0.45f, 0.0f, 1.0f,
		0.2f, -0.8f, -0.45f, 1.0f, 1.0f,
		-0.2f, -0.433f, -0.45f, 0.0f, 0.0f,
		0.2f, -0.433f, -0.45f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);

	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(load), load, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &EBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	
    // ########## end loading screen geometry ##########
	
    // #################### LOADING SCREEN ####################
    // draw loading screen background image
	glBindTexture(GL_TEXTURE_2D, menu_textures[70].id);
	glBindVertexArray(VAO1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // draw loading screen message
	glBindTexture(GL_TEXTURE_2D, menu_textures[71].id);
	glBindVertexArray(VAO2);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);

    // FRAMEBUFFERS
	set_framebuffers();
	
	// platform
	platform = new Environment("Platform", "../assets/platform/platform.obj", this);
	
	// environment
	env = new Environment("Tatooine", this);
	
	// podracer
	pod = new Podracer("anakin_podracer", this);
	cam = pod_specs_cam;
	
	// minimap
	minimap = new Minimap("../assets/environment/minimap.obj", this);
	
    // World Physics
    tatooine = new WorldPhysics(this);
	
    // Draw master
    draw_master = new DrawMaster();
    draw_master->build_tree(env->get_mesh_collection());
	
	// init timer
	timer = 0.0;
	timer1 = 0.0;
	timer2 = 0.0;
	timer3 = 0.0;

	timer_min_d0 = 0;
	timer_min_d1 = 0;
	timer_sec_d0 = 0;
	timer_sec_d1 = 0;
	timer_ms_d0 = 0;
	timer_ms_d1 = 0;

	// init countdown timer
	countdown_timer = 3;

	// SOUNDS
	sounds = new Audio();
	sounds->load_sound("../assets/audio/the_pod_race.wav"); // 0
	sounds->load_sound("../assets/audio/fire_power_coupling.wav"); // 1
	sounds->load_sound("../assets/audio/power_coupling.wav"); // 2
	sounds->load_sound("../assets/audio/start_electric_engine.wav"); // 3
	sounds->load_sound("../assets/audio/electric_engine_idle.wav"); // 4
	sounds->load_sound("../assets/audio/fire_engine.wav"); // 5
	sounds->load_sound("../assets/audio/base_engine.wav"); // 6
	sounds->load_sound("../assets/audio/break_engine.wav"); // 7
	sounds->load_sound("../assets/audio/new_lap_record.wav"); // 8
	sounds->load_sound("../assets/audio/afterburn.wav"); // 9
	sounds->load_sound("../assets/audio/countdown.wav"); // 10
	sounds->load_sound("../assets/audio/go.wav"); // 11
	sounds->load_sound("../assets/audio/collide.wav"); // 12
	sounds->load_sound("../assets/audio/crash.wav"); // 13
	
	main_menu_source = new Source();
	main_menu_source->set_volume(sound_volume);
	main_menu_source->set_looping(true);
	
	pod_fire_power_coupling = new Source();
	pod_fire_power_coupling->set_volume(100);
	
	pod_power_coupling = new Source();
	pod_power_coupling->set_volume(100);
	pod_power_coupling->set_looping(true);

	pod_start_electric_engine = new Source();
	pod_start_electric_engine->set_volume(30);
	
	pod_electric_engine = new Source();
	pod_electric_engine->set_volume(30);
	pod_electric_engine->set_looping(true);

	pod_fire_engine = new Source();
	pod_fire_engine->set_volume(60);

	pod_base_engine = new Source();
	pod_base_engine->set_volume(30);
	pod_base_engine->set_looping(true);

	pod_break = new Source();
	pod_break->set_volume(100);
	
	pod_afterburn = new Source();
	pod_afterburn->set_volume(100);
	
	fodesinbeed = new Source();
	fodesinbeed->set_volume(100);

	tatooine_wind = new Source();
	tatooine_wind->set_volume(30);
	tatooine_wind->set_looping(true);
	
	countdown_sounds = new Source();
	countdown_sounds->set_volume(100);

    pod_collide = new Source();
    pod_collide->set_volume(60);
    
    pod_crash = new Source();
    pod_crash->set_volume(80);

	// render pass visualization
	check_render_pass = false;
	render_pass = 0;
	
    // delete loading screen VAOs
    glDeleteBuffers(1, &VBO1);
	glDeleteBuffers(1, &VBO2);
			
	glDeleteBuffers(1, &EBO1);
	glDeleteBuffers(1, &EBO2);

	glDeleteVertexArrays(1, &VAO1);
	glDeleteVertexArrays(1, &VAO2);
}

void Game::set_framebuffers()
{
	// =-=-=-=-= Create shadowmap framebuffer =-=-=-=-=
	shadowMapWidth = 4096;
	shadowMapHeight = 4096;

    // ++++++++++ first shadowmap ++++++++++
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: shadow framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
    
    // ++++++++++ second shadowmap ++++++++++
	glGenFramebuffers(1, &shadowMap2FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap2FBO);

	glGenTextures(1, &shadowMap2);
	glBindTexture(GL_TEXTURE_2D, shadowMap2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap2, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: shadow2 framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create env framebuffer
	glGenFramebuffers(1, &envFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, envFBO);

	glGenTextures(1, &envTexture);
	glBindTexture(GL_TEXTURE_2D, envTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, envTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &envRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, envRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, envRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: env framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create smoke framebuffer
	glGenFramebuffers(1, &smokeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, smokeFBO);

	glGenTextures(1, &smokeTexture);
	glBindTexture(GL_TEXTURE_2D, smokeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smokeTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &smokeRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, smokeRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, smokeRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: smoke framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create env motion blur framebuffer
	glGenFramebuffers(1, &envMotionBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, envMotionBlurFBO);

	glGenTextures(1, &envMotionBlurTexture);
	glBindTexture(GL_TEXTURE_2D, envMotionBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, envMotionBlurTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &envMotionBlurRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, envMotionBlurRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, envMotionBlurRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: env motion blur framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create smoke motion blur framebuffer
	glGenFramebuffers(1, &smokeMotionBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, smokeMotionBlurFBO);

	glGenTextures(1, &smokeMotionBlurTexture);
	glBindTexture(GL_TEXTURE_2D, smokeMotionBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smokeMotionBlurTexture, 0);
	
	glGenTextures(1, &smokeBrightMotionBlurTexture);
	glBindTexture(GL_TEXTURE_2D, smokeBrightMotionBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, smokeBrightMotionBlurTexture, 0);

	GLuint smokeBrightMotionBlurAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, smokeBrightMotionBlurAttachments);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &smokeMotionBlurRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, smokeMotionBlurRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, smokeMotionBlurRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: smoke motion blur framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create depth framebuffer
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: depth framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create depth bis framebuffer
	glGenFramebuffers(1, &depthBisFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthBisFBO);

	glGenTextures(1, &depthBisTexture);
	glBindTexture(GL_TEXTURE_2D, depthBisTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBisTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: depth bis framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create podracer depth framebuffer
	glGenFramebuffers(1, &depthPodFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthPodFBO);

	glGenTextures(1, &depthPodTexture);
	glBindTexture(GL_TEXTURE_2D, depthPodTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthPodTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: podracer depth framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create podracer framebuffer
	glGenFramebuffers(1, &podracerFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, podracerFBO);

	glGenTextures(1, &podracerTexture);
	glBindTexture(GL_TEXTURE_2D, podracerTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, podracerTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &podRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, podRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, podRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: podracer framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create color framebuffer
	glGenFramebuffers(1, &colorFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &colorRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, colorRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: color framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create ping framebuffer
	glGenFramebuffers(1, &pingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pingFBO);

	glGenTextures(1, &pingTexture);
	glBindTexture(GL_TEXTURE_2D, pingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &pingRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, pingRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pingRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: ping framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create pong framebuffer
	glGenFramebuffers(1, &pongFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pongFBO);

	glGenTextures(1, &pongTexture);
	glBindTexture(GL_TEXTURE_2D, pongTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pongTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &pongRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, pongRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pongRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: pong framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create ping2 framebuffer
	glGenFramebuffers(1, &ping2FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ping2FBO);

	glGenTextures(1, &ping2Texture);
	glBindTexture(GL_TEXTURE_2D, ping2Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping2Texture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &ping2RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, ping2RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ping2RBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: ping2 framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create pong2 framebuffer
	glGenFramebuffers(1, &pong2FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pong2FBO);

	glGenTextures(1, &pong2Texture);
	glBindTexture(GL_TEXTURE_2D, pong2Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pong2Texture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &pong2RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, pong2RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pong2RBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: pong2 framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Game::update_framebuffers()
{
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// Create env framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &envRBO);
    glDeleteTextures(1, &envTexture);
    glDeleteFramebuffers(1, &envFBO);
    // ----- end delete
	glGenFramebuffers(1, &envFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, envFBO);

	glGenTextures(1, &envTexture);
	glBindTexture(GL_TEXTURE_2D, envTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, envTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &envRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, envRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, envRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: env framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create smoke framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &smokeRBO);
    glDeleteTextures(1, &smokeTexture);
    glDeleteFramebuffers(1, &smokeFBO);
    // ----- end delete
	glGenFramebuffers(1, &smokeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, smokeFBO);

	glGenTextures(1, &smokeTexture);
	glBindTexture(GL_TEXTURE_2D, smokeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smokeTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &smokeRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, smokeRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, smokeRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: smoke framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create env motion blur framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &envMotionBlurRBO);
    glDeleteTextures(1, &envMotionBlurTexture);
    glDeleteFramebuffers(1, &envMotionBlurFBO);
    // ----- end delete
	glGenFramebuffers(1, &envMotionBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, envMotionBlurFBO);

	glGenTextures(1, &envMotionBlurTexture);
	glBindTexture(GL_TEXTURE_2D, envMotionBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, envMotionBlurTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &envMotionBlurRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, envMotionBlurRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, envMotionBlurRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: env motion blur framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create smoke motion blur framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &smokeMotionBlurRBO);
    glDeleteTextures(1, &smokeMotionBlurTexture);
    glDeleteTextures(1, &smokeBrightMotionBlurTexture);
    glDeleteFramebuffers(1, &smokeMotionBlurFBO);
    // ----- end delete
	glGenFramebuffers(1, &smokeMotionBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, smokeMotionBlurFBO);

	glGenTextures(1, &smokeMotionBlurTexture);
	glBindTexture(GL_TEXTURE_2D, smokeMotionBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smokeMotionBlurTexture, 0);
	
	glGenTextures(1, &smokeBrightMotionBlurTexture);
	glBindTexture(GL_TEXTURE_2D, smokeBrightMotionBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, smokeBrightMotionBlurTexture, 0);

	GLuint smokeBrightMotionBlurAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, smokeBrightMotionBlurAttachments);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &smokeMotionBlurRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, smokeMotionBlurRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, smokeMotionBlurRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: smoke motion blur framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create depth framebuffer
    // ----- start delete
    glDeleteTextures(1, &depthTexture);
    glDeleteFramebuffers(1, &depthFBO);
    // ----- end delete
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: depth framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create depth bis framebuffer
    // ----- start delete
    glDeleteTextures(1, &depthBisTexture);
    glDeleteFramebuffers(1, &depthBisFBO);
    // ----- end delete
	glGenFramebuffers(1, &depthBisFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthBisFBO);

	glGenTextures(1, &depthBisTexture);
	glBindTexture(GL_TEXTURE_2D, depthBisTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBisTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: depth bis framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create podracer depth framebuffer
    // ----- start delete
    glDeleteTextures(1, &depthPodTexture);
    glDeleteFramebuffers(1, &depthPodFBO);
    // ----- end delete
	glGenFramebuffers(1, &depthPodFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthPodFBO);

	glGenTextures(1, &depthPodTexture);
	glBindTexture(GL_TEXTURE_2D, depthPodTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthPodTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: podracer depth framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create podracer framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &podRBO);
    glDeleteTextures(1, &podracerTexture);
    glDeleteFramebuffers(1, &podracerFBO);
    // ----- end delete
	glGenFramebuffers(1, &podracerFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, podracerFBO);

	glGenTextures(1, &podracerTexture);
	glBindTexture(GL_TEXTURE_2D, podracerTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, podracerTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &podRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, podRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, podRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: podracer framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create color framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &colorRBO);
    glDeleteTextures(1, &colorTexture);
    glDeleteFramebuffers(1, &colorFBO);
    // ----- end delete
	glGenFramebuffers(1, &colorFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &colorRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, colorRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: color framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create ping framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &pingRBO);
    glDeleteTextures(1, &pingTexture);
    glDeleteFramebuffers(1, &pingFBO);
    // ----- end delete
	glGenFramebuffers(1, &pingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pingFBO);

	glGenTextures(1, &pingTexture);
	glBindTexture(GL_TEXTURE_2D, pingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &pingRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, pingRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pingRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: ping framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create pong framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &pongRBO);
    glDeleteTextures(1, &pongTexture);
    glDeleteFramebuffers(1, &pongFBO);
    // ----- end delete
	glGenFramebuffers(1, &pongFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pongFBO);

	glGenTextures(1, &pongTexture);
	glBindTexture(GL_TEXTURE_2D, pongTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pongTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &pongRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, pongRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pongRBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: pong framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create ping2 framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &ping2RBO);
    glDeleteTextures(1, &ping2Texture);
    glDeleteFramebuffers(1, &ping2FBO);
    // ----- end delete
	glGenFramebuffers(1, &ping2FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ping2FBO);

	glGenTextures(1, &ping2Texture);
	glBindTexture(GL_TEXTURE_2D, ping2Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping2Texture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &ping2RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, ping2RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ping2RBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: ping2 framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Create pong2 framebuffer
    // ----- start delete
    glDeleteRenderbuffers(1, &pong2RBO);
    glDeleteTextures(1, &pong2Texture);
    glDeleteFramebuffers(1, &pong2FBO);
    // ----- end delete
	glGenFramebuffers(1, &pong2FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pong2FBO);

	glGenTextures(1, &pong2Texture);
	glBindTexture(GL_TEXTURE_2D, pong2Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pong2Texture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &pong2RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, pong2RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pong2RBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: pong2 framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Game::~Game()
{
	delete(editor_cam);
	delete(racing_cam);
	delete(pod_specs_cam);
	delete(pod);
	delete(env);
	delete(platform);
	delete(minimap);
	delete(sky);
	delete(ui_shader);
    delete(tatooine);
    delete(draw_master);
	delete(sounds);
	delete(main_menu_source);
	delete(pod_fire_power_coupling);
	delete(pod_power_coupling);
	delete(pod_start_electric_engine);
	delete(pod_electric_engine);
	delete(pod_fire_engine);
	delete(pod_base_engine);
	delete(pod_break);
	delete(pod_afterburn);
	delete(fodesinbeed);
	delete(tatooine_wind);
	delete(countdown_sounds);
    delete(pod_collide);
    delete(pod_crash);
    
    glDeleteRenderbuffers(1, &envRBO);
    glDeleteTextures(1, &envTexture);
    glDeleteFramebuffers(1, &envFBO);
    
    glDeleteRenderbuffers(1, &smokeRBO);
    glDeleteTextures(1, &smokeTexture);
    glDeleteFramebuffers(1, &smokeFBO);
    
    glDeleteRenderbuffers(1, &envMotionBlurRBO);
    glDeleteTextures(1, &envMotionBlurTexture);
    glDeleteFramebuffers(1, &envMotionBlurFBO);
    
    glDeleteRenderbuffers(1, &smokeMotionBlurRBO);
    glDeleteTextures(1, &smokeMotionBlurTexture);
    glDeleteTextures(1, &smokeBrightMotionBlurTexture);
    glDeleteFramebuffers(1, &smokeMotionBlurFBO);
    
    glDeleteTextures(1, &depthTexture);
    glDeleteFramebuffers(1, &depthFBO);
    
    glDeleteTextures(1, &depthBisTexture);
    glDeleteFramebuffers(1, &depthBisFBO);
    
    glDeleteTextures(1, &depthPodTexture);
    glDeleteFramebuffers(1, &depthPodFBO);
    
    glDeleteRenderbuffers(1, &podRBO);
    glDeleteTextures(1, &podracerTexture);
    glDeleteFramebuffers(1, &podracerFBO);
    
    glDeleteRenderbuffers(1, &colorRBO);
    glDeleteTextures(1, &colorTexture);
    glDeleteFramebuffers(1, &colorFBO);
    
    glDeleteRenderbuffers(1, &pingRBO);
    glDeleteTextures(1, &pingTexture);
    glDeleteFramebuffers(1, &pingFBO);
    
    glDeleteRenderbuffers(1, &pongRBO);
    glDeleteTextures(1, &pongTexture);
    glDeleteFramebuffers(1, &pongFBO);
    
    glDeleteRenderbuffers(1, &ping2RBO);
    glDeleteTextures(1, &ping2Texture);
    glDeleteFramebuffers(1, &ping2FBO);
    
    glDeleteRenderbuffers(1, &pong2RBO);
    glDeleteTextures(1, &pong2Texture);
    glDeleteFramebuffers(1, &pong2FBO);
}

SDL_Window* Game::createWindow(int w, int h, const std::string& title)
{
	SDL_Window* window = nullptr;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << SDL_GetError() << std::endl;
		return nullptr;
	}

	// OPENGL VERSION
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

	// DOUBLE BUFFER
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	window = SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			w,
			h,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
	
	if(window == nullptr)
	{
		std::cerr << SDL_GetError() << std::endl;
		return nullptr;
	}

	return window;
}

void Game::set_menu_textures()
{
	int tex_count = tex_path.size();
	for(int t = 0; t < tex_count; t++)
	{
		GLuint tex_id = Object::create_texture(tex_path[t], flip[t]);
		Texture tex(tex_id, TextureType::DIFFUSE_TEXTURE, tex_path[t]);
		menu_textures.push_back(tex);
	}
}

void Game::start()
{
    // set screen to menu width and height
    width = menu_width;
    height = menu_height;
    if(menu_fullscreen)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    else
        SDL_SetWindowFullscreen(window, 0);

    SDL_SetWindowSize(window, width, height);
	glViewport(0, 0, width, height);
	update_menu_bb(width, height);
    update_framebuffers();

    // set current page
	current_page = MAIN;

	// sound system
	sound_system();

	// disable gamma correction
	glDisable(GL_FRAMEBUFFER_SRGB);

	// menu geometry
	int indices[] = {0, 1, 2, 2, 1, 3};

	GLuint VAO1, VBO1, EBO1;
	GLuint VAO2, VBO2, EBO2;
	GLuint VAO3, VBO3, EBO3;
	GLuint VAO4, VBO4, EBO4;
	GLuint VAO5, VBO5, EBO5;
	
	float menu_background[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(menu_background), menu_background, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	float play_button[] = {
		-0.2f, 0.5f, -0.15f, 0.0f, 0.0f,
		0.2f, 0.5f, -0.15f, 1.0f, 0.0f,
		-0.2f, 0.666f, -0.15f, 0.0f, 1.0f,
		0.2f, 0.666f, -0.15f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);
	
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(play_button), play_button, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	float infos_button[] = {
		-0.2f, 0.174f, -0.30f, 0.0f, 0.0f,
		0.2f, 0.174f, -0.30f, 1.0f, 0.0f,
		-0.2f, 0.344f, -0.30f, 0.0f, 1.0f,
		0.2f, 0.344f, -0.30f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO3);
	glBindVertexArray(VAO3);
	
	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(infos_button), infos_button, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	float pod_specs_button[] = {
		-0.25f, -0.166f, -0.30f, 0.0f, 0.0f,
		0.25f, -0.166f, -0.30f, 1.0f, 0.0f,
		-0.25f, 0.0f, -0.30f, 0.0f, 1.0f,
		0.25f, 0.0f, -0.30f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO4);
	glBindVertexArray(VAO4);
	
	glGenBuffers(1, &VBO4);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pod_specs_button), pod_specs_button, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	float quit_button[] = {
		-0.2f, -0.5f, -0.45f, 0.0f, 1.0f,
		0.2f, -0.5f, -0.45f, 1.0f, 1.0f,
		-0.2f, -0.333f, -0.45f, 0.0f, 0.0f,
		0.2f, -0.333f, -0.45f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO5);
	glBindVertexArray(VAO5);

	glGenBuffers(1, &VBO5);
	glBindBuffer(GL_ARRAY_BUFFER, VBO5);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quit_button), quit_button, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &EBO5);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO5);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// use ui_shader
	glActiveTexture(GL_TEXTURE0);
	ui_shader->use();
	
	// render loop
	reset(); // important line to print the shadows I DON'T KNOW WHY !!!
	show_menu = true;
		
	while(show_menu)
	{
		check_events();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(show_tuning)
		{
			// color framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
			glEnable(GL_DEPTH);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			// draw background image
			glBindTexture(GL_TEXTURE_2D, menu_textures[0].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// draw tuning button
			glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// draw play button
			glBindTexture(GL_TEXTURE_2D, menu_textures[1].id);
			glBindVertexArray(VAO2);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// draw infos button
			glBindTexture(GL_TEXTURE_2D, menu_textures[3].id);
			glBindVertexArray(VAO3);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// draw pod specs button
			glBindTexture(GL_TEXTURE_2D, menu_textures[5].id);
			glBindVertexArray(VAO4);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// draw quit button
			glBindTexture(GL_TEXTURE_2D, menu_textures[9].id);
			glBindVertexArray(VAO5);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	

			// default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			tuning();
			
			// reset shader
			glActiveTexture(GL_TEXTURE0);
			ui_shader->use();
		}

		else
		{
			current_page = MAIN;
			if(user_actions.clicked_play || user_actions.clicked_quit || user_actions.clicked_infos || user_actions.clicked_pod_specs)
			{
				show_menu = false;
				glDeleteBuffers(1, &VBO1);
				glDeleteBuffers(1, &VBO2);
				glDeleteBuffers(1, &VBO3);
				glDeleteBuffers(1, &VBO4);
				glDeleteBuffers(1, &VBO5);
			
				glDeleteBuffers(1, &EBO1);
				glDeleteBuffers(1, &EBO2);
				glDeleteBuffers(1, &EBO3);
				glDeleteBuffers(1, &EBO4);
				glDeleteBuffers(1, &EBO5);

				glDeleteVertexArrays(1, &VAO1);
				glDeleteVertexArrays(1, &VAO2);
				glDeleteVertexArrays(1, &VAO3);
				glDeleteVertexArrays(1, &VAO4);
				glDeleteVertexArrays(1, &VAO5);
			}

			// draw background image
			glBindTexture(GL_TEXTURE_2D, menu_textures[0].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// draw tuning button
			if(user_actions.mouseX >= bb_tuning.top_left_x && user_actions.mouseX <= bb_tuning.bottom_right_x && user_actions.mouseY >= bb_tuning.top_left_y && user_actions.mouseY <= bb_tuning.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[12].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			}
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// draw play button
			if(user_actions.mouseX >= bb_play.top_left_x && user_actions.mouseX <= bb_play.bottom_right_x && user_actions.mouseY >= bb_play.top_left_y && user_actions.mouseY <= bb_play.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[2].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[1].id);
			}
			glBindVertexArray(VAO2);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// draw infos button
			if(user_actions.mouseX >= bb_infos.top_left_x && user_actions.mouseX <= bb_infos.bottom_right_x && user_actions.mouseY >= bb_infos.top_left_y && user_actions.mouseY <= bb_infos.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[4].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[3].id);
			}
			glBindVertexArray(VAO3);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// draw pod specs button
			if(user_actions.mouseX >= bb_pod_specs.top_left_x && user_actions.mouseX <= bb_pod_specs.bottom_right_x && user_actions.mouseY >= bb_pod_specs.top_left_y && user_actions.mouseY <= bb_pod_specs.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[6].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[5].id);
			}
			glBindVertexArray(VAO4);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// draw quit button
			if(user_actions.mouseX >= bb_quit.top_left_x && user_actions.mouseX <= bb_quit.bottom_right_x && user_actions.mouseY >= bb_quit.top_left_y && user_actions.mouseY <= bb_quit.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[10].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[9].id);
			}
			glBindVertexArray(VAO5);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		SDL_GL_SwapWindow(window);
	}

	if(user_actions.clicked_play)
	{
		user_actions.clicked_play = false;
		play();
	}
	else if(user_actions.clicked_infos)
	{
		user_actions.clicked_infos = false;
		gameInfo();
	}
	else if(user_actions.clicked_pod_specs)
	{
		user_actions.clicked_pod_specs = false;
		pod_specs();
	}
	else if(user_actions.clicked_quit)
	{
		quit();
	}
}

void Game::quit()
{
	if(window != nullptr)
	{
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
}

void Game::tuning()
{	
	current_page = TUNING;
	
	// geometry
	int indices[] = {0, 1, 2, 2, 1, 3};
	
	GLuint VAO1, VBO1, EBO1;
	
	float tuning_window[] = {
		-0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
		0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
		-0.5f, 0.6f, -0.5f, 0.0f, 1.0f,
		0.5f, 0.6f, -0.5f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tuning_window), tuning_window, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// sound volume button
	float sound_button[] = {
		0.23f + (static_cast<float>(sound_volume)/595.0f), 0.25f, -0.55f, 0.0f, 0.0f,
		0.25f + (static_cast<float>(sound_volume)/595.0f), 0.25f, -0.55f, 1.0f, 0.0f,
		0.23f + (static_cast<float>(sound_volume)/595.0f), 0.28f, -0.55f, 0.0f, 1.0f,
		0.25f + (static_cast<float>(sound_volume)/595.0f), 0.28f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint soundButtonVAO, soundButtonVBO, soundButtonEBO;

	glGenVertexArrays(1, &soundButtonVAO);
	glBindVertexArray(soundButtonVAO);

	glGenBuffers(1, &soundButtonVBO);
	glBindBuffer(GL_ARRAY_BUFFER, soundButtonVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sound_button), sound_button, GL_DYNAMIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &soundButtonEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, soundButtonEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// post process quad and its grey shader
	float quad[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	
	Shader grey_shader("../shaders/greyscale/vertex.glsl", "../shaders/greyscale/fragment.glsl", "../shaders/greyscale/geometry.glsl");

	while(show_tuning)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// user input
		check_events();

		// post process quad
		glBindVertexArray(VAO);
		grey_shader.use();
		glActiveTexture(GL_TEXTURE0);
		grey_shader.set_int("img", 0);
		grey_shader.set_int("apply_greyScale", 1);
		glBindTexture(GL_TEXTURE_2D, colorTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// gamma correction
		glEnable(GL_FRAMEBUFFER_SRGB);

		// show tuning window
		glActiveTexture(GL_TEXTURE0);
		ui_shader->use();
	
		if(user_actions.mouseX >= bb_close_tuning_window.top_left_x && user_actions.mouseX <= bb_close_tuning_window.bottom_right_x && user_actions.mouseY >= bb_close_tuning_window.top_left_y && user_actions.mouseY <= bb_close_tuning_window.bottom_right_y)
		{
			if(cast_shadows)
				glBindTexture(GL_TEXTURE_2D, menu_textures[27].id);
			else
				glBindTexture(GL_TEXTURE_2D, menu_textures[28].id);
		}
		else
		{
			if(cast_shadows)
				glBindTexture(GL_TEXTURE_2D, menu_textures[26].id);
			else
				glBindTexture(GL_TEXTURE_2D, menu_textures[29].id);
		}
		glBindVertexArray(VAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// show sound volume button
		glBindBuffer(GL_ARRAY_BUFFER, soundButtonVBO);
		float sound_button_update[] = {
			0.23f + (static_cast<float>(sound_volume)/595.0f), 0.25f, -0.55f, 0.0f, 0.0f,
			0.25f + (static_cast<float>(sound_volume)/595.0f), 0.25f, -0.55f, 1.0f, 0.0f,
			0.23f + (static_cast<float>(sound_volume)/595.0f), 0.28f, -0.55f, 0.0f, 1.0f,
			0.25f + (static_cast<float>(sound_volume)/595.0f), 0.28f, -0.55f, 1.0f, 1.0f
		};
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sound_button_update), sound_button_update);

		glBindTexture(GL_TEXTURE_2D, menu_textures[45].id);
		glBindVertexArray(soundButtonVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
		// disable gamma correction
		glDisable(GL_FRAMEBUFFER_SRGB);

		SDL_GL_SwapWindow(window);
	}
}

void Game::gameInfo()
{
	// disable gamma correction
	glDisable(GL_FRAMEBUFFER_SRGB);
	
	// set current page
	current_page = INFOS;

	// geometry
	int indices[] = {0, 1, 2, 2, 1, 3};
	
	GLuint VAO1, VBO1, EBO1;
	GLuint VAO2, VBO2, EBO2;
	GLuint VAO3, VBO3, EBO3;

	float menu_background[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(menu_background), menu_background, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	float map_button[] = {
		-0.5f, -0.25f, -0.15f, 0.0f, 0.0f,
		-0.25f, -0.25f, -0.15f, 1.0f, 0.0f,
		-0.5f, 0.25f, -0.15f, 0.0f, 1.0f,
		-0.25f, 0.25f, -0.15f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);

	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(map_button), map_button, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	float controls_button[] = {
		0.25f, -0.25f, -0.15f, 0.0f, 0.0f,
		0.5f, -0.25f, -0.15f, 1.0f, 0.0f,
		0.25f, 0.25f, -0.15f, 0.0f, 1.0f,
		0.5f, 0.25f, -0.15f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO3);
	glBindVertexArray(VAO3);

	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(controls_button), controls_button, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// use ui_shader
	ui_shader->use();

	// render loop
	show_info_menu = true;

	while(show_info_menu)
	{
		check_events();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		// switch page
		if(show_tuning)
		{
			// color framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
			glEnable(GL_DEPTH);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// display menu background
			glBindTexture(GL_TEXTURE_2D, menu_textures[22].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display map button
			glBindTexture(GL_TEXTURE_2D, menu_textures[16].id);
			glBindVertexArray(VAO2);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display podracer's controls button
			glBindTexture(GL_TEXTURE_2D, menu_textures[18].id);
			glBindVertexArray(VAO3);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display back button
			glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			tuning();

			// reset shader
			glActiveTexture(GL_TEXTURE0);
			ui_shader->use();
		}
		else
		{
			current_page = INFOS;
			
			if(user_actions.clicked_map || user_actions.clicked_controls || user_actions.clicked_back)
			{
				show_info_menu = false;
				
                glDeleteBuffers(1, &VBO1);
				glDeleteBuffers(1, &VBO2);
				glDeleteBuffers(1, &VBO3);
			
				glDeleteBuffers(1, &EBO1);
				glDeleteBuffers(1, &EBO2);
				glDeleteBuffers(1, &EBO3);

				glDeleteVertexArrays(1, &VAO1);
				glDeleteVertexArrays(1, &VAO2);
				glDeleteVertexArrays(1, &VAO3);
			}
			// display menu background
			glBindTexture(GL_TEXTURE_2D, menu_textures[22].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display map button
			if(user_actions.mouseX >= bb_map.top_left_x && user_actions.mouseX <= bb_map.bottom_right_x && user_actions.mouseY >= bb_map.top_left_y && user_actions.mouseY <= bb_map.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[17].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[16].id);
			}
			glBindVertexArray(VAO2);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display podracer's controls button
			if(user_actions.mouseX >= bb_controls.top_left_x && user_actions.mouseX <= bb_controls.bottom_right_x && user_actions.mouseY >= bb_controls.top_left_y && user_actions.mouseY <= bb_controls.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[19].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[18].id);
			}
			glBindVertexArray(VAO3);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display back button
			if(user_actions.mouseX >= bb_back.top_left_x && user_actions.mouseX <= bb_back.bottom_right_x && user_actions.mouseY >= bb_back.top_left_y && user_actions.mouseY <= bb_back.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[14].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			}
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			if(user_actions.mouseX >= bb_tuning.top_left_x && user_actions.mouseX <= bb_tuning.bottom_right_x && user_actions.mouseY >= bb_tuning.top_left_y && user_actions.mouseY <= bb_tuning.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[12].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			}
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		
		SDL_GL_SwapWindow(window);
	}

	if(user_actions.clicked_back)
	{
		user_actions.clicked_back = false;
		start();
	}
	else if(user_actions.clicked_map)
	{
		user_actions.clicked_map = false;
		map();
	}
	else if(user_actions.clicked_controls)
	{
		user_actions.clicked_controls = false;
		podracer_controls();
	}
}

void Game::map()
{
	// disable gamma correction
	glDisable(GL_FRAMEBUFFER_SRGB);
	
	current_page = MAP;

	// geometry
	int indices[] = {0, 1, 2, 2, 1, 3};
	
	GLuint VAO1, VBO1, EBO1;

	float map[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(map), map, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// use ui_shader
	ui_shader->use();

	// render loop
	show_map = true;

	while(show_map)
	{
		check_events();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// switch page
		if(show_tuning)
		{
			// color framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
			glEnable(GL_DEPTH);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			// display map
			glBindTexture(GL_TEXTURE_2D, menu_textures[15].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display back button
			glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			tuning();

			// reset shader
			glActiveTexture(GL_TEXTURE0);
			ui_shader->use();
		}
		
		else
		{
			current_page = MAP;
			if(user_actions.clicked_back)
			{
				glDeleteBuffers(1, &VBO1);
				glDeleteBuffers(1, &EBO1);
				glDeleteVertexArrays(1, &VAO1);

				if(user_actions.clicked_back)
				{
					show_map = false;
					user_actions.clicked_back = false;
					gameInfo();
				}
			}

			// display map
			glBindTexture(GL_TEXTURE_2D, menu_textures[15].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display back button
			if(user_actions.mouseX >= bb_back.top_left_x && user_actions.mouseX <= bb_back.bottom_right_x && user_actions.mouseY >= bb_back.top_left_y && user_actions.mouseY <= bb_back.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[14].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			}
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			if(user_actions.mouseX >= bb_tuning.top_left_x && user_actions.mouseX <= bb_tuning.bottom_right_x && user_actions.mouseY >= bb_tuning.top_left_y && user_actions.mouseY <= bb_tuning.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[12].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			}
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		SDL_GL_SwapWindow(window);
	}
}

void Game::podracer_controls()
{
	// disable gamma correction
	glDisable(GL_FRAMEBUFFER_SRGB);
	
	current_page = CONTROLS;

	// geometry
	int indices[] = {0, 1, 2, 2, 1, 3};
	
	GLuint VAO1, VBO1, EBO1;

	float controls[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(controls), controls, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// use ui_shader
	ui_shader->use();

	// render loop
	show_controls = true;

	while(show_controls)
	{
		check_events();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// switch page
		if(show_tuning)
		{
			// color framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
			glEnable(GL_DEPTH);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			// display controls
			glEnable(GL_FRAMEBUFFER_SRGB);
			glBindTexture(GL_TEXTURE_2D, menu_textures[25].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glDisable(GL_FRAMEBUFFER_SRGB);
		
			// display back button
			glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			tuning();

			// reset shader
			glActiveTexture(GL_TEXTURE0);
			ui_shader->use();
		}

		else
		{
			current_page = CONTROLS;
			if(user_actions.clicked_back)
			{
				glDeleteBuffers(1, &VBO1);
				glDeleteBuffers(1, &EBO1);
				glDeleteVertexArrays(1, &VAO1);

				if(user_actions.clicked_back)
				{
					show_controls = false;
					user_actions.clicked_back = false;
					gameInfo();
				}
			}

			// display controls
			glEnable(GL_FRAMEBUFFER_SRGB);
			glBindTexture(GL_TEXTURE_2D, menu_textures[25].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glDisable(GL_FRAMEBUFFER_SRGB);
		
			// display back button
			if(user_actions.mouseX >= bb_back.top_left_x && user_actions.mouseX <= bb_back.bottom_right_x && user_actions.mouseY >= bb_back.top_left_y && user_actions.mouseY <= bb_back.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[14].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			}
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			if(user_actions.mouseX >= bb_tuning.top_left_x && user_actions.mouseX <= bb_tuning.bottom_right_x && user_actions.mouseY >= bb_tuning.top_left_y && user_actions.mouseY <= bb_tuning.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[12].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			}
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		SDL_GL_SwapWindow(window);
	}
}

void Game::pod_specs()
{
	// disable gamma correction
	glDisable(GL_FRAMEBUFFER_SRGB);
	
	// set cam ptr
	cam = pod_specs_cam;

	current_page = POD_SPECS;
	glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
	
	// geometry
	int indices[] = {0, 1, 2, 2, 1, 3};
	
	GLuint VAO1, VBO1, EBO1;
	GLuint VAO2, VBO2, EBO2;

	// pod specs title
	float title[] = {
		-0.4f, 0.5f, -0.0f, 0.0f, 0.0f,
		0.4f, 0.5f, 0.0f, 1.0f, 0.0f,
		-0.4f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.4f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(title), title, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);	
	
	// pod specs
	float pod_specs[] = {
		0.45f, 0.0f, -0.15f, 0.0f, 0.0f,
		0.85f, 0.0f, -0.15f, 1.0f, 0.0f,
		0.45f, 0.8f, -0.15f, 0.0f, 1.0f,
		0.85f, 0.8f, -0.15f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);

	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pod_specs), pod_specs, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &EBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// render loop
	show_pod_specs = true;

	// delta time for render loop
	lastFrame = omp_get_wtime();
	accuDelta = 0.0;
	animRate = 1.0 / 24.0;

	while(show_pod_specs)
	{
		check_events();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// switch page
		if(show_tuning)
		{
			// color framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
			glEnable(GL_DEPTH);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if(cast_shadows)
			{
				// shadowMap
				render_to_shadowMap(true);
				platform->draw(true);
				pod->draw(true);
			
				// color framebuffer
				glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
				glViewport(0, 0, width, height);
				glEnable(GL_DEPTH);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

			// gamma correction
			glEnable(GL_FRAMEBUFFER_SRGB);
		
			// draw podracer
			pod->draw(false);
		
			// disable gamma correction
			glDisable(GL_FRAMEBUFFER_SRGB);
		
			// draw platform
			platform->draw(false);

			// draw title
			glBindTexture(GL_TEXTURE_2D, menu_textures[23].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// draw pod specs
			ui_shader->set_float("alpha", 0.5f);
			glBindTexture(GL_TEXTURE_2D, menu_textures[24].id);
			glBindVertexArray(VAO2);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			ui_shader->set_float("alpha", 1.0f);
		
			// display back button
			glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			tuning();

			// reset shader
			glActiveTexture(GL_TEXTURE0);
			ui_shader->use();
		}

		else
		{
			current_page = POD_SPECS;
			if(user_actions.clicked_back)
			{
				show_pod_specs = false;

                glDeleteBuffers(1, &VBO1);
				glDeleteBuffers(1, &EBO1);
				glDeleteVertexArrays(1, &VAO1);
			}
	
			// delta calculation
			currentFrame = omp_get_wtime();
			delta = currentFrame - lastFrame;
			lastFrame = currentFrame;
			fps = 1.0 / delta;
	
			if(cast_shadows)
			{
				// shadowMap
				render_to_shadowMap(true);
				platform->draw(true);
				pod->draw(true);

				// default framebuffer
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0, 0, width, height);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

			// gamma correction
			glEnable(GL_FRAMEBUFFER_SRGB);
		
			// view matrix update
			cam->update_view(this, delta);

			// draw podracer
			pod->draw(false);
		
			// disable gamma correction
			glDisable(GL_FRAMEBUFFER_SRGB);
		
			// draw platform
			platform->draw(false);

			// use ui_shader
			ui_shader->use();

			// draw title
			glBindTexture(GL_TEXTURE_2D, menu_textures[23].id);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// draw pod specs
			ui_shader->set_float("alpha", 0.5f);
			glBindTexture(GL_TEXTURE_2D, menu_textures[24].id);
			glBindVertexArray(VAO2);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			ui_shader->set_float("alpha", 1.0f);

			// display back button
			if(user_actions.mouseX >= bb_back.top_left_x && user_actions.mouseX <= bb_back.bottom_right_x && user_actions.mouseY >= bb_back.top_left_y && user_actions.mouseY <= bb_back.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[14].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[13].id);
			}
			glBindVertexArray(backVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
			// display tuning button
			if(user_actions.mouseX >= bb_tuning.top_left_x && user_actions.mouseX <= bb_tuning.bottom_right_x && user_actions.mouseY >= bb_tuning.top_left_y && user_actions.mouseY <= bb_tuning.bottom_right_y)
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[12].id);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, menu_textures[11].id);
			}
			glBindVertexArray(tuningVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		SDL_GL_SwapWindow(window);
	}

	if(user_actions.clicked_back)
	{
		user_actions.clicked_back = false;
		start();
	}
}

void Game::end_game_stats()
{
	// set current page
	current_page = END_GAME_STATS;

	// sound system
	sound_system();

	// menu geometry
	int indices[] = {0, 1, 2, 2, 1, 3};

	GLuint VAO1, VBO1, EBO1;
	
    // l1_min
    GLuint l1_minVAO1, l1_minVBO1, l1_minEBO1;
	GLuint l1_minVAO2, l1_minVBO2, l1_minEBO2;
	GLuint l1_minVAO3, l1_minVBO3, l1_minEBO3;
	// l1_sec
    GLuint l1_secVAO1, l1_secVBO1, l1_secEBO1;
	GLuint l1_secVAO2, l1_secVBO2, l1_secEBO2;
	GLuint l1_secVAO3, l1_secVBO3, l1_secEBO3;
	// l1_ms
    GLuint l1_msVAO1, l1_msVBO1, l1_msEBO1;
	GLuint l1_msVAO2, l1_msVBO2, l1_msEBO2;
	GLuint l1_msVAO3, l1_msVBO3, l1_msEBO3;

    // l2_min
    GLuint l2_minVAO1, l2_minVBO1, l2_minEBO1;
	GLuint l2_minVAO2, l2_minVBO2, l2_minEBO2;
	GLuint l2_minVAO3, l2_minVBO3, l2_minEBO3;
	// l2_sec
    GLuint l2_secVAO1, l2_secVBO1, l2_secEBO1;
	GLuint l2_secVAO2, l2_secVBO2, l2_secEBO2;
	GLuint l2_secVAO3, l2_secVBO3, l2_secEBO3;
	// l2_ms
    GLuint l2_msVAO1, l2_msVBO1, l2_msEBO1;
	GLuint l2_msVAO2, l2_msVBO2, l2_msEBO2;
	GLuint l2_msVAO3, l2_msVBO3, l2_msEBO3;

    // l3_min
    GLuint l3_minVAO1, l3_minVBO1, l3_minEBO1;
	GLuint l3_minVAO2, l3_minVBO2, l3_minEBO2;
	GLuint l3_minVAO3, l3_minVBO3, l3_minEBO3;
	// l3_sec
    GLuint l3_secVAO1, l3_secVBO1, l3_secEBO1;
	GLuint l3_secVAO2, l3_secVBO2, l3_secEBO2;
	GLuint l3_secVAO3, l3_secVBO3, l3_secEBO3;
	// l3_ms
    GLuint l3_msVAO1, l3_msVBO1, l3_msEBO1;
	GLuint l3_msVAO2, l3_msVBO2, l3_msEBO2;
	GLuint l3_msVAO3, l3_msVBO3, l3_msEBO3;
    
    // total_min
    GLuint total_minVAO1, total_minVBO1, total_minEBO1;
	GLuint total_minVAO2, total_minVBO2, total_minEBO2;
	GLuint total_minVAO3, total_minVBO3, total_minEBO3;
	// total_sec
    GLuint total_secVAO1, total_secVBO1, total_secEBO1;
	GLuint total_secVAO2, total_secVBO2, total_secEBO2;
	GLuint total_secVAO3, total_secVBO3, total_secEBO3;
	// total_ms
    GLuint total_msVAO1, total_msVBO1, total_msEBO1;
	GLuint total_msVAO2, total_msVBO2, total_msEBO2;
	GLuint total_msVAO3, total_msVBO3, total_msEBO3;
    
    // avg_speed
    GLuint avgVAO1, avgVBO1, avgEBO1;
	GLuint avgVAO2, avgVBO2, avgEBO2;
	GLuint avgVAO3, avgVBO3, avgEBO3;
	GLuint avgVAO4, avgVBO4, avgEBO4;

	float background[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(background), background, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l1_min1[] = {
        -0.274f, 0.231f, - 0.2f, 0.0f, 0.0f,
        -0.189f, 0.231f, - 0.2f, 1.0f, 0.0f,
        -0.274f, 0.394f, - 0.2f, 0.0f, 1.0f,
        -0.189f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_minVAO1);
	glBindVertexArray(l1_minVAO1);

	glGenBuffers(1, &l1_minVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l1_minVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_min1), l1_min1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_minEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_minEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l1_min2[] = {
        -0.189f, 0.231f, - 0.2f, 0.0f, 0.0f,
        -0.104f, 0.231f, - 0.2f, 1.0f, 0.0f,
        -0.189f, 0.394f, - 0.2f, 0.0f, 1.0f,
        -0.104f, 0.394f, - 0.2f, 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &l1_minVAO2);
	glBindVertexArray(l1_minVAO2);

	glGenBuffers(1, &l1_minVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l1_minVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_min2), l1_min2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_minEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_minEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l1_min3[] = {
        -0.104f, 0.231f, - 0.2f, 0.0f, 0.0f,
        0.066f, 0.231f, - 0.2f, 1.0f, 0.0f,
        -0.104f, 0.394f, - 0.2f, 0.0f, 1.0f,
        0.066f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_minVAO3);
	glBindVertexArray(l1_minVAO3);

	glGenBuffers(1, &l1_minVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l1_minVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_min3), l1_min3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_minEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_minEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l1_sec1[] = {
        0.066f, 0.231f, - 0.2f, 0.0f, 0.0f,
        0.151f, 0.231f, - 0.2f, 1.0f, 0.0f,
        0.066f, 0.394f, - 0.2f, 0.0f, 1.0f,
        0.151f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_secVAO1);
	glBindVertexArray(l1_secVAO1);

	glGenBuffers(1, &l1_secVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l1_secVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_sec1), l1_sec1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_secEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_secEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l1_sec2[] = {
        0.151f, 0.231f, - 0.2f, 0.0f, 0.0f,
        0.236f, 0.231f, - 0.2f, 1.0f, 0.0f,
        0.151f, 0.394f, - 0.2f, 0.0f, 1.0f,
        0.236f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_secVAO2);
	glBindVertexArray(l1_secVAO2);

	glGenBuffers(1, &l1_secVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l1_secVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_sec2), l1_sec2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_secEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_secEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l1_sec3[] = {
        0.236f, 0.231f, - 0.2f, 0.0f, 0.0f,
        0.406f, 0.231f, - 0.2f, 1.0f, 0.0f,
        0.236f, 0.394f, - 0.2f, 0.0f, 1.0f,
        0.406f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_secVAO3);
	glBindVertexArray(l1_secVAO3);

	glGenBuffers(1, &l1_secVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l1_secVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_sec3), l1_sec3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_secEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_secEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l1_ms1[] = {
        0.406f, 0.231f, - 0.2f, 0.0f, 0.0f,
        0.491f, 0.231f, - 0.2f, 1.0f, 0.0f,
        0.406f, 0.394f, - 0.2f, 0.0f, 1.0f,
        0.491f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_msVAO1);
	glBindVertexArray(l1_msVAO1);

	glGenBuffers(1, &l1_msVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l1_msVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_ms1), l1_ms1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_msEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_msEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l1_ms2[] = {
        0.491f, 0.231f, - 0.2f, 0.0f, 0.0f,
        0.576f, 0.231f, - 0.2f, 1.0f, 0.0f,
        0.491f, 0.394f, - 0.2f, 0.0f, 1.0f,
        0.576f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_msVAO2);
	glBindVertexArray(l1_msVAO2);

	glGenBuffers(1, &l1_msVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l1_msVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_ms2), l1_ms2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_msEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_msEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l1_ms3[] = {
        0.576f, 0.231f, - 0.2f, 0.0f, 0.0f,
        0.746f, 0.231f, - 0.2f, 1.0f, 0.0f,
        0.576f, 0.394f, - 0.2f, 0.0f, 1.0f,
        0.746f, 0.394f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l1_msVAO3);
	glBindVertexArray(l1_msVAO3);

	glGenBuffers(1, &l1_msVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l1_msVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l1_ms3), l1_ms3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l1_msEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l1_msEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l2_min1[] = {
        -0.274f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        -0.189f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        -0.274f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        -0.189f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_minVAO1);
	glBindVertexArray(l2_minVAO1);

	glGenBuffers(1, &l2_minVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l2_minVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_min1), l2_min1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_minEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_minEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l2_min2[] = {
        -0.189f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        -0.104f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        -0.189f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        -0.104f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &l2_minVAO2);
	glBindVertexArray(l2_minVAO2);

	glGenBuffers(1, &l2_minVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l2_minVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_min2), l2_min2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_minEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_minEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l2_min3[] = {
        -0.104f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        0.066f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        -0.104f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        0.066f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_minVAO3);
	glBindVertexArray(l2_minVAO3);

	glGenBuffers(1, &l2_minVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l2_minVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_min3), l2_min3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_minEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_minEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l2_sec1[] = {
        0.066f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        0.151f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        0.066f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        0.151f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_secVAO1);
	glBindVertexArray(l2_secVAO1);

	glGenBuffers(1, &l2_secVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l2_secVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_sec1), l2_sec1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_secEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_secEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l2_sec2[] = {
        0.151f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        0.236f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        0.151f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        0.236f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_secVAO2);
	glBindVertexArray(l2_secVAO2);

	glGenBuffers(1, &l2_secVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l2_secVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_sec2), l2_sec2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_secEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_secEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l2_sec3[] = {
        0.236f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        0.406f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        0.236f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        0.406f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_secVAO3);
	glBindVertexArray(l2_secVAO3);

	glGenBuffers(1, &l2_secVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l2_secVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_sec3), l2_sec3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_secEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_secEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l2_ms1[] = {
        0.406f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        0.491f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        0.406f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        0.491f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_msVAO1);
	glBindVertexArray(l2_msVAO1);

	glGenBuffers(1, &l2_msVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l2_msVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_ms1), l2_ms1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_msEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_msEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l2_ms2[] = {
        0.491f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        0.576f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        0.491f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        0.576f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_msVAO2);
	glBindVertexArray(l2_msVAO2);

	glGenBuffers(1, &l2_msVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l2_msVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_ms2), l2_ms2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_msEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_msEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l2_ms3[] = {
        0.576f, 0.0717f, - 0.2f, 0.0f, 0.0f,
        0.746f, 0.0717f, - 0.2f, 1.0f, 0.0f,
        0.576f, 0.2347f, - 0.2f, 0.0f, 1.0f,
        0.746f, 0.2347f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l2_msVAO3);
	glBindVertexArray(l2_msVAO3);

	glGenBuffers(1, &l2_msVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l2_msVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l2_ms3), l2_ms3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l2_msEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l2_msEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	
    float l3_min1[] = {
        -0.274f, -0.110f, - 0.2f, 0.0f, 0.0f,
        -0.189f, -0.110f, - 0.2f, 1.0f, 0.0f,
        -0.274f, 0.073f, - 0.2f, 0.0f, 1.0f,
        -0.189f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_minVAO1);
	glBindVertexArray(l3_minVAO1);

	glGenBuffers(1, &l3_minVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l3_minVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_min1), l3_min1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_minEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_minEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l3_min2[] = {
        -0.189f, -0.110f, - 0.2f, 0.0f, 0.0f,
        -0.104f, -0.110f, - 0.2f, 1.0f, 0.0f,
        -0.189f, 0.073f, - 0.2f, 0.0f, 1.0f,
        -0.104f, 0.073f, - 0.2f, 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &l3_minVAO2);
	glBindVertexArray(l3_minVAO2);

	glGenBuffers(1, &l3_minVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l3_minVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_min2), l3_min2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_minEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_minEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l3_min3[] = {
        -0.104f, -0.110f, - 0.2f, 0.0f, 0.0f,
        0.066f, -0.110f, - 0.2f, 1.0f, 0.0f,
        -0.104f, 0.073f, - 0.2f, 0.0f, 1.0f,
        0.066f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_minVAO3);
	glBindVertexArray(l3_minVAO3);

	glGenBuffers(1, &l3_minVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l3_minVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_min3), l3_min3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_minEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_minEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l3_sec1[] = {
        0.066f, -0.110f, - 0.2f, 0.0f, 0.0f,
        0.151f, -0.110f, - 0.2f, 1.0f, 0.0f,
        0.066f, 0.073f, - 0.2f, 0.0f, 1.0f,
        0.151f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_secVAO1);
	glBindVertexArray(l3_secVAO1);

	glGenBuffers(1, &l3_secVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l3_secVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_sec1), l3_sec1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_secEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_secEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l3_sec2[] = {
        0.151f, -0.110f, - 0.2f, 0.0f, 0.0f,
        0.236f, -0.110f, - 0.2f, 1.0f, 0.0f,
        0.151f, 0.073f, - 0.2f, 0.0f, 1.0f,
        0.236f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_secVAO2);
	glBindVertexArray(l3_secVAO2);

	glGenBuffers(1, &l3_secVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l3_secVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_sec2), l3_sec2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_secEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_secEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l3_sec3[] = {
        0.236f, -0.110f, - 0.2f, 0.0f, 0.0f,
        0.406f, -0.110f, - 0.2f, 1.0f, 0.0f,
        0.236f, 0.073f, - 0.2f, 0.0f, 1.0f,
        0.406f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_secVAO3);
	glBindVertexArray(l3_secVAO3);

	glGenBuffers(1, &l3_secVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l3_secVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_sec3), l3_sec3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_secEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_secEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float l3_ms1[] = {
        0.406f, -0.110f, - 0.2f, 0.0f, 0.0f,
        0.491f, -0.110f, - 0.2f, 1.0f, 0.0f,
        0.406f, 0.073f, - 0.2f, 0.0f, 1.0f,
        0.491f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_msVAO1);
	glBindVertexArray(l3_msVAO1);

	glGenBuffers(1, &l3_msVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, l3_msVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_ms1), l3_ms1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_msEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_msEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l3_ms2[] = {
        0.491f, -0.110f, - 0.2f, 0.0f, 0.0f,
        0.576f, -0.110f, - 0.2f, 1.0f, 0.0f,
        0.491f, 0.073f, - 0.2f, 0.0f, 1.0f,
        0.576f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_msVAO2);
	glBindVertexArray(l3_msVAO2);

	glGenBuffers(1, &l3_msVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, l3_msVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_ms2), l3_ms2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_msEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_msEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float l3_ms3[] = {
        0.576f, -0.110f, - 0.2f, 0.0f, 0.0f,
        0.746f, -0.110f, - 0.2f, 1.0f, 0.0f,
        0.576f, 0.073f, - 0.2f, 0.0f, 1.0f,
        0.746f, 0.073f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &l3_msVAO3);
	glBindVertexArray(l3_msVAO3);

	glGenBuffers(1, &l3_msVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, l3_msVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(l3_ms3), l3_ms3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &l3_msEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l3_msEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float total_min1[] = {
        -0.274f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        -0.189f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        -0.274f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        -0.189f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_minVAO1);
	glBindVertexArray(total_minVAO1);

	glGenBuffers(1, &total_minVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, total_minVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_min1), total_min1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_minEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_minEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float total_min2[] = {
        -0.189f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        -0.104f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        -0.189f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        -0.104f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &total_minVAO2);
	glBindVertexArray(total_minVAO2);

	glGenBuffers(1, &total_minVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, total_minVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_min2), total_min2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_minEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_minEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float total_min3[] = {
        -0.104f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        0.066f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        -0.104f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        0.066f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_minVAO3);
	glBindVertexArray(total_minVAO3);

	glGenBuffers(1, &total_minVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, total_minVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_min3), total_min3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_minEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_minEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float total_sec1[] = {
        0.066f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        0.151f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        0.066f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        0.151f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_secVAO1);
	glBindVertexArray(total_secVAO1);

	glGenBuffers(1, &total_secVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, total_secVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_sec1), total_sec1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_secEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_secEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float total_sec2[] = {
        0.151f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        0.236f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        0.151f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        0.236f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_secVAO2);
	glBindVertexArray(total_secVAO2);

	glGenBuffers(1, &total_secVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, total_secVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_sec2), total_sec2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_secEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_secEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float total_sec3[] = {
        0.236f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        0.406f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        0.236f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        0.406f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_secVAO3);
	glBindVertexArray(total_secVAO3);

	glGenBuffers(1, &total_secVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, total_secVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_sec3), total_sec3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_secEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_secEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float total_ms1[] = {
        0.406f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        0.491f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        0.406f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        0.491f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_msVAO1);
	glBindVertexArray(total_msVAO1);

	glGenBuffers(1, &total_msVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, total_msVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_ms1), total_ms1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_msEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_msEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float total_ms2[] = {
        0.491f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        0.576f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        0.491f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        0.576f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_msVAO2);
	glBindVertexArray(total_msVAO2);

	glGenBuffers(1, &total_msVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, total_msVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_ms2), total_ms2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_msEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_msEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    float total_ms3[] = {
        0.576f, -0.27593f, - 0.2f, 0.0f, 0.0f,
        0.746f, -0.27593f, - 0.2f, 1.0f, 0.0f,
        0.576f, -0.11293f, - 0.2f, 0.0f, 1.0f,
        0.746f, -0.11293f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &total_msVAO3);
	glBindVertexArray(total_msVAO3);

	glGenBuffers(1, &total_msVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, total_msVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(total_ms3), total_ms3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &total_msEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, total_msEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float avg_d1[] = {
        -0.274f, -0.4662f, - 0.2f, 0.0f, 0.0f,
        -0.189f, -0.4662f, - 0.2f, 1.0f, 0.0f,
        -0.274f, -0.3032f, - 0.2f, 0.0f, 1.0f,
        -0.189f, -0.3032f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &avgVAO1);
	glBindVertexArray(avgVAO1);

	glGenBuffers(1, &avgVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, avgVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(avg_d1), avg_d1, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &avgEBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, avgEBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float avg_d2[] = {
        -0.189f, -0.4662f, - 0.2f, 0.0f, 0.0f,
        -0.104f, -0.4662f, - 0.2f, 1.0f, 0.0f,
        -0.189f, -0.3032f, - 0.2f, 0.0f, 1.0f,
        -0.104f, -0.3032f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &avgVAO2);
	glBindVertexArray(avgVAO2);

	glGenBuffers(1, &avgVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, avgVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(avg_d2), avg_d2, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &avgEBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, avgEBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float avg_d3[] = {
        -0.104f, -0.4662f, - 0.2f, 0.0f, 0.0f,
        -0.019f, -0.4662f, - 0.2f, 1.0f, 0.0f,
        -0.104f, -0.3032f, - 0.2f, 0.0f, 1.0f,
        -0.019f, -0.3032f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &avgVAO3);
	glBindVertexArray(avgVAO3);

	glGenBuffers(1, &avgVBO3);
	glBindBuffer(GL_ARRAY_BUFFER, avgVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(avg_d3), avg_d3, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &avgEBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, avgEBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
    
    float avg_d4[] = {
        -0.019f, -0.4662f, - 0.2f, 0.0f, 0.0f,
        0.181f, -0.4662f, - 0.2f, 1.0f, 0.0f,
        -0.019f, -0.3032f, - 0.2f, 0.0f, 1.0f,
        0.181f, -0.3032f, - 0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &avgVAO4);
	glBindVertexArray(avgVAO4);

	glGenBuffers(1, &avgVBO4);
	glBindBuffer(GL_ARRAY_BUFFER, avgVBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(avg_d4), avg_d4, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &avgEBO4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, avgEBO4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

    // use ui_shader
	glActiveTexture(GL_TEXTURE0);
	ui_shader->use();

	// render loop
	show_end_game_stats = true;

	while(show_end_game_stats)
	{
		check_events();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(user_actions.clicked_main_menu)
		{
			show_end_game_stats = false;
			
            glDeleteBuffers(1, &VBO1);
            glDeleteBuffers(1, &EBO1);
			glDeleteVertexArrays(1, &VAO1);
            
            glDeleteBuffers(1, &l1_minVBO1);
            glDeleteBuffers(1, &l1_minVBO2);
            glDeleteBuffers(1, &l1_minVBO3);
            glDeleteBuffers(1, &l1_secVBO1);
            glDeleteBuffers(1, &l1_secVBO2);
            glDeleteBuffers(1, &l1_secVBO3);
            glDeleteBuffers(1, &l1_msVBO1);
            glDeleteBuffers(1, &l1_msVBO2);
            glDeleteBuffers(1, &l1_msVBO3);

			glDeleteBuffers(1, &l1_minEBO1);
			glDeleteBuffers(1, &l1_minEBO2);
			glDeleteBuffers(1, &l1_minEBO3);
			glDeleteBuffers(1, &l1_secEBO1);
			glDeleteBuffers(1, &l1_secEBO2);
			glDeleteBuffers(1, &l1_secEBO3);
			glDeleteBuffers(1, &l1_msEBO1);
			glDeleteBuffers(1, &l1_msEBO2);
			glDeleteBuffers(1, &l1_msEBO3);

			glDeleteVertexArrays(1, &l1_minVAO1);
			glDeleteVertexArrays(1, &l1_minVAO2);
			glDeleteVertexArrays(1, &l1_minVAO3);
			glDeleteVertexArrays(1, &l1_secVAO1);
			glDeleteVertexArrays(1, &l1_secVAO2);
			glDeleteVertexArrays(1, &l1_secVAO3);
			glDeleteVertexArrays(1, &l1_msVAO1);
			glDeleteVertexArrays(1, &l1_msVAO2);
			glDeleteVertexArrays(1, &l1_msVAO3);

            glDeleteBuffers(1, &l2_minVBO1);
            glDeleteBuffers(1, &l2_minVBO2);
            glDeleteBuffers(1, &l2_minVBO3);
            glDeleteBuffers(1, &l2_secVBO1);
            glDeleteBuffers(1, &l2_secVBO2);
            glDeleteBuffers(1, &l2_secVBO3);
            glDeleteBuffers(1, &l2_msVBO1);
            glDeleteBuffers(1, &l2_msVBO2);
            glDeleteBuffers(1, &l2_msVBO3);

			glDeleteBuffers(1, &l2_minEBO1);
			glDeleteBuffers(1, &l2_minEBO2);
			glDeleteBuffers(1, &l2_minEBO3);
			glDeleteBuffers(1, &l2_secEBO1);
			glDeleteBuffers(1, &l2_secEBO2);
			glDeleteBuffers(1, &l2_secEBO3);
			glDeleteBuffers(1, &l2_msEBO1);
			glDeleteBuffers(1, &l2_msEBO2);
			glDeleteBuffers(1, &l2_msEBO3);

			glDeleteVertexArrays(1, &l2_minVAO1);
			glDeleteVertexArrays(1, &l2_minVAO2);
			glDeleteVertexArrays(1, &l2_minVAO3);
			glDeleteVertexArrays(1, &l2_secVAO1);
			glDeleteVertexArrays(1, &l2_secVAO2);
			glDeleteVertexArrays(1, &l2_secVAO3);
			glDeleteVertexArrays(1, &l2_msVAO1);
			glDeleteVertexArrays(1, &l2_msVAO2);
			glDeleteVertexArrays(1, &l2_msVAO3);
            
            glDeleteBuffers(1, &l3_minVBO1);
            glDeleteBuffers(1, &l3_minVBO2);
            glDeleteBuffers(1, &l3_minVBO3);
            glDeleteBuffers(1, &l3_secVBO1);
            glDeleteBuffers(1, &l3_secVBO2);
            glDeleteBuffers(1, &l3_secVBO3);
            glDeleteBuffers(1, &l3_msVBO1);
            glDeleteBuffers(1, &l3_msVBO2);
            glDeleteBuffers(1, &l3_msVBO3);

			glDeleteBuffers(1, &l3_minEBO1);
			glDeleteBuffers(1, &l3_minEBO2);
			glDeleteBuffers(1, &l3_minEBO3);
			glDeleteBuffers(1, &l3_secEBO1);
			glDeleteBuffers(1, &l3_secEBO2);
			glDeleteBuffers(1, &l3_secEBO3);
			glDeleteBuffers(1, &l3_msEBO1);
			glDeleteBuffers(1, &l3_msEBO2);
			glDeleteBuffers(1, &l3_msEBO3);

			glDeleteVertexArrays(1, &l3_minVAO1);
			glDeleteVertexArrays(1, &l3_minVAO2);
			glDeleteVertexArrays(1, &l3_minVAO3);
			glDeleteVertexArrays(1, &l3_secVAO1);
			glDeleteVertexArrays(1, &l3_secVAO2);
			glDeleteVertexArrays(1, &l3_secVAO3);
			glDeleteVertexArrays(1, &l3_msVAO1);
			glDeleteVertexArrays(1, &l3_msVAO2);
			glDeleteVertexArrays(1, &l3_msVAO3);
            
            glDeleteBuffers(1, &total_minVBO1);
            glDeleteBuffers(1, &total_minVBO2);
            glDeleteBuffers(1, &total_minVBO3);
            glDeleteBuffers(1, &total_secVBO1);
            glDeleteBuffers(1, &total_secVBO2);
            glDeleteBuffers(1, &total_secVBO3);
            glDeleteBuffers(1, &total_msVBO1);
            glDeleteBuffers(1, &total_msVBO2);
            glDeleteBuffers(1, &total_msVBO3);

			glDeleteBuffers(1, &total_minEBO1);
			glDeleteBuffers(1, &total_minEBO2);
			glDeleteBuffers(1, &total_minEBO3);
			glDeleteBuffers(1, &total_secEBO1);
			glDeleteBuffers(1, &total_secEBO2);
			glDeleteBuffers(1, &total_secEBO3);
			glDeleteBuffers(1, &total_msEBO1);
			glDeleteBuffers(1, &total_msEBO2);
			glDeleteBuffers(1, &total_msEBO3);

			glDeleteVertexArrays(1, &total_minVAO1);
			glDeleteVertexArrays(1, &total_minVAO2);
			glDeleteVertexArrays(1, &total_minVAO3);
			glDeleteVertexArrays(1, &total_secVAO1);
			glDeleteVertexArrays(1, &total_secVAO2);
			glDeleteVertexArrays(1, &total_secVAO3);
			glDeleteVertexArrays(1, &total_msVAO1);
			glDeleteVertexArrays(1, &total_msVAO2);
			glDeleteVertexArrays(1, &total_msVAO3);
            
            glDeleteBuffers(1, &avgVBO1);
            glDeleteBuffers(1, &avgVBO2);
            glDeleteBuffers(1, &avgVBO3);
            glDeleteBuffers(1, &avgVBO4);

			glDeleteBuffers(1, &avgEBO1);
			glDeleteBuffers(1, &avgEBO2);
			glDeleteBuffers(1, &avgEBO3);
			glDeleteBuffers(1, &avgEBO4);

			glDeleteVertexArrays(1, &avgVAO1);
			glDeleteVertexArrays(1, &avgVAO2);
			glDeleteVertexArrays(1, &avgVAO3);
			glDeleteVertexArrays(1, &avgVAO4);

			user_actions.clicked_main_menu = false;
		}

		// draw background image
		if(user_actions.mouseX >= bb_main_menu.top_left_x && user_actions.mouseX <= bb_main_menu.bottom_right_x && user_actions.mouseY >= bb_main_menu.top_left_y && user_actions.mouseY <= bb_main_menu.bottom_right_y)
		{
			glBindTexture(GL_TEXTURE_2D, menu_textures[65].id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, menu_textures[64].id);
		}
		glBindVertexArray(VAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // draw l1_min1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_min_d0].id);
		glBindVertexArray(l1_minVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_min2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_min_d1].id);
		glBindVertexArray(l1_minVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_min3
        glBindTexture(GL_TEXTURE_2D, menu_textures[66].id);
		glBindVertexArray(l1_minVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_sec1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_sec_d0].id);
		glBindVertexArray(l1_secVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_sec2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_sec_d1].id);
		glBindVertexArray(l1_secVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_sec3
        glBindTexture(GL_TEXTURE_2D, menu_textures[67].id);
		glBindVertexArray(l1_secVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_ms1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_ms_d0].id);
		glBindVertexArray(l1_msVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_ms2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_ms_d1].id);
		glBindVertexArray(l1_msVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l1_ms3
        glBindTexture(GL_TEXTURE_2D, menu_textures[68].id);
		glBindVertexArray(l1_msVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // draw l2_min1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_min_d0].id);
		glBindVertexArray(l2_minVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_min2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_min_d1].id);
		glBindVertexArray(l2_minVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_min3
        glBindTexture(GL_TEXTURE_2D, menu_textures[66].id);
		glBindVertexArray(l2_minVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_sec1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_sec_d0].id);
		glBindVertexArray(l2_secVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_sec2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_sec_d1].id);
		glBindVertexArray(l2_secVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_sec3
        glBindTexture(GL_TEXTURE_2D, menu_textures[67].id);
		glBindVertexArray(l2_secVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_ms1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_ms_d0].id);
		glBindVertexArray(l2_msVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_ms2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_ms_d1].id);
		glBindVertexArray(l2_msVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l2_ms3
        glBindTexture(GL_TEXTURE_2D, menu_textures[68].id);
		glBindVertexArray(l2_msVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // draw l3_min1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_min_d0].id);
		glBindVertexArray(l3_minVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_min2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_min_d1].id);
		glBindVertexArray(l3_minVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_min3
        glBindTexture(GL_TEXTURE_2D, menu_textures[66].id);
		glBindVertexArray(l3_minVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_sec1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_sec_d0].id);
		glBindVertexArray(l3_secVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_sec2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_sec_d1].id);
		glBindVertexArray(l3_secVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_sec3
        glBindTexture(GL_TEXTURE_2D, menu_textures[67].id);
		glBindVertexArray(l3_secVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_ms1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_ms_d0].id);
		glBindVertexArray(l3_msVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_ms2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_ms_d1].id);
		glBindVertexArray(l3_msVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw l3_ms3
        glBindTexture(GL_TEXTURE_2D, menu_textures[68].id);
		glBindVertexArray(l3_msVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_min1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + total_min_d0].id);
		glBindVertexArray(total_minVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_min2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + total_min_d1].id);
		glBindVertexArray(total_minVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_min3
        glBindTexture(GL_TEXTURE_2D, menu_textures[66].id);
		glBindVertexArray(total_minVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_sec1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + total_sec_d0].id);
		glBindVertexArray(total_secVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_sec2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + total_sec_d1].id);
		glBindVertexArray(total_secVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_sec3
        glBindTexture(GL_TEXTURE_2D, menu_textures[67].id);
		glBindVertexArray(total_secVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_ms1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + total_ms_d0].id);
		glBindVertexArray(total_msVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_ms2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + total_ms_d1].id);
		glBindVertexArray(total_msVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw total_ms3
        glBindTexture(GL_TEXTURE_2D, menu_textures[68].id);
		glBindVertexArray(total_msVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw avg1
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + ((avg_speed / 100) % 10)].id);
		glBindVertexArray(avgVAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw avg2
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + ((avg_speed / 10) % 10)].id);
		glBindVertexArray(avgVAO2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw avg3
        glBindTexture(GL_TEXTURE_2D, menu_textures[47 + (avg_speed % 10)].id);
		glBindVertexArray(avgVAO3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw avg4
        glBindTexture(GL_TEXTURE_2D, menu_textures[69].id);
		glBindVertexArray(avgVAO4);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
		SDL_GL_SwapWindow(window);
	}

	start();
}

void Game::play()
{
    // set to fullscreen
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    SDL_GetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	update_menu_bb(width, height);
    update_framebuffers();
    minimap->update_framebuffer(width, height);

    // hide cursor
    SDL_ShowCursor(SDL_DISABLE);
	
    // set current page
	current_page = RACE;

	// clear color
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);
	
	// gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);
	
	// set proper camera
	//cam = editor_cam;
	cam = racing_cam;

	// geometry
	int indices[] = {0, 1, 2, 2, 1, 3};

	// post process quad and its shadow & grey shaders
	float quad[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	
	Shader shadow_shader("../shaders/shadows/vertex.glsl", "../shaders/shadows/fragment.glsl", "../shaders/shadows/geometry.glsl");
	Shader grey_shader("../shaders/greyscale/vertex.glsl", "../shaders/greyscale/fragment.glsl", "../shaders/greyscale/geometry.glsl");
	Shader motionBlur_shader("../shaders/motion_blur/vertex.glsl", "../shaders/motion_blur/fragment.glsl", "../shaders/motion_blur/geometry.glsl");
	motionBlur_shader.use();
	motionBlur_shader.set_float("width", static_cast<float>(width));
	motionBlur_shader.set_float("height", static_cast<float>(height));
	Shader gaussian_blur_shader("../shaders/gaussian_blur/vertex.glsl", "../shaders/gaussian_blur/fragment.glsl", "../shaders/gaussian_blur/geometry.glsl");
	Shader color_shader("../shaders/color_pass/vertex.glsl", "../shaders/color_pass/fragment.glsl", "../shaders/color_pass/geometry.glsl");
	Shader final_shader("../shaders/final_pass/vertex.glsl", "../shaders/final_pass/fragment.glsl", "../shaders/final_pass/geometry.glsl");

	Shader countdown_shader("../shaders/countdown/vertex.glsl", "../shaders/countdown/fragment.glsl", "../shaders/countdown/geometry.glsl");
	countdown_shader.use();
	countdown_shader.set_float("alpha", 1.0f);
	countdown_shader.set_int("img", 0);
	
	// countdown
	float countdown[] =
	{
		-0.328f / 2.0, (-0.656f / 2.0) + 0.15f, -0.6f, 0.0f, 0.0f,
		0.328f / 2.0, (-0.656f / 2.0) + 0.15f, -0.6f, 1.0f, 0.0f,
		-0.328f / 2.0, (0.656f / 2.0) + 0.15f, -0.6f, 0.0f, 1.0f,
		0.328f / 2.0, (0.656f / 2.0) + 0.15f, -0.6f, 1.0f, 1.0f
	};
	
	GLuint countdownVAO, countdownVBO, countdownEBO;
	glGenVertexArrays(1, &countdownVAO);
	glBindVertexArray(countdownVAO);

	glGenBuffers(1, &countdownVBO);
	glBindBuffer(GL_ARRAY_BUFFER, countdownVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(countdown), &countdown, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &countdownEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, countdownEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// quit game pop-up
	float quit[] =
	{
		-0.3f, 0.0f, -0.15f, 0.0f, 0.0f,
		0.3f, 0.0f, -0.15f, 1.0f, 0.0f,
		-0.3f, 0.6f, -0.15f, 0.0f, 1.0f,
		0.3f, 0.6f, -0.15f, 1.0f, 1.0f
	};

	GLuint VAO1, VBO1, EBO1;
	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quit), &quit, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD speed
	float HUD_speed[] =
	{
		-1.0f, -1.0f, -0.045f, 0.0f, 0.0f,
		-0.5f, -1.0f, -0.045f, 1.0f, 0.0f,
		-1.0f, -0.5f, -0.045f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.045f, 1.0f, 1.0f
	};
	
	GLuint VAO2, VBO2, EBO2;
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);

	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_speed), &HUD_speed, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &EBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD speed bar
	float HUD_speed_bar[] =
	{
		-0.608f, -0.9488f, -0.05f, 0.0f, 0.0f,
		-0.548f, -0.9488f, -0.05f, 1.0f, 0.0f,
		-0.608f, -0.5488f, -0.05f, 0.0f, 1.0f,
		-0.548f, -0.5488f, -0.05f, 1.0f, 1.0f
	};
	
	GLuint VAO3, VBO3, EBO3;
	glGenVertexArrays(1, &VAO3);
	glBindVertexArray(VAO3);

	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_speed_bar), &HUD_speed_bar, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &EBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD speed digits
	float speed_digit0[] =
	{
		-0.897f, -0.847f, -0.05f, 0.0f, 0.0f,
		-0.812f, -0.847f, -0.05f, 1.0f, 0.0f,
		-0.897f, -0.677f, -0.05f, 0.0f, 1.0f,
		-0.812f, -0.677f, -0.05f, 1.0f, 1.0f
	};
	float speed_digit1[] =
	{
		-0.812f, -0.847f, -0.05f, 0.0f, 0.0f,
		-0.735f, -0.847f, -0.05f, 1.0f, 0.0f,
		-0.812f, -0.677f, -0.05f, 0.0f, 1.0f,
		-0.735f, -0.677f, -0.05f, 1.0f, 1.0f
	};
	float speed_digit2[] =
	{
		-0.735f, -0.847f, -0.05f, 0.0f, 0.0f,
		-0.655f, -0.847f, -0.05f, 1.0f, 0.0f,
		-0.735f, -0.677f, -0.05f, 0.0f, 1.0f,
		-0.655f, -0.677f, -0.05f, 1.0f, 1.0f
	};
	
	GLuint VAO4, VBO4, EBO4;
	GLuint VAO5, VBO5, EBO5;
	GLuint VAO6, VBO6, EBO6;

	glGenVertexArrays(1, &VAO4);
	glBindVertexArray(VAO4);
	glGenBuffers(1, &VBO4);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(speed_digit0), &speed_digit0, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &EBO4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	
	glGenVertexArrays(1, &VAO5);
	glBindVertexArray(VAO5);
	glGenBuffers(1, &VBO5);
	glBindBuffer(GL_ARRAY_BUFFER, VBO5);
	glBufferData(GL_ARRAY_BUFFER, sizeof(speed_digit1), &speed_digit1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &EBO5);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO5);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	
	glGenVertexArrays(1, &VAO6);
	glBindVertexArray(VAO6);
	glGenBuffers(1, &VBO6);
	glBindBuffer(GL_ARRAY_BUFFER, VBO6);
	glBufferData(GL_ARRAY_BUFFER, sizeof(speed_digit2), &speed_digit2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &EBO6);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO6);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	
	// HUD top bar
	float HUD_top_bar[] =
	{
		-1.0f, 0.6f, -0.5f, 0.0f, 0.0f,
		1.0f, 0.6f, -0.5f, 1.0f, 0.0f,
		-1.0f, 1.0f, -0.5f, 0.0f, 1.0f,
		1.0f, 1.0f, -0.5f, 1.0f, 1.0f
	};
	
	GLuint topBarVAO, topBarVBO, topBarEBO;
	glGenVertexArrays(1, &topBarVAO);
	glBindVertexArray(topBarVAO);

	glGenBuffers(1, &topBarVBO);
	glBindBuffer(GL_ARRAY_BUFFER, topBarVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_top_bar), &HUD_top_bar, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &topBarEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topBarEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD lap current
	float HUD_lap_current[] =
	{
		-0.847f, 0.817f, -0.55f, 0.0f, 0.0f,
		-0.777f, 0.817f, -0.55f, 1.0f, 0.0f,
		-0.847f, 0.99f, -0.55f, 0.0f, 1.0f,
		-0.777f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint currLapVAO, currLapVBO, currLapEBO;
	glGenVertexArrays(1, &currLapVAO);
	glBindVertexArray(currLapVAO);

	glGenBuffers(1, &currLapVBO);
	glBindBuffer(GL_ARRAY_BUFFER, currLapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lap_current), &HUD_lap_current, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &currLapEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currLapEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD lap slash
	float HUD_lap_slash[] =
	{
		-0.777f, 0.817f, -0.55f, 0.0f, 0.0f,
		-0.707f, 0.817f, -0.55f, 1.0f, 0.0f,
		-0.777f, 0.990f, -0.55f, 0.0f, 1.0f,
		-0.707f, 0.990f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint slashLapVAO, slashLapVBO, slashLapEBO;
	glGenVertexArrays(1, &slashLapVAO);
	glBindVertexArray(slashLapVAO);

	glGenBuffers(1, &slashLapVBO);
	glBindBuffer(GL_ARRAY_BUFFER, slashLapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lap_slash), &HUD_lap_slash, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &slashLapEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, slashLapEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD lap max
	float HUD_lap_max[] =
	{
		-0.707f, 0.817f, -0.55f, 0.0f, 0.0f,
		-0.637f, 0.817f, -0.55f, 1.0f, 0.0f,
		-0.707f, 0.99f, -0.55f, 0.0f, 1.0f,
		-0.637f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint maxLapVAO, maxLapVBO, maxLapEBO;
	glGenVertexArrays(1, &maxLapVAO);
	glBindVertexArray(maxLapVAO);

	glGenBuffers(1, &maxLapVBO);
	glBindBuffer(GL_ARRAY_BUFFER, maxLapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lap_max), &HUD_lap_max, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &maxLapEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, maxLapEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD pos
	float HUD_pos[] =
	{
		0.637f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.707f, 0.817f, -0.55f, 1.0f, 0.0f,
		0.637f, 0.99f, -0.55f, 0.0f, 1.0f,
		0.707f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint posVAO, posVBO, posEBO;
	glGenVertexArrays(1, &posVAO);
	glBindVertexArray(posVAO);

	glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_pos), &HUD_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &posEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, posEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD pos slash
	float HUD_pos_slash[] =
	{
		0.707f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.777f, 0.817f, -0.55f, 1.0f, 0.0f,
		0.707f, 0.990f, -0.55f, 0.0f, 1.0f,
		0.777f, 0.990f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint slashPosVAO, slashPosVBO, slashPosEBO;
	glGenVertexArrays(1, &slashPosVAO);
	glBindVertexArray(slashPosVAO);

	glGenBuffers(1, &slashPosVBO);
	glBindBuffer(GL_ARRAY_BUFFER, slashPosVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_pos_slash), &HUD_pos_slash, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &slashPosEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, slashPosEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD pos max
	float HUD_pos_max[] =
	{
		0.777f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.847f, 0.817f, -0.55f, 1.0f, 0.0f,
		0.777f, 0.99f, -0.55f, 0.0f, 1.0f,
		0.847f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint maxPosVAO, maxPosVBO, maxPosEBO;
	glGenVertexArrays(1, &maxPosVAO);
	glBindVertexArray(maxPosVAO);

	glGenBuffers(1, &maxPosVBO);
	glBindBuffer(GL_ARRAY_BUFFER, maxPosVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_pos_max), &HUD_pos_max, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &maxPosEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, maxPosEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD chrono min
	float HUD_chrono_min_d0[] =
	{
		-0.212f, 0.817f, -0.55f, 0.0f, 0.0f,
		-0.159f, 0.817f, -0.55f, 1.0f, 0.0f,
		-0.212f, 0.99f, -0.55f, 0.0f, 1.0f,
		-0.159f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chronoMinD0VAO, chronoMinD0VBO, chronoMinD0EBO;
	glGenVertexArrays(1, &chronoMinD0VAO);
	glBindVertexArray(chronoMinD0VAO);

	glGenBuffers(1, &chronoMinD0VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chronoMinD0VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_min_d0), &HUD_chrono_min_d0, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chronoMinD0EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chronoMinD0EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	float HUD_chrono_min_d1[] =
	{
		-0.159f, 0.817f, -0.55f, 0.0f, 0.0f,
		-0.106f, 0.817f, -0.55f, 1.0f, 0.0f,
		-0.159f, 0.99f, -0.55f, 0.0f, 1.0f,
		-0.106f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chronoMinD1VAO, chronoMinD1VBO, chronoMinD1EBO;
	glGenVertexArrays(1, &chronoMinD1VAO);
	glBindVertexArray(chronoMinD1VAO);

	glGenBuffers(1, &chronoMinD1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chronoMinD1VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_min_d1), &HUD_chrono_min_d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chronoMinD1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chronoMinD1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD chrono 2dots
	float HUD_chrono_2dots[] =
	{
		-0.106f, 0.817f, -0.55f, 0.0f, 0.0f,
		-0.053f, 0.817f, -0.55f, 1.0f, 0.0f,
		-0.106f, 0.99f, -0.55f, 0.0f, 1.0f,
		-0.053f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chrono2DotsVAO, chrono2DotsVBO, chrono2DotsEBO;
	glGenVertexArrays(1, &chrono2DotsVAO);
	glBindVertexArray(chrono2DotsVAO);

	glGenBuffers(1, &chrono2DotsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chrono2DotsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_2dots), &HUD_chrono_2dots, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chrono2DotsEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chrono2DotsEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD chrono sec d0
	float HUD_chrono_sec_d0[] =
	{
		-0.053f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.0f, 0.817f, -0.55f, 1.0f, 0.0f,
		-0.053f, 0.99f, -0.55f, 0.0f, 1.0f,
		0.0f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chronoSecD0VAO, chronoSecD0VBO, chronoSecD0EBO;
	glGenVertexArrays(1, &chronoSecD0VAO);
	glBindVertexArray(chronoSecD0VAO);

	glGenBuffers(1, &chronoSecD0VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chronoSecD0VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_sec_d0), &HUD_chrono_sec_d0, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chronoSecD0EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chronoSecD0EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD chrono sec d1
	float HUD_chrono_sec_d1[] =
	{
		0.0f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.053f, 0.817f, -0.55f, 1.0f, 0.0f,
		0.0f, 0.99f, -0.55f, 0.0f, 1.0f,
		0.053f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chronoSecD1VAO, chronoSecD1VBO, chronoSecD1EBO;
	glGenVertexArrays(1, &chronoSecD1VAO);
	glBindVertexArray(chronoSecD1VAO);

	glGenBuffers(1, &chronoSecD1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chronoSecD1VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_sec_d1), &HUD_chrono_sec_d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chronoSecD1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chronoSecD1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD chrono dot
	float HUD_chrono_dot[] =
	{
		0.053f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.106f, 0.817f, -0.55f, 1.0f, 0.0f,
		0.053f, 0.99f, -0.55f, 0.0f, 1.0f,
		0.106f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chronoDotVAO, chronoDotVBO, chronoDotEBO;
	glGenVertexArrays(1, &chronoDotVAO);
	glBindVertexArray(chronoDotVAO);

	glGenBuffers(1, &chronoDotVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chronoDotVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_dot), &HUD_chrono_dot, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chronoDotEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chronoDotEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD chrono ms d0
	float HUD_chrono_ms_d0[] =
	{
		0.106f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.159f, 0.817f, -0.55f, 1.0f, 0.0f,
		0.106f, 0.99f, -0.55f, 0.0f, 1.0f,
		0.159f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chronoMsD0VAO, chronoMsD0VBO, chronoMsD0EBO;
	glGenVertexArrays(1, &chronoMsD0VAO);
	glBindVertexArray(chronoMsD0VAO);

	glGenBuffers(1, &chronoMsD0VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chronoMsD0VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_ms_d0), &HUD_chrono_ms_d0, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chronoMsD0EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chronoMsD0EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD chrono ms d1
	float HUD_chrono_ms_d1[] =
	{
		0.159f, 0.817f, -0.55f, 0.0f, 0.0f,
		0.212f, 0.817f, -0.55f, 1.0f, 0.0f,
		0.159f, 0.99f, -0.55f, 0.0f, 1.0f,
		0.212f, 0.99f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint chronoMsD1VAO, chronoMsD1VBO, chronoMsD1EBO;
	glGenVertexArrays(1, &chronoMsD1VAO);
	glBindVertexArray(chronoMsD1VAO);

	glGenBuffers(1, &chronoMsD1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, chronoMsD1VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_chrono_ms_d1), &HUD_chrono_ms_d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &chronoMsD1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chronoMsD1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
    
    // HUD lap_time min
	float HUD_lapTime_min_d0[] =
	{
		-0.212f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		-0.159f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		-0.212f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		-0.159f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTimeMinD0VAO, lapTimeMinD0VBO, lapTimeMinD0EBO;
	glGenVertexArrays(1, &lapTimeMinD0VAO);
	glBindVertexArray(lapTimeMinD0VAO);

	glGenBuffers(1, &lapTimeMinD0VBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTimeMinD0VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_min_d0), &HUD_lapTime_min_d0, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTimeMinD0EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTimeMinD0EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	float HUD_lapTime_min_d1[] =
	{
		-0.159f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		-0.106f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		-0.159f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		-0.106f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTimeMinD1VAO, lapTimeMinD1VBO, lapTimeMinD1EBO;
	glGenVertexArrays(1, &lapTimeMinD1VAO);
	glBindVertexArray(lapTimeMinD1VAO);

	glGenBuffers(1, &lapTimeMinD1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTimeMinD1VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_min_d1), &HUD_lapTime_min_d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTimeMinD1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTimeMinD1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD lap_time 2dots
	float HUD_lapTime_2dots[] =
	{
		-0.106f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		-0.053f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		-0.106f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		-0.053f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTime2DotsVAO, lapTime2DotsVBO, lapTime2DotsEBO;
	glGenVertexArrays(1, &lapTime2DotsVAO);
	glBindVertexArray(lapTime2DotsVAO);

	glGenBuffers(1, &lapTime2DotsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTime2DotsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_2dots), &HUD_lapTime_2dots, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTime2DotsEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTime2DotsEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD lap_time sec d0
	float HUD_lapTime_sec_d0[] =
	{
		-0.053f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		0.0f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		-0.053f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		0.0f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTimeSecD0VAO, lapTimeSecD0VBO, lapTimeSecD0EBO;
	glGenVertexArrays(1, &lapTimeSecD0VAO);
	glBindVertexArray(lapTimeSecD0VAO);

	glGenBuffers(1, &lapTimeSecD0VBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTimeSecD0VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_sec_d0), &HUD_lapTime_sec_d0, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTimeSecD0EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTimeSecD0EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD lap_time sec d1
	float HUD_lapTime_sec_d1[] =
	{
		0.0f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		0.053f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		0.0f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		0.053f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTimeSecD1VAO, lapTimeSecD1VBO, lapTimeSecD1EBO;
	glGenVertexArrays(1, &lapTimeSecD1VAO);
	glBindVertexArray(lapTimeSecD1VAO);

	glGenBuffers(1, &lapTimeSecD1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTimeSecD1VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_sec_d1), &HUD_lapTime_sec_d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTimeSecD1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTimeSecD1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	
	// HUD lap_time dot
	float HUD_lapTime_dot[] =
	{
		0.053f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		0.106f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		0.053f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		0.106f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTimeDotVAO, lapTimeDotVBO, lapTimeDotEBO;
	glGenVertexArrays(1, &lapTimeDotVAO);
	glBindVertexArray(lapTimeDotVAO);

	glGenBuffers(1, &lapTimeDotVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTimeDotVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_dot), &HUD_lapTime_dot, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTimeDotEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTimeDotEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD lap_time ms d0
	float HUD_lapTime_ms_d0[] =
	{
		0.106f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		0.159f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		0.106f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		0.159f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTimeMsD0VAO, lapTimeMsD0VBO, lapTimeMsD0EBO;
	glGenVertexArrays(1, &lapTimeMsD0VAO);
	glBindVertexArray(lapTimeMsD0VAO);

	glGenBuffers(1, &lapTimeMsD0VBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTimeMsD0VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_ms_d0), &HUD_lapTime_ms_d0, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTimeMsD0EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTimeMsD0EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// HUD lap_time ms d1
	float HUD_lapTime_ms_d1[] =
	{
		0.159f, 0.817f - 0.45f, -0.55f, 0.0f, 0.0f,
		0.212f, 0.817f - 0.45f, -0.55f, 1.0f, 0.0f,
		0.159f, 0.99f - 0.45f, -0.55f, 0.0f, 1.0f,
		0.212f, 0.99f - 0.45f, -0.55f, 1.0f, 1.0f
	};
	
	GLuint lapTimeMsD1VAO, lapTimeMsD1VBO, lapTimeMsD1EBO;
	glGenVertexArrays(1, &lapTimeMsD1VAO);
	glBindVertexArray(lapTimeMsD1VAO);

	glGenBuffers(1, &lapTimeMsD1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, lapTimeMsD1VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HUD_lapTime_ms_d1), &HUD_lapTime_ms_d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &lapTimeMsD1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lapTimeMsD1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	// set HUD lap struct
	hud_lap.current = currLapVAO;
	hud_lap.slash = slashLapVAO;
	hud_lap.max = maxLapVAO;
	
	// set HUD pos struct
	hud_pos.pos = posVAO;
	hud_pos.slash = slashPosVAO;
	hud_pos.max = maxPosVAO;
	
	// set HUD chrono struct
	chrono.min_d0 = chronoMinD0VAO;
	chrono.min_d1 = chronoMinD1VAO;
	chrono.dots2 = chrono2DotsVAO;
	chrono.sec_d0 = chronoSecD0VAO;
	chrono.sec_d1 = chronoSecD1VAO;
	chrono.dot = chronoDotVAO;
	chrono.ms_d0 = chronoMsD0VAO;
	chrono.ms_d1 = chronoMsD1VAO;
	
    // set HUD lap_time struct
	lap_time.min_d0 = lapTimeMinD0VAO;
	lap_time.min_d1 = lapTimeMinD1VAO;
	lap_time.dots2 = lapTime2DotsVAO;
	lap_time.sec_d0 = lapTimeSecD0VAO;
	lap_time.sec_d1 = lapTimeSecD1VAO;
	lap_time.dot = lapTimeDotVAO;
	lap_time.ms_d0 = lapTimeMsD0VAO;
	lap_time.ms_d1 = lapTimeMsD1VAO;

	// set HUD speed struct
	hud_speed.speed_bar = VAO3;
	hud_speed.speed_bar_VBO = VBO3;
	hud_speed.layout = VAO2;
	hud_speed.d0 = VAO4;
	hud_speed.d1 = VAO5;
	hud_speed.d2 = VAO6;

	// store previous camera's view matrix
	bool first_loop = true;

	// delta time for render loop
	lastFrame = omp_get_wtime();
	accuDelta = 0.0;
	animRate = 1.0 / 24.0;

	// set again exit_game to false for some reason
	exit_game = false;

    // go
    in_racing_game = true;
    
	while(in_racing_game)
	{
		// clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
        // check if game is finished
        if(lap_iterate == 3)
        {
            in_racing_game = false;
        }

		// user input
		check_events();

		// delta calculation
		currentFrame = omp_get_wtime();
		delta = currentFrame - lastFrame;
		lastFrame = currentFrame;
		fps = 1.0 / delta;
        //std::cout << "fps = " << fps << std::endl;

		// timer
		if(!check_render_pass && countdown_timer < 0)
		{
			timer += delta;
			timer_min_d0 = static_cast<int>((timer / 60.0)) / 10;
			timer_min_d1 = static_cast<int>(timer / 60.0) % 10;
			if(timer_min_d1 > 9)
				timer_min_d1 = 9;
			timer_sec_d0 = (static_cast<int>(timer) - (static_cast<int>(timer / 60.0) * 60)) / 10;
			timer_sec_d1 = (static_cast<int>(timer) - (static_cast<int>(timer / 60.0) * 60)) % 10;
			timer_ms_d0 = static_cast<int>(timer * 10.0) % 10;
			timer_ms_d1 = static_cast<int>(timer * 100.0) % 10;
		}

        // process drawable meshes list
        draw_master->process_drawable_meshes_list(cam->get_position(), cam->get_direction(), cam->get_vector_right(), cam->get_vector_up());

		// shadowMap
		if(cast_shadows && ! print_quit_game && ! check_render_pass)
		{
			render_to_shadowMap();
			env->draw(true);
			pod->draw(true);
			render_to_shadowMap(true);
			env->draw(true);
			pod->draw(true);

			// default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		
		// draw post process quad and quit pop-up
		if(print_quit_game)
		{
			prepare_print_quit_game(VAO, VAO1, &grey_shader);
		}
		else
		{
			// get proper view matrix (bullet or camera based + world step sim)
			if(!check_render_pass)
				set_view_matrix(first_loop);

			// print quit game ?
			if(exit_game)
			{
                // show cursor
                SDL_ShowCursor(SDL_ENABLE);
				// show quit game
                quit_game(hud_speed, topBarVAO, hud_lap, hud_pos, chrono);
			}
			// draw racing game
			else
			{
                // hide cursor
                SDL_ShowCursor(SDL_DISABLE);
				
                // check render pass
				if(check_render_pass)
				{
					view_render_pass(VAO, color_shader);
					
					// =-=-=-=-= final pass =-=-=-=-=
					render_HUD(final_shader, VAO, hud_speed, topBarVAO, hud_lap, hud_pos, chrono, true);
				}
				else
				{
					// env pass
					render_env_texture();

					// smoke pass
					render_smoke_texture();

					// depth pass (for both previous passes)
					render_depth_texture();

					// env motion blur pass
					render_env_motion_blur_texture(VAO, motionBlur_shader);

					// smoke motion blur pass
					render_smoke_motion_blur_texture(VAO, motionBlur_shader);

					// podracer pass
					render_podracer();

					// podracer depth pass
					render_depth_texture(true);

					// gaussian blur pass on bright colors
					render_gaussian_blur_bright_colors(VAO, gaussian_blur_shader);

					// color pass
					render_color(VAO, color_shader);

					// =-=-=-=-= final pass =-=-=-=-=
					render_HUD(final_shader, VAO, hud_speed, topBarVAO, hud_lap, hud_pos, chrono);
		
					// show countdown animation
					if(countdown_timer >= 0)
						process_countdown(countdownVAO, static_cast<float>(delta), countdown_shader);
				}
			}
		}
		SDL_GL_SwapWindow(window);

        // update nb frames and accumulate pod speed
        nb_frames++;
        avg_speed += pod->speed;

		// sound system
		sound_system();

        // reset env drawable status
        env->reset_drawable();
	}

    // show cursor
    SDL_ShowCursor(SDL_ENABLE);

    // game is finished
    if(lap_iterate == 3)
    {
        // calculate average pod speed
        avg_speed = avg_speed / nb_frames;
	    end_game_stats();
    }
    else
    {
        reset();
        start();
    }
}

void Game::prepare_print_quit_game(GLuint VAO, GLuint VAO1, Shader* grey_shader)
{
	// post process quad
	glBindVertexArray(VAO);
	grey_shader->use();
	glActiveTexture(GL_TEXTURE0);
	grey_shader->set_int("img", 0);
	grey_shader->set_int("apply_greyScale", 1);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// quit pop-up
	if(user_actions.mouseX >= bb_quit_no.top_left_x && user_actions.mouseX <= bb_quit_no.bottom_right_x && user_actions.mouseY >= bb_quit_no.top_left_y && user_actions.mouseY <= bb_quit_no.bottom_right_y)
	{
		glBindTexture(GL_TEXTURE_2D, menu_textures[31].id);
	}
	else if(user_actions.mouseX >= bb_quit_yes.top_left_x && user_actions.mouseX <= bb_quit_yes.bottom_right_x && user_actions.mouseY >= bb_quit_yes.top_left_y && user_actions.mouseY <= bb_quit_yes.bottom_right_y)
	{
		glBindTexture(GL_TEXTURE_2D, menu_textures[32].id);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, menu_textures[30].id);
	}
	ui_shader->use();
	glBindVertexArray(VAO1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Game::quit_game(HUD_speed& hud_speed, GLuint topBarVAO, HUD_lap& hud_lap, HUD_pos& hud_pos, HUD_chrono& chrono)
{
	exit_game = false;
	print_quit_game = true;

	// color framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
	glEnable(GL_DEPTH);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
	// draw podracer
	pod->draw(false);

	// draw mos espa arena
	env->draw(false);

	// draw skybox
	glDepthFunc(GL_LEQUAL);
	sky->draw(cam->get_view(), cam->get_projection());
	glDepthFunc(GL_LESS);
				
	// draw speed HUD
	draw_speed_HUD(hud_speed);

	// draw top bar HUD
	draw_topBar_HUD(topBarVAO, hud_lap, hud_pos, chrono);

	// draw map HUD
	draw_map_HUD(true);

	// default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Game::set_view_matrix(bool & first_loop)
{
	if(!first_loop)
	{
        prev_camera_view = cam->get_view();
        tatooine->update_dynamics();
        cam->update_view(tatooine->chariot_model, tatooine->get_pod_direction(), this);
	}
	else
	{
        first_loop = false;
		prev_camera_view = cam->get_view();
        tatooine->update_dynamics();
        cam->update_view(tatooine->chariot_model, tatooine->get_pod_direction(), this);
	}
}

void Game::render_env_texture()
{
	// env framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, envFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);

	// draw mos espa arena
	env->draw(false);

	// draw skybox
	glDepthFunc(GL_LEQUAL);
	sky->draw(cam->get_view(), cam->get_projection());
	glDepthFunc(GL_LESS);
}

void Game::render_smoke_texture()
{
	// smoke framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, smokeFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);

	// draw podracer
	pod->draw(false, false, true);
}

void Game::render_depth_texture(bool podracerDepth)
{
	if(podracerDepth == false) // env and smoke depth textures
	{
		// depth framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glEnable(GL_DEPTH);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw mos espa arena
		env->draw(true, true);

		// draw skybox
		glDepthFunc(GL_LEQUAL);
		sky->draw(cam->get_view(), cam->get_projection());
		glDepthFunc(GL_LESS);

		// depth bis framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthBisFBO);
		glEnable(GL_DEPTH);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw podracer
		pod->draw(false, false, true);
	}
	else
	{
		// depth framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthPodFBO);
		glEnable(GL_DEPTH);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw podracer
		pod->draw(false, true, false);
	}
}

void Game::render_env_motion_blur_texture(GLuint VAO, Shader& motionBlur_shader)
{
	// env motion blur framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, envMotionBlurFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);

	// post process quad
	glBindVertexArray(VAO);
	motionBlur_shader.use();
	motionBlur_shader.set_int("env", 1);
	motionBlur_shader.set_int("smoke", 0);
	motionBlur_shader.set_float("pod_speed", static_cast<float>(pod->speed));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	motionBlur_shader.set_int("depthTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, envTexture);
	motionBlur_shader.set_int("colorTexture", 1);

	motionBlur_shader.set_float("fps", fps);

	motionBlur_shader.set_float("width", static_cast<float>(width));
	motionBlur_shader.set_float("height", static_cast<float>(height));
	//motionBlur_shader.set_Matrix("inv_MVP", glm::inverse(cam->get_projection() * cam->get_view() * cam->get_model()));
	
	glm::mat4 env_model = glm::mat4(1.0f);
	env_model = glm::scale(env_model, glm::vec3(1.0f, 1.0f, 1.0f));
	
	motionBlur_shader.set_vec3f("center_ray", cam->get_center_ray());
	motionBlur_shader.set_vec3f("cam_up", cam->get_vector_up());
	motionBlur_shader.set_vec3f("cam_right", cam->get_vector_right());
	motionBlur_shader.set_Matrix("inv_modelView", glm::inverse(cam->get_view()));
	motionBlur_shader.set_Matrix("prev_MVP", cam->get_projection() * prev_camera_view * cam->get_model());
	
	//motionBlur_shader.set_Matrix("prev_MVP", cam->get_projection() * cam->get_view() * cam->get_model());

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Game::render_smoke_motion_blur_texture(GLuint VAO, Shader& motionBlur_shader)
{
	// smoke motion blur framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, smokeMotionBlurFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);

	// post process quad
	glBindVertexArray(VAO);
	motionBlur_shader.use();
	motionBlur_shader.set_int("env", 0);
	motionBlur_shader.set_int("smoke", 1);
	motionBlur_shader.set_float("pod_speed", static_cast<float>(pod->speed));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthBisTexture);
	motionBlur_shader.set_int("depthTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, smokeTexture);
	motionBlur_shader.set_int("colorTexture", 1);

	motionBlur_shader.set_float("fps", fps);

	motionBlur_shader.set_float("width", static_cast<float>(width));
	motionBlur_shader.set_float("height", static_cast<float>(height));
	
	motionBlur_shader.set_vec3f("center_ray", cam->get_center_ray());
	motionBlur_shader.set_vec3f("cam_up", cam->get_vector_up());
	motionBlur_shader.set_vec3f("cam_right", cam->get_vector_right());
	
	motionBlur_shader.set_Matrix("inv_modelView", glm::inverse(cam->get_view()));
	motionBlur_shader.set_Matrix("prev_MVP", cam->get_projection() * prev_camera_view * cam->get_model());

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Game::render_podracer()
{
	// color framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, podracerFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);
	
	pod->draw(false, false, false);
}

void Game::render_gaussian_blur_bright_colors(GLuint VAO, Shader& gaussianBlurShader)
{
	bool horizontal = true;
	bool first_loop = true;
	int amount = 2;

	glBindVertexArray(VAO);
	gaussianBlurShader.use();
	glActiveTexture(GL_TEXTURE0);
	gaussianBlurShader.set_int("img", 0);

	for(int i = 0; i < amount; i++)
	{
		if(horizontal)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingFBO);
			glEnable(GL_DEPTH);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);
			gaussianBlurShader.set_int("horizontal", 1);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pongFBO);
			glEnable(GL_DEPTH);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);
			gaussianBlurShader.set_int("horizontal", 0);
		}
			glBindVertexArray(VAO);
			gaussianBlurShader.use();
			glActiveTexture(GL_TEXTURE0);
			gaussianBlurShader.set_int("img", 0);

		if(first_loop)
			glBindTexture(GL_TEXTURE_2D, smokeBrightMotionBlurTexture);
		else if(horizontal)
			glBindTexture(GL_TEXTURE_2D, pongTexture);
		else
			glBindTexture(GL_TEXTURE_2D, pingTexture);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		horizontal = !horizontal;
		if(first_loop)
			first_loop = false;
	}
}

void Game::render_color(GLuint VAO, Shader& color_shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);
	
	glBindVertexArray(VAO);
	color_shader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, envTexture);
	color_shader.set_int("env_texture", 0);
				
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, smokeTexture);
	color_shader.set_int("smoke_texture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	color_shader.set_int("envDepth_texture", 2);
				
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthBisTexture);
	color_shader.set_int("smokeDepth_texture", 3);
				
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, envMotionBlurTexture);
	color_shader.set_int("envMotionBlur", 4);
				
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, smokeMotionBlurTexture);
	color_shader.set_int("smokeMotionBlur", 5);
				
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, podracerTexture);
	color_shader.set_int("podracer_texture", 6);
	
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, podracerBrightTexture);
	color_shader.set_int("podracer_bright_texture", 7);
				
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, smokeBrightMotionBlurTexture);
	color_shader.set_int("smoke_bright_texture", 8);
	
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, depthPodTexture);
	color_shader.set_int("depthPod_texture", 9);

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, pongTexture);
	color_shader.set_int("gaussian_blur_smoke_bright_texture", 10);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Game::render_HUD(Shader& final_shader, GLuint VAO, HUD_speed& hud_speed, GLuint topBarVAO, HUD_lap& hud_lap, HUD_pos& hud_pos, HUD_chrono& chrono, bool disable_HUD)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH);
	glClearColor(0.325f, 0.25f, 0.25f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);

	glBindVertexArray(VAO);
	final_shader.use();
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	final_shader.set_int("img", 0);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	if(!disable_HUD)
	{
		// draw speed HUD
		draw_speed_HUD(hud_speed);

		// draw top bar HUD
		draw_topBar_HUD(topBarVAO, hud_lap, hud_pos, chrono);

		// draw map HUD
		draw_map_HUD();

        // draw lap time
        if(hit_count_lap_wall || display_lap_timer < 2.0)
        {
            draw_lap_time(lap_time);
            display_lap_timer -= delta;
            
            if(display_lap_timer < 0.0)
            {
                display_lap_timer = 2.0;
                delta_anim = 0.0f;
            }
        }
	}
}

void Game::view_render_pass(GLuint VAO, Shader& color_shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, colorFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(Color::LIGHT_GREY[0], Color::LIGHT_GREY[1], Color::LIGHT_GREY[2], Color::LIGHT_GREY[3]);
	
	glBindVertexArray(VAO);
	color_shader.use();
	color_shader.set_int("check_render_pass", 1);
	color_shader.set_int("r", render_pass);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, envTexture);
	color_shader.set_int("env_texture", 0);
				
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, smokeTexture);
	color_shader.set_int("smoke_texture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	color_shader.set_int("envDepth_texture", 2);
				
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthBisTexture);
	color_shader.set_int("smokeDepth_texture", 3);
				
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, envMotionBlurTexture);
	color_shader.set_int("envMotionBlur", 4);
				
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, smokeMotionBlurTexture);
	color_shader.set_int("smokeMotionBlur", 5);
				
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, podracerTexture);
	color_shader.set_int("podracer_texture", 6);
	
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, podracerBrightTexture);
	color_shader.set_int("podracer_bright_texture", 7);
				
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, smokeBrightMotionBlurTexture);
	color_shader.set_int("smoke_bright_texture", 8);
	
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, depthPodTexture);
	color_shader.set_int("depthPod_texture", 9);

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, pongTexture);
	color_shader.set_int("gaussian_blur_smoke_bright_texture", 10);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	color_shader.set_int("check_render_pass", 0);
}

void Game::process_countdown(GLuint countdownVAO, float delta, Shader& countdown_shader)
{
	// timer
	static float sec = 1.0f;

	// bind VAO
	glBindVertexArray(countdownVAO);

	// decrease sec
	sec -= delta;
	if(sec < 0.0f)
	{
		sec = 1.0f;
		countdown_timer--;
	}

	glActiveTexture(GL_TEXTURE0);

	if(countdown_timer == 3)
		glBindTexture(GL_TEXTURE_2D, menu_textures[60].id);
	else if(countdown_timer == 2)
		glBindTexture(GL_TEXTURE_2D, menu_textures[61].id);
	else if(countdown_timer == 1)
		glBindTexture(GL_TEXTURE_2D, menu_textures[62].id);
	else
		glBindTexture(GL_TEXTURE_2D, menu_textures[63].id);
		
	countdown_shader.set_int("img", 0);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Game::render_to_shadowMap(bool podShadowMap)
{
    if(current_page == POD_SPECS)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	    glClear(GL_DEPTH_BUFFER_BIT);
    }

	// FBO
    if(!podShadowMap)
    {
	    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	    glClear(GL_DEPTH_BUFFER_BIT);

        process_pod_shadowPass = false;
    }
    else
    {
	    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	    glBindFramebuffer(GL_FRAMEBUFFER, shadowMap2FBO);
	    glClear(GL_DEPTH_BUFFER_BIT);
        
        process_pod_shadowPass = true;
    }
}

void Game::draw_speed_HUD(HUD_speed& hud_speed)
{
	// HUD
	ui_shader->use();
	ui_shader->set_float("alpha", 1.0f);
				
	//draw HUD speed
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, menu_textures[33].id);
	glBindVertexArray(hud_speed.layout);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				
	//draw HUD speed bar
	int pod_speed = static_cast<int>(pod->speed);
	float speed_bar_offset = pod->speed / 850.0f;
	float HUD_speed_bar_update[] =
	{
		-0.608f, -0.9488f, -0.05f, 0.0f, 0.0f,
		-0.548f, -0.9488f, -0.05f, 1.0f, 0.0f,
		-0.608f, -0.9488f + (0.4f * speed_bar_offset), -0.05f, 0.0f, speed_bar_offset,
		-0.548f, -0.9488f + (0.4f * speed_bar_offset), -0.05f, 1.0f, speed_bar_offset
	};
	glBindBuffer(GL_ARRAY_BUFFER, hud_speed.speed_bar_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(HUD_speed_bar_update), HUD_speed_bar_update);
	glBindTexture(GL_TEXTURE_2D, menu_textures[34].id);
	glBindVertexArray(hud_speed.speed_bar);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				
	// draw speed digits
	int digit_left = pod_speed / 100;
	int digit_mid = (pod_speed / 10) % 10;
	int digit_right = pod_speed % 10;
				
	if(digit_left != 0)
	{
		glBindTexture(GL_TEXTURE_2D, menu_textures[35 + digit_left].id);
		glBindVertexArray(hud_speed.d0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				
		glBindTexture(GL_TEXTURE_2D, menu_textures[35 + digit_mid].id);
		glBindVertexArray(hud_speed.d1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				
		glBindTexture(GL_TEXTURE_2D, menu_textures[35 + digit_right].id);
		glBindVertexArray(hud_speed.d2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	else if(digit_mid != 0)
	{
		glBindTexture(GL_TEXTURE_2D, menu_textures[35 + digit_mid].id);
		glBindVertexArray(hud_speed.d1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				
		glBindTexture(GL_TEXTURE_2D, menu_textures[35 + digit_right].id);
		glBindVertexArray(hud_speed.d2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	else if(digit_right != 0)
	{
		glBindTexture(GL_TEXTURE_2D, menu_textures[35 + digit_right].id);
		glBindVertexArray(hud_speed.d2);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
}

void Game::draw_topBar_HUD(GLuint topBarVAO, HUD_lap& hud_lap, HUD_pos& hud_pos, HUD_chrono& chrono)
{
	// HUD
	ui_shader->use();
	ui_shader->set_float("alpha", 1.0f);
				
	//draw HUD top bar
	glBindTexture(GL_TEXTURE_2D, menu_textures[46].id);
	glBindVertexArray(topBarVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// draw HUD current lap
    if(hit_count_lap_wall && lap_iterate < 3)
    {
	    glBindTexture(GL_TEXTURE_2D, menu_textures[48 + lap_iterate].id);
    }
    else
    {
        if(lap_iterate == -1)
	        glBindTexture(GL_TEXTURE_2D, menu_textures[48].id);
        else
        {
            if(lap_iterate == 3)
	            glBindTexture(GL_TEXTURE_2D, menu_textures[48 + lap_iterate - 1].id);
            else
	            glBindTexture(GL_TEXTURE_2D, menu_textures[48 + lap_iterate].id);
        }
    }
	glBindVertexArray(hud_lap.current);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD max lap
	glBindTexture(GL_TEXTURE_2D, menu_textures[50].id);
	glBindVertexArray(hud_lap.max);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD slash lap
	glBindTexture(GL_TEXTURE_2D, menu_textures[58].id);
	glBindVertexArray(hud_lap.slash);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD pos
	glBindTexture(GL_TEXTURE_2D, menu_textures[48].id);
	glBindVertexArray(hud_pos.pos);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD slash pos
	glBindTexture(GL_TEXTURE_2D, menu_textures[58].id);
	glBindVertexArray(hud_pos.slash);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD max pos
	glBindTexture(GL_TEXTURE_2D, menu_textures[48].id);
	glBindVertexArray(hud_pos.max);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// draw HUD chrono min d0
	glBindTexture(GL_TEXTURE_2D, menu_textures[47 + timer_min_d0].id);
	glBindVertexArray(chrono.min_d0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD chrono min d1
	glBindTexture(GL_TEXTURE_2D, menu_textures[47 + timer_min_d1].id);
	glBindVertexArray(chrono.min_d1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD chrono 2 dots
	glBindTexture(GL_TEXTURE_2D, menu_textures[57].id);
	glBindVertexArray(chrono.dots2);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD chrono sec d0
	glBindTexture(GL_TEXTURE_2D, menu_textures[47 + timer_sec_d0].id);
	glBindVertexArray(chrono.sec_d0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD chrono sec d1
	glBindTexture(GL_TEXTURE_2D, menu_textures[47 + timer_sec_d1].id);
	glBindVertexArray(chrono.sec_d1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD chrono dot
	glBindTexture(GL_TEXTURE_2D, menu_textures[59].id);
	glBindVertexArray(chrono.dot);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD chrono ms d0
	glBindTexture(GL_TEXTURE_2D, menu_textures[47 + timer_ms_d0].id);
	glBindVertexArray(chrono.ms_d0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// draw HUD chrono ms d1
	glBindTexture(GL_TEXTURE_2D, menu_textures[47 + timer_ms_d1].id);
	glBindVertexArray(chrono.ms_d1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Game::draw_map_HUD(bool quit_game)
{
	minimap->draw(quit_game);
}

void Game::draw_lap_time(HUD_chrono& lap_time)
{
    static bool lap1_done = false;
    static bool lap2_done = false;

    // set lap1 & lap2 & lap3 timers value
    if(lap_iterate == 1 && !lap1_done)
    {
        timer1 = timer;
        
        lap1_done = true;
        lap1_min_d0 = timer_min_d0;
        lap1_min_d1 = timer_min_d1;
        lap1_sec_d0 = timer_sec_d0;
        lap1_sec_d1 = timer_sec_d1;
        lap1_ms_d0 = timer_ms_d0;
        lap1_ms_d1 = timer_ms_d1;
    }
    else if(lap_iterate == 2 && !lap2_done)
    {
        timer2 = timer - timer1;

        lap2_min_d0 = static_cast<int>((timer2 / 60.0)) / 10;
		lap2_min_d1 = static_cast<int>(timer2 / 60.0) % 10;
		if(lap2_min_d1 > 9)
			lap2_min_d1 = 9;
		lap2_sec_d0 = (static_cast<int>(timer2) - (static_cast<int>(timer2 / 60.0) * 60)) / 10;
		lap2_sec_d1 = (static_cast<int>(timer2) - (static_cast<int>(timer2 / 60.0) * 60)) % 10;
		lap2_ms_d0 = static_cast<int>(timer2 * 10.0) % 10;
		lap2_ms_d1 = static_cast<int>(timer2 * 100.0) % 10;
        
        lap2_done = true;
    }
    else if(lap_iterate == 3)
    {
        timer3 = timer - (timer1 + timer2);
        
        lap3_min_d0 = static_cast<int>((timer3 / 60.0)) / 10;
		lap3_min_d1 = static_cast<int>(timer3 / 60.0) % 10;
		if(lap3_min_d1 > 9)
			lap3_min_d1 = 9;
		lap3_sec_d0 = (static_cast<int>(timer3) - (static_cast<int>(timer3 / 60.0) * 60)) / 10;
		lap3_sec_d1 = (static_cast<int>(timer3) - (static_cast<int>(timer3 / 60.0) * 60)) % 10;
		lap3_ms_d0 = static_cast<int>(timer3 * 10.0) % 10;
		lap3_ms_d1 = static_cast<int>(timer3 * 100.0) % 10;
        
        total_min_d0 = static_cast<int>((timer / 60.0)) / 10;
		total_min_d1 = static_cast<int>(timer / 60.0) % 10;
		if(total_min_d1 > 9)
			total_min_d1 = 9;
		total_sec_d0 = (static_cast<int>(timer) - (static_cast<int>(timer / 60.0) * 60)) / 10;
		total_sec_d1 = (static_cast<int>(timer) - (static_cast<int>(timer / 60.0) * 60)) % 10;
		total_ms_d0 = static_cast<int>(timer * 10.0) % 10;
		total_ms_d1 = static_cast<int>(timer * 100.0) % 10;
        
        lap1_done = false;
        lap2_done = false;
    }

	// HUD
	ui_shader->use();
	ui_shader->set_float("alpha", 1.0f);
	ui_shader->set_int("lap_timer_anim", 1);
    delta_anim += 0.3f;
	ui_shader->set_float("delta_anim", delta_anim);

    if(lap_iterate == 1)
    {
	    // draw HUD lap_time min d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_min_d0].id);
	    glBindVertexArray(lap_time.min_d0);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time min d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_min_d1].id);
	    glBindVertexArray(lap_time.min_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time 2 dots
	    glBindTexture(GL_TEXTURE_2D, menu_textures[57].id);
	    glBindVertexArray(lap_time.dots2);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time sec d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_sec_d0].id);
	    glBindVertexArray(lap_time.sec_d0);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time sec d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_sec_d1].id);
	    glBindVertexArray(lap_time.sec_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time dot
	    glBindTexture(GL_TEXTURE_2D, menu_textures[59].id);
	    glBindVertexArray(lap_time.dot);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time ms d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_ms_d0].id);
	    glBindVertexArray(lap_time.ms_d0);
    	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time ms d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap1_ms_d1].id);
	    glBindVertexArray(lap_time.ms_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    else if(lap_iterate == 2)
    {
	    // draw HUD lap_time min d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_min_d0].id);
	    glBindVertexArray(lap_time.min_d0);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time min d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_min_d1].id);
	    glBindVertexArray(lap_time.min_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time 2 dots
	    glBindTexture(GL_TEXTURE_2D, menu_textures[57].id);
	    glBindVertexArray(lap_time.dots2);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time sec d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_sec_d0].id);
	    glBindVertexArray(lap_time.sec_d0);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time sec d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_sec_d1].id);
	    glBindVertexArray(lap_time.sec_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time dot
	    glBindTexture(GL_TEXTURE_2D, menu_textures[59].id);
	    glBindVertexArray(lap_time.dot);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time ms d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_ms_d0].id);
	    glBindVertexArray(lap_time.ms_d0);
    	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time ms d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap2_ms_d1].id);
	    glBindVertexArray(lap_time.ms_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    else if(lap_iterate == 3)
    {
	    // draw HUD lap_time min d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_min_d0].id);
	    glBindVertexArray(lap_time.min_d0);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time min d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_min_d1].id);
	    glBindVertexArray(lap_time.min_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time 2 dots
	    glBindTexture(GL_TEXTURE_2D, menu_textures[57].id);
	    glBindVertexArray(lap_time.dots2);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time sec d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_sec_d0].id);
	    glBindVertexArray(lap_time.sec_d0);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time sec d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_sec_d1].id);
	    glBindVertexArray(lap_time.sec_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time dot
	    glBindTexture(GL_TEXTURE_2D, menu_textures[59].id);
	    glBindVertexArray(lap_time.dot);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time ms d0
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_ms_d0].id);
	    glBindVertexArray(lap_time.ms_d0);
    	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	    // draw HUD lap_time ms d1
	    glBindTexture(GL_TEXTURE_2D, menu_textures[47 + lap3_ms_d1].id);
	    glBindVertexArray(lap_time.ms_d1);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
	ui_shader->set_int("lap_timer_anim", 0);
}

void Game::reset()
{
	racing_cam->reset();
	editor_cam->reset();
	pod_specs_cam->reset();
	pod->reset();
	env->reset();
    tatooine->reset();

	timer = 0.0;
	timer1 = 0.0;
	timer2 = 0.0;
	timer3 = 0.0;
	timer_min_d0 = 0;
	timer_min_d1 = 0;
	timer_sec_d0 = 0;
	timer_sec_d1 = 0;
	timer_ms_d0 = 0;
	timer_ms_d1 = 0;

	countdown_timer = 3;
    hit_count_lap_wall = false;
    lap_iterate = -1;
    lap1_min_d0 = 0;
    lap1_min_d1 = 0;
    lap1_sec_d0 = 0;
    lap1_sec_d1 = 0;
    lap1_ms_d0 = 0;
    lap1_ms_d1 = 0;
    lap2_min_d0 = 0;
    lap2_min_d1 = 0;
    lap2_sec_d0 = 0;
    lap2_sec_d1 = 0;
    lap2_ms_d0 = 0;
    lap2_ms_d1 = 0;
    lap3_min_d0 = 0;
    lap3_min_d1 = 0;
    lap3_sec_d0 = 0;
    lap3_sec_d1 = 0;
    lap3_ms_d0 = 0;
    lap3_ms_d1 = 0;
    total_min_d0 = 0;
    total_min_d1 = 0;
    total_sec_d0 = 0;
    total_sec_d1 = 0;
    total_ms_d0 = 0;
    total_ms_d1 = 0;
    avg_speed = 0;
    nb_frames = 0;
    display_lap_timer = 2.0;

    exit_game = false;
    print_quit_game = false;

	check_render_pass = false;
	render_pass = 0;
}

void Game::sound_system()
{
	static bool fire_power_coupling_done = false;
	static bool power_coupling_running = false;
	static bool start_electric_engine_done = false;
	static bool electric_engine_running = false;
	static bool turbojet_fired = false;
	static bool turbojet_running = false;
	static bool last_action_break_engine = false;
	static bool last_action_afterburn = false;
	static int bip = 0;
    static bool go_sound = false;
    static bool new_lap_record = false;
    static bool pod_crash_done = false;

	if(current_page != RACE)
	{
		fire_power_coupling_done = false;
		power_coupling_running = false;
		start_electric_engine_done = false;
		electric_engine_running = false;
		turbojet_fired = false;
		turbojet_running = false;
		last_action_break_engine = false;
		last_action_afterburn = false;
		bip = 0;
        go_sound = false;
        new_lap_record = false;
        pod_crash_done = false;

		if(tatooine_wind->is_playing())
			tatooine_wind->stop_sound();
		if(pod_base_engine->is_playing())
			pod_base_engine->stop_sound();
		if(pod_electric_engine->is_playing())
			pod_electric_engine->stop_sound();
		if(pod_power_coupling->is_playing())
			pod_power_coupling->stop_sound();

		if(!main_menu_source->is_playing())
			main_menu_source->play_sound(sounds->sounds.at(0));
	}
	else
	{
		if(main_menu_source->is_playing())
			main_menu_source->stop_sound();

		if(countdown_timer > 0 && bip < 3)
		{
			if(!countdown_sounds->is_playing())
			{
				bip++;
				countdown_sounds->play_sound(sounds->sounds.at(10));
			}
		}
		else if(countdown_timer == 0 && !go_sound)
		{
			if(!countdown_sounds->is_playing())
            {
				countdown_sounds->play_sound(sounds->sounds.at(11));
                go_sound = true;
            }
		}

		if(pod->power_coupling_on)
		{
			if(!pod_fire_power_coupling->is_playing() && !fire_power_coupling_done)
			{
				pod_fire_power_coupling->play_sound(sounds->sounds.at(1));
				fire_power_coupling_done = true;
			}
			if((pod_fire_power_coupling->get_elapsed_time() >= 0.5f) && fire_power_coupling_done && !power_coupling_running)
			{
				power_coupling_running = true;
				pod_power_coupling->play_sound(sounds->sounds.at(2));
			}
		}

		if(pod->electric_engine_on)
		{
			if(!pod_start_electric_engine->is_playing() && !start_electric_engine_done)
			{
				pod_start_electric_engine->play_sound(sounds->sounds.at(3));
				start_electric_engine_done = true;
			}
			if((pod_start_electric_engine->get_elapsed_time() >= 3.3f) && start_electric_engine_done && !electric_engine_running)
			{
				electric_engine_running = true;
				pod_electric_engine->play_sound(sounds->sounds.at(4));
			}
		}

		if(pod->turbojet_on)
		{
			if(!turbojet_fired)
			{
				turbojet_fired = true;
				pod_fire_engine->play_sound(sounds->sounds.at(5));
			}
			else if(!turbojet_running)
			{
				turbojet_running = true;
				pod_base_engine->play_sound(sounds->sounds.at(6));
			}

			if(user_actions.key_up || user_actions.key_space)
			{
				last_action_break_engine = false;
			}

			if(user_actions.key_up || user_actions.key_down || user_actions.key_right || user_actions.key_left)
			{
				last_action_afterburn = false;
			}

			if(user_actions.key_down)
			{
				if(!pod_break->is_playing() && !last_action_break_engine)
				{
					pod_base_engine->set_volume(10);
					pod_break->play_sound(sounds->sounds.at(7));
				}
				last_action_break_engine = true;
			}
			else
				pod_base_engine->set_volume(30);

			if(user_actions.key_space)
			{
				pod_base_engine->set_volume(80);
				if(!pod_afterburn->is_playing() && !last_action_afterburn)
				{
					last_action_afterburn = true;
					pod_afterburn->play_sound(sounds->sounds.at(9));
				}
			}
			else if(!user_actions.key_down)
				pod_base_engine->set_volume(30);
		}

        if(hit_count_lap_wall && !fodesinbeed->is_playing() && (lap_iterate > 0))
        {
            new_lap_record = true;
            fodesinbeed->play_sound(sounds->sounds.at(8));
        }

        if(pod->collide_terrain && !pod_collide->is_playing())
        {
            if(pod->speed > 0.1f)
            {
                pod_collide->play_sound(sounds->sounds.at(12));
            }
            pod->collide_terrain = false;
        }
        
        if(pod->collide_ground && !pod_crash->is_playing() && !pod_crash_done)
        {
            if(pod->speed > 0.1f)
            {
                pod_crash->play_sound(sounds->sounds.at(13));
                pod->collide_ground = false;
                pod_crash_done = true;
            }
        }
	}
}

void Game::check_events()
{
	keyboardState = SDL_GetKeyboardState(nullptr);
	mouseButtonBitMask = SDL_GetRelativeMouseState(&mouseX_rel, &mouseY_rel);

	while(SDL_PollEvent(&event))
	{
		if(event.type == SDL_QUIT)
		{
			if(show_menu)
				show_menu = false;
			if(in_racing_game)
				in_racing_game = false;
		}
		if(event.type == SDL_WINDOWEVENT)
		{
			if(event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				width = event.window.data1;
				height = event.window.data2;
                int w, h;
                SDL_GetWindowMaximumSize(window, &w, &h);
                SDL_SetWindowSize(window, width, height);
				glViewport(0, 0, width, height);
				update_menu_bb(width, height);
                update_framebuffers();
                if(!in_racing_game)
                {
                    menu_width = width;
                    menu_height = height;
                    if(width == w && height == h)
                        menu_fullscreen = true;
                    else
                        menu_fullscreen = false;
                }
			}
		}
		if(event.type == SDL_MOUSEBUTTONUP && user_actions.mouse_left_down)
		{
			user_actions.mouse_left_down = false;
			user_actions.slide_volume = false;
		}
#if CHECK_RENDER_PASS == 1
		if(in_racing_game && ! print_quit_game && event.type == SDL_MOUSEBUTTONDOWN)
		{
			if(SDL_BUTTON(mouseButtonBitMask) == SDL_BUTTON_LEFT)
			{
				if(check_render_pass)
				{
					render_pass++;
					if(render_pass == 12)
						render_pass = 0;
				}
			}
		}
#endif
		if(in_racing_game && print_quit_game && event.type == SDL_MOUSEBUTTONDOWN)
		{
			if(event.button.x >= bb_quit_no.top_left_x && event.button.x <= bb_quit_no.bottom_right_x && event.button.y >= bb_quit_no.top_left_y && event.button.y <= bb_quit_no.bottom_right_y)
			{
				print_quit_game = false;
			}
			if(event.button.x >= bb_quit_yes.top_left_x && event.button.x <= bb_quit_yes.bottom_right_x && event.button.y >= bb_quit_yes.top_left_y && event.button.y <= bb_quit_yes.bottom_right_y)
			{
				print_quit_game = false;
                in_racing_game = false;
			}
		}
		if((show_menu || show_info_menu || show_map || show_controls || show_pod_specs || show_tuning || show_end_game_stats) && event.type == SDL_MOUSEBUTTONDOWN)
		{
			user_actions.mouse_left_down = true;
			if(current_page == MAIN)
			{
				if(event.button.x >= bb_play.top_left_x && event.button.x <= bb_play.bottom_right_x && event.button.y >= bb_play.top_left_y && event.button.y <= bb_play.bottom_right_y)
					user_actions.clicked_play = true;
				else if(event.button.x >= bb_infos.top_left_x && event.button.x <= bb_infos.bottom_right_x && event.button.y >= bb_infos.top_left_y && event.button.y <= bb_infos.bottom_right_y)
					user_actions.clicked_infos = true;
				else if(event.button.x >= bb_pod_specs.top_left_x && event.button.x <= bb_pod_specs.bottom_right_x && event.button.y >= bb_pod_specs.top_left_y && event.button.y <= bb_pod_specs.bottom_right_y)
					user_actions.clicked_pod_specs = true;
				else if(event.button.x >= bb_quit.top_left_x && event.button.x <= bb_quit.bottom_right_x && event.button.y >= bb_quit.top_left_y && event.button.y <= bb_quit.bottom_right_y)
					user_actions.clicked_quit = true;
			}
			if(current_page == INFOS)
			{
				if(event.button.x >= bb_map.top_left_x && event.button.x <= bb_map.bottom_right_x && event.button.y >= bb_map.top_left_y && event.button.y <= bb_map.bottom_right_y)
					user_actions.clicked_map = true;
				else if(event.button.x >= bb_controls.top_left_x && event.button.x <= bb_controls.bottom_right_x && event.button.y >= bb_controls.top_left_y && event.button.y <= bb_controls.bottom_right_y)
					user_actions.clicked_controls = true;
			}
			if(current_page == TUNING)
			{
				if(event.button.x >= bb_close_tuning_window.top_left_x && event.button.x <= bb_close_tuning_window.bottom_right_x && event.button.y >= bb_close_tuning_window.top_left_y && event.button.y <= bb_close_tuning_window.bottom_right_y)
				{
					show_tuning = false;
				}
				if(event.button.x >= bb_cast_shadows.top_left_x && event.button.x <= bb_cast_shadows.bottom_right_x && event.button.y >= bb_cast_shadows.top_left_y && event.button.y <= bb_cast_shadows.bottom_right_y)
				{
					if(cast_shadows)
						cast_shadows = false;
					else
						cast_shadows = true;
				}
				if(event.button.x >= bb_sound_slider.top_left_x && event.button.x <= bb_sound_slider.bottom_right_x && event.button.y >= bb_sound_slider.top_left_y && event.button.y <= bb_sound_slider.bottom_right_y)
				{
					user_actions.slide_volume = true;
				}
			}
            if(current_page == END_GAME_STATS)
            {
				if(event.button.x >= bb_main_menu.top_left_x && event.button.x <= bb_main_menu.bottom_right_x && event.button.y >= bb_main_menu.top_left_y && event.button.y <= bb_main_menu.bottom_right_y)
					user_actions.clicked_main_menu = true;
            }
			if(event.button.x >= bb_tuning.top_left_x && event.button.x <= bb_tuning.bottom_right_x && event.button.y >= bb_tuning.top_left_y && event.button.y <= bb_tuning.bottom_right_y)
			{
				show_tuning = true;
			}
			if(event.button.x >= bb_back.top_left_x && event.button.x <= bb_back.bottom_right_x && event.button.y >= bb_back.top_left_y && event.button.y <= bb_back.bottom_right_y)
			{
				user_actions.clicked_back = true;
			}
		}

		if(event.type == SDL_MOUSEWHEEL)
		{
			user_actions.mouse_scroll = true;
			user_actions.scroll_direction = event.wheel.y;
		}
		else
		{
			user_actions.mouse_scroll = false;
			user_actions.scroll_direction = 0;
		}
		if(event.type == SDL_MOUSEMOTION)
		{
			user_actions.mouseX = event.motion.x;
			user_actions.mouseY = event.motion.y;
		}
		if(event.type == SDL_KEYDOWN && in_racing_game)
		{
			if(event.key.keysym.sym == SDLK_c)
				user_actions.key_c = true;
			else
				user_actions.key_c = false;

			if(event.key.keysym.sym == SDLK_v)
				user_actions.key_v = true;
			else
				user_actions.key_v = false;
		}
	}
	
	if(SDL_BUTTON(mouseButtonBitMask) == SDL_BUTTON_MIDDLE)
	{
		user_actions.xRel = mouseX_rel;
		user_actions.yRel = mouseY_rel;
		user_actions.mouse_middle = true;
	}
	else
	{
		user_actions.xRel = 0;
		user_actions.yRel = 0;
		user_actions.mouse_middle = false;
	}
	if(SDL_BUTTON(mouseButtonBitMask) == SDL_BUTTON_LEFT)
	{
		user_actions.mouse_left = true;
		if(user_actions.slide_volume)
		{
			sound_volume = ((user_actions.mouseX - bb_sound_slider.top_left_x) * 100) / (bb_sound_slider.bottom_right_x - bb_sound_slider.top_left_x);
			if(sound_volume > 100){sound_volume = 100;}
			if(sound_volume < 0){sound_volume = 0;}
			main_menu_source->set_volume(sound_volume);
		}
	}
	else
		user_actions.mouse_left = false;

	if(keyboardState[SDL_SCANCODE_LSHIFT])
		user_actions.key_shift = true;
	else
		user_actions.key_shift = false;
	if(countdown_timer < 0)
	{
	    if(keyboardState[SDL_SCANCODE_DOWN])
		    user_actions.key_down = true;
	    else
		    user_actions.key_down = false;
		if(keyboardState[SDL_SCANCODE_UP])
			user_actions.key_up = true;
		else
			user_actions.key_up = false;
		if(keyboardState[SDL_SCANCODE_SPACE])
			user_actions.key_space = true;
		else
			user_actions.key_space = false;
		if(keyboardState[SDL_SCANCODE_LEFT])
			user_actions.key_left = true;
		else
			user_actions.key_left = false;
		if(keyboardState[SDL_SCANCODE_RIGHT])
			user_actions.key_right = true;
		else
			user_actions.key_right = false;
	}
	if(keyboardState[SDL_SCANCODE_ESCAPE] && in_racing_game)
	{
		exit_game = true;
	}
#if CHECK_RENDER_PASS == 1
	if(keyboardState[SDL_SCANCODE_F1] && in_racing_game && ! print_quit_game)
	{
		if(in_racing_game && check_render_pass)
			check_render_pass = false;
		else if(in_racing_game && !check_render_pass)
			check_render_pass = true;
	}
#endif
}

void Game::update_menu_bb(int win_max_width, int win_max_height)
{
	bb_play.top_left_x = (win_max_width / 8) * 3;
	bb_play.top_left_y = (win_max_height / 2) * 0.334;
	bb_play.bottom_right_x = (win_max_width / 8) * 5;
	bb_play.bottom_right_y = (win_max_height / 2) * 0.5;
	
	bb_infos.top_left_x = (win_max_width / 8) * 3;
	bb_infos.top_left_y = (win_max_height / 2) * 0.656;
	bb_infos.bottom_right_x = (win_max_width / 8) * 5;
	bb_infos.bottom_right_y = (win_max_height / 2) * 0.826;
	
	bb_pod_specs.top_left_x = (win_max_width / 16) * 5;
	bb_pod_specs.top_left_y = (win_max_height / 2);
	bb_pod_specs.bottom_right_x = (win_max_width / 16) * 11;
	bb_pod_specs.bottom_right_y = (win_max_height / 2) * 1.166;
	
	bb_quit.top_left_x = (win_max_width / 8) * 3;
	bb_quit.top_left_y = (win_max_height / 2) * 1.333;
	bb_quit.bottom_right_x = (win_max_width / 8) * 5;
	bb_quit.bottom_right_y = (win_max_height / 2) * 1.5;

	bb_tuning.top_left_x = (win_max_width / 30) * 29;
	bb_tuning.top_left_y = 0;
	bb_tuning.bottom_right_x = win_max_width;
	bb_tuning.bottom_right_y = (win_max_width / 40);
	
	bb_back.top_left_x = 0;
	bb_back.top_left_y = 0;
	bb_back.bottom_right_x = (win_max_width / 30);
	bb_back.bottom_right_y = (win_max_width / 40);
	
	bb_map.top_left_x = (win_max_width / 8) * 2;
	bb_map.top_left_y = (win_max_height / 8) * 3;
	bb_map.bottom_right_x = (win_max_width / 8) * 3;
	bb_map.bottom_right_y = (win_max_height / 8) * 5;
	
	bb_controls.top_left_x = (win_max_width / 8) * 5;
	bb_controls.top_left_y = (win_max_height / 8) * 3;
	bb_controls.bottom_right_x = (win_max_width / 8) * 6;
	bb_controls.bottom_right_y = (win_max_height / 8) * 5;
	
    bb_cast_shadows.top_left_x = win_max_width * 0.43;
	bb_cast_shadows.top_left_y = win_max_height * 0.34;
	bb_cast_shadows.bottom_right_x = win_max_width * 0.47;
	bb_cast_shadows.bottom_right_y = win_max_height * 0.391;
	
	bb_close_tuning_window.top_left_x = (win_max_width / 4) * 2.875;
	bb_close_tuning_window.top_left_y = (win_max_height / 10) * 2;
	bb_close_tuning_window.bottom_right_x = (win_max_width / 4) * 3;
	bb_close_tuning_window.bottom_right_y = (win_max_height / 10) * 2.55;
	
	bb_sound_slider.top_left_x = win_max_width * 0.615;
	bb_sound_slider.top_left_y = win_max_height * 0.353;
	bb_sound_slider.bottom_right_x = win_max_width * 0.715;
	bb_sound_slider.bottom_right_y = win_max_height * 0.38;
	
	bb_quit_no.top_left_x = win_max_width * 0.496;
	bb_quit_no.top_left_y = win_max_height * 0.375;
	bb_quit_no.bottom_right_x = win_max_width * 0.555;
	bb_quit_no.bottom_right_y = win_max_height * 0.45;
	
	bb_quit_yes.top_left_x = win_max_width * 0.417;
	bb_quit_yes.top_left_y = win_max_height * 0.375;
	bb_quit_yes.bottom_right_x = win_max_width * 0.48;
	bb_quit_yes.bottom_right_y = win_max_height * 0.45;
	
    bb_main_menu.top_left_x = win_max_width * 0.406;
	bb_main_menu.top_left_y = win_max_height * 0.859;
	bb_main_menu.bottom_right_x = win_max_width * 0.595;
	bb_main_menu.bottom_right_y = win_max_height * 0.955;
}

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

const int Camera::EDITOR = 0;
const int Camera::PODRACER_PILOT = 1;
const int Camera::POD_SPECS = 2;
const int Camera::MINIMAP = 3;
const float Camera::ROLL_ANGLE_RATE = 0.01445f;
const float Camera::TURN_ANGLE_RATE = 4.71f; // 1.5 * PI
const float Camera::KMH_TO_MS = 0.278f;

Camera::Camera(int p_type, int screen_w, int screen_h, float p_fov)
{
	type = p_type;
	fov = p_fov;
	yaw = 0.0f;
	pitch = 0.0f;
	roll = 0.0f;
	move_sensitivity = 0.35f;
    cam_move = true;

	if(type == Camera::PODRACER_PILOT)
	{
		position = glm::vec3(0.0f, 3.8f, -7.0f);
		direction = glm::vec3(0.0f, 0.0f, 1.0f);
		target = position + direction;
		target_eye_length = glm::length(target - position);

		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::normalize(glm::cross(up, direction));	

		view = glm::lookAt(position, target, up);
		projection = glm::perspective(glm::radians(fov), static_cast<float>(screen_w / screen_h), 0.1f, 4500.0f);
		model = glm::translate(glm::mat4(1.0f), position);
	}
	if(type == Camera::EDITOR)
	{
		position = glm::vec3(0.0f, 0.575f, 0.0f);
		direction = glm::vec3(0.0f, 0.0f, 1.0f);
		target = position + direction;
		target_eye_length = glm::length(target - position);

		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::normalize(glm::cross(up, direction));	

		view = glm::lookAt(position, target, up);
		projection = glm::perspective(glm::radians(fov), static_cast<float>(screen_w / screen_h), 0.1f, 1000.0f);
		model = glm::translate(glm::mat4(1.0f), position);
	}
	if(type == Camera::POD_SPECS)
	{
		yaw = -1.4f;
		pitch = -0.4f;
		update_yaw_and_pitch(yaw, pitch, 0.0f);
		target = glm::vec3(0.0f, 1.25f, 0.0f);
		target_eye_length = 1.5f;
		position = target - 4.85f * direction;
		up = glm::vec3(0.0f, 1.0f, 0.0f);

		view = glm::lookAt(glm::vec3(position.x * target_eye_length, position.y, position.z * target_eye_length), target, up);
		projection = glm::perspective(glm::radians(fov), static_cast<float>(screen_w / screen_h), 0.1f, 100.0f);
		model = glm::translate(glm::mat4(1.0f), position);
	}
	if(type == Camera::MINIMAP)
	{
		position = glm::vec3(-8000.0f, 18000.0f, 3000.0f);
		direction = glm::vec3(0.0f, -1.0f, 0.0f);
		target = position + direction;
		target_eye_length = glm::length(target - position);

		up = glm::vec3(0.0f, 0.0f, 1.0f);
		right = glm::normalize(glm::cross(up, direction));	

		view = glm::lookAt(position, target, up);
		projection = glm::perspective(glm::radians(fov), static_cast<float>(screen_w / screen_h), 0.1f, 18000.0f);
		model = glm::translate(glm::mat4(1.0f), position);
	}
}

void Camera::reset()
{
	yaw = 0.0f;
	pitch = 0.0f;
	roll = 0.0f;
    cam_move = true;
	
	if(type == Camera::PODRACER_PILOT)
	{
		position = glm::vec3(0.0f, 3.8f, -7.0f);
		direction = glm::vec3(0.0f, 0.0f, 1.0f);
		target = position + direction;
		target_eye_length = glm::length(target - position);

		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::normalize(glm::cross(up, direction));	

		view = glm::lookAt(position, target, up);
		model = glm::translate(glm::mat4(1.0f), position);
	}
	if(type == Camera::EDITOR)
	{
		position = glm::vec3(0.0f, 0.575f, 0.0f);
		direction = glm::vec3(0.0f, 0.0f, 1.0f);
		target = position + direction;
		target_eye_length = glm::length(target - position);

		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::normalize(glm::cross(up, direction));	

		view = glm::lookAt(position, target, up);
		model = glm::translate(glm::mat4(1.0f), position);
	}
	if(type == Camera::POD_SPECS)
	{
		yaw = -1.4f;
		pitch = -0.4f;
		update_yaw_and_pitch(yaw, pitch, 0.0f);
		target = glm::vec3(0.0f, 1.25f, 0.0f);
		target_eye_length = 1.5f;
		position = target - 4.85f * direction;
		up = glm::vec3(0.0f, 1.0f, 0.0f);

		view = glm::lookAt(glm::vec3(position.x * target_eye_length, position.y, position.z * target_eye_length), target, up);
		model = glm::translate(glm::mat4(1.0f), position);
	}
}

int Camera::get_type() const { return type; }

glm::vec3 Camera::get_position() const { return position; }

glm::vec3 Camera::get_direction() const { return direction; }

glm::vec3 Camera::get_vector_up() const { return glm::normalize(glm::cross(glm::normalize(direction), glm::normalize(right))); }

glm::vec3 Camera::get_vector_right() const { return glm::normalize(glm::cross(glm::normalize(up), glm::normalize(direction))); }

glm::vec3 Camera::get_center_ray() const
{
	glm::vec3 start = position - (0.1f * glm::normalize(direction));
	glm::vec3 end = position - (4500.1f * glm::normalize(direction));
	glm::vec3 center_ray = glm::normalize(end - start);
	//std::cout << "center_ray = (" << center_ray.x << ", " << center_ray.y << ", " << center_ray.z << ")" << std::endl;
	return center_ray;
}

glm::mat4 Camera::get_view() const { return view; }

glm::mat4 Camera::get_projection() const { return projection; }

glm::mat4 Camera::get_model() const
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(yaw), this->get_vector_up());
	model = glm::rotate(model, glm::radians(pitch), this->get_vector_right());
	return glm::mat4(1.0f);
}

float Camera::get_yaw() const { return yaw; }

void Camera::update_view(Game* g, float delta)
{
	if(type == Camera::EDITOR)
	{
		if(g->user_actions.mouse_scroll)
		{
			// zoom in
			if(g->user_actions.scroll_direction == 1)
			{
				position = position - (direction * move_sensitivity * delta * 10.0f);
				target_eye_length = glm::length(target - position);
			}
			// zoom out
			else
			{
				position = position + (direction * move_sensitivity * delta * 10.0f);
				target_eye_length = glm::length(target - position);
			}
			// final step
			direction = glm::normalize(position - target);
			right = glm::normalize(glm::cross(up, direction));
			view = glm::lookAt(position, target, up);
			// reset mouse scroll states
			g->user_actions.mouse_scroll = false;
			g->user_actions.scroll_direction = 0;
		}
		if(g->user_actions.mouse_middle && !g->user_actions.key_shift)
		{
			// look around target
			update_yaw_and_pitch(g->user_actions.xRel, g->user_actions.yRel, delta);
			position = target - (direction * target_eye_length);
			position.y = position.y;
			right = glm::normalize(glm::cross(up, direction));
			view = glm::lookAt(position, target, up);
		}
		if(g->user_actions.mouse_middle && g->user_actions.key_shift)
		{
			// pan view
			glm::vec3 tmp_up = glm::normalize(glm::cross(direction, right));
			
			position += right * g->user_actions.xRel * move_sensitivity * delta;
			position += tmp_up * g->user_actions.yRel * move_sensitivity * delta;
			
			target += right * g->user_actions.xRel * move_sensitivity * delta;
			target += tmp_up * g->user_actions.yRel * move_sensitivity * delta;
			
			target_eye_length = glm::length(target - position);

			view = glm::lookAt(position, target, up);
		}
	}
	
	if(type == Camera::POD_SPECS)
	{
		if(g->user_actions.mouse_middle && !g->user_actions.key_shift)
		{
			// look around target
			update_yaw_and_pitch(g->user_actions.xRel, 0.0f, delta);
			position = target - 4.85f * direction;
			right = glm::normalize(glm::cross(up, direction));
			view = glm::lookAt(glm::vec3(position.x * target_eye_length, position.y, position.z * target_eye_length), target, up);
		}
	}
}

void Camera::update_view(glm::mat4 & transform, glm::vec3 pod_direction, Game* g)
{
    glm::vec3 translate;
    glm::vec3 scale;
    glm::quat orientation;
    glm::vec3 skew;
    glm::vec4 persp;
    glm::decompose(transform, scale, orientation, translate, skew, persp);
    orientation = glm::conjugate(orientation);

    float angle = glm::angle(orientation);
    glm::vec3 axis = glm::axis(orientation);
    glm::vec3 ypr(angle * axis.x, angle * axis.y, angle * axis.z);
    yaw = ypr.y;
    pitch = ypr.x;

    position = translate;
    direction = pod_direction;
    right = glm::normalize(glm::cross(up, direction));
    position += (-3.25f * direction) + (1.25f * up);
    target = position + direction;

    if(g->pod->turbojet_on && !g->user_actions.key_right && !g->user_actions.key_left)
        update_roll(Camera::ROLL_ANGLE_RATE, g, true);
    else if(g->pod->turbojet_on && g->user_actions.key_left)
        update_roll(Camera::ROLL_ANGLE_RATE, g);
    else if(g->pod->turbojet_on && g->user_actions.key_right)
        update_roll(-Camera::ROLL_ANGLE_RATE, g);

    view = glm::lookAt(position, target, up);
}

void Camera::turn_view(float p_yaw, float p_pitch)
{
    yaw = p_yaw;
    pitch = p_pitch;

    direction.x = cos(pitch) * sin(-yaw);
    direction.y = sin(pitch);
    direction.z = cos(yaw) * cos(pitch);
    direction = glm::normalize(direction);
}

void Camera::update_yaw_and_pitch(float xDif, float yDif, float delta)
{
	yaw += xDif * move_sensitivity * delta;
	pitch += yDif * move_sensitivity * delta;

	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	direction.x = cos(pitch) * sin(-yaw);
	direction.y = sin(pitch);
	direction.z = cos(yaw) * cos(pitch);
	direction = glm::normalize(direction);
}

void Camera::update_roll(float v, Game * g, bool reset)
{
    float epsilon = 0.01f;

    if(reset)
    {
        if(roll > 0.0f)
            roll -= v;
        else if(roll < 0.0f)
            roll += v;
        else
            return;

        if(fabs(roll) < epsilon)
        {
            roll = 0.0f;
            up = glm::vec3(0.0f, 1.0f, 0.0f);
            return;
        }

        up.x = sin(roll) * cos(yaw);
        up.y = cos(roll);
        up.z = sin(yaw) * sin(roll);
        up = glm::normalize(up);
    }
    else
    {
        roll += v;
        if(roll > g->pod->max_roll_angle)
        {
            roll = g->pod->max_roll_angle;
        }
        if(roll < - g->pod->max_roll_angle)
        {
            roll = - g->pod->max_roll_angle;
        }

        up.x = sin(roll) * cos(yaw);
        up.y = cos(roll);
        up.z = sin(yaw) * sin(roll);
        up = glm::normalize(up);
    }
}

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

Podracer::Podracer(std::string p_name, Game* p_g)
{
	// engine variables
	name = p_name;
	power_coupling_on = false;
	electric_engine_on = false;
	afterburn_on = false;
	turbojet_on = false;
	fuel_power = 0.0f;
	speed = 0.0f;
	heat = 42.0f;
	overheat = 750.0f;
	max_speed = 850.0f;
    max_roll_angle = 0.348f; // (PI/9) or 20 degrees
    collide_terrain = false;
    collide_ground = false;

	// camera + game ptr
	g = p_g;
	cam = g->cam;

	// pod objects + objects_shader + smoke + smoke_shader + power + power_shader
    chariot = new Object("../assets/podracers/chariot.obj", true);
    dir_left = new Object("../assets/podracers/direction_left.obj", true);
    dir_right = new Object("../assets/podracers/direction_right.obj", true);
    cable_left = new Object("../assets/podracers/cable_left.obj", true, false, true);
    cable_right = new Object("../assets/podracers/cable_right.obj", true, false, true);
    reactor_left = new Object("../assets/podracers/reactor_left.obj", true);
    reactor_right = new Object("../assets/podracers/reactor_right.obj", true);
    rotor_left = new Object("../assets/podracers/rotor_left.obj", true);
    rotor_right = new Object("../assets/podracers/rotor_right.obj", true);
    air_scoops_left1 = new Object("../assets/podracers/air_scoops_left1.obj", true);
    air_scoops_left2 = new Object("../assets/podracers/air_scoops_left2.obj", true);
    air_scoops_left3 = new Object("../assets/podracers/air_scoops_left3.obj", true);
    air_scoops_left_hinge1 = new Object("../assets/podracers/air_scoops_left_hinge1.obj", true);
    air_scoops_left_hinge2 = new Object("../assets/podracers/air_scoops_left_hinge2.obj", true);
    air_scoops_left_hinge3 = new Object("../assets/podracers/air_scoops_left_hinge3.obj", true);
    air_scoops_right1 = new Object("../assets/podracers/air_scoops_right1.obj", true);
    air_scoops_right2 = new Object("../assets/podracers/air_scoops_right2.obj", true);
    air_scoops_right3 = new Object("../assets/podracers/air_scoops_right3.obj", true);
    air_scoops_right_hinge1 = new Object("../assets/podracers/air_scoops_right_hinge1.obj", true);
    air_scoops_right_hinge2 = new Object("../assets/podracers/air_scoops_right_hinge2.obj", true);
    air_scoops_right_hinge3 = new Object("../assets/podracers/air_scoops_right_hinge3.obj", true);

    // get rotor left translate vector
    std::vector<Mesh*> rotor_meshes = rotor_left->get_mesh_collection();
    float rotor_left_max_x = 0.0f; float rotor_left_min_x = 0.0f;
    float rotor_left_max_y = 0.0f; float rotor_left_min_y = 0.0f;
    for(int i = 0; i < rotor_meshes.size(); i++)
    {
        Mesh * m = rotor_meshes.at(i);
        std::vector<Vertex> vertices = m->get_vertex_list();
        for(int j = 0; j < vertices.size(); j++)
        {
            if(j == 0)
            {
                glm::vec3 pos = vertices.at(j).position;
                rotor_left_min_x = pos.x;
                rotor_left_max_x = pos.x;
                rotor_left_min_y = pos.y;
                rotor_left_max_y = pos.y;
            }
            else
            {
                glm::vec3 pos = vertices.at(j).position;
                if(pos.x <= rotor_left_min_x)
                    rotor_left_min_x = pos.x;
                if(pos.x >= rotor_left_max_x)
                    rotor_left_max_x = pos.x;
                if(pos.y <= rotor_left_min_y)
                    rotor_left_min_y = pos.y;
                if(pos.y >= rotor_left_max_y)
                    rotor_left_max_y = pos.y;
            }
        }
    }
    rotor_left_translate = glm::vec3((rotor_left_max_x + rotor_left_min_x) / 2.0f, (rotor_left_max_y + rotor_left_min_y) / 2.0f, 0.0f);
    
    // get rotor right translate vector
    rotor_meshes.clear();
    rotor_meshes = rotor_right->get_mesh_collection();
    float rotor_right_max_x = 0.0f; float rotor_right_min_x = 0.0f;
    float rotor_right_max_y = 0.0f; float rotor_right_min_y = 0.0f;
    for(int i = 0; i < rotor_meshes.size(); i++)
    {
        Mesh * m = rotor_meshes.at(i);
        std::vector<Vertex> vertices = m->get_vertex_list();
        for(int j = 0; j < vertices.size(); j++)
        {
            if(j == 0)
            {
                glm::vec3 pos = vertices.at(j).position;
                rotor_right_min_x = pos.x;
                rotor_right_max_x = pos.x;
                rotor_right_min_y = pos.y;
                rotor_right_max_y = pos.y;
            }
            else
            {
                glm::vec3 pos = vertices.at(j).position;
                if(pos.x <= rotor_right_min_x)
                    rotor_right_min_x = pos.x;
                if(pos.x >= rotor_right_max_x)
                    rotor_right_max_x = pos.x;
                if(pos.y <= rotor_right_min_y)
                    rotor_right_min_y = pos.y;
                if(pos.y >= rotor_right_max_y)
                    rotor_right_max_y = pos.y;
            }
        }
    }
    rotor_right_translate = glm::vec3((rotor_right_max_x + rotor_right_min_x) / 2.0f, (rotor_right_max_y + rotor_right_min_y) / 2.0f, 0.0f);
    
	pod_shader = new Shader("../shaders/podracer/vertex.glsl", "../shaders/podracer/fragment.glsl", "../shaders/podracer/geometry.glsl");
	pod_shader->use();
	pod_shader->set_Matrix("model", glm::mat4(1.0f));
	pod_shader->set_Matrix("view", cam->get_view());
	pod_shader->set_Matrix("proj", cam->get_projection());
	pod_shader->set_vec3f("sun.dir", g->env->get_sun_direction());
	pod_shader->set_vec3f("sun.color", g->env->get_sun_color());
	pod_shader->set_vec3f("view_pos", cam->get_position());
	pod_shader->set_Matrix("sunlightSpaceMatrix", g->env->get_sun_spaceMatrix(true));

	// power coupling
	power = new Power(reactor_left->get_connectors(LEFT), reactor_right->get_connectors(RIGHT));
	power_shader = new Shader("../shaders/power/vertex.glsl", "../shaders/power/fragment.glsl", "../shaders/power/geometry.glsl");
	power_shader->use();
	power_shader->set_int("depth", 0);
	power_shader->set_Matrix("model", glm::mat4(1.0f));
	power_shader->set_Matrix("view", cam->get_view());
	power_shader->set_Matrix("proj", cam->get_projection());
	power_shader->set_texture("../assets/textures/bolt/bolt.png", 15, "img", true);
	
	// smoke
	smoke_left = new Smoke(reactor_left->get_left_sources(), reactor_left->get_smoke_direction(SMOKE_LEFT));
	smoke_left->set_init_dir(reactor_left->get_smoke_direction(SMOKE_LEFT));
	smoke_right = new Smoke(reactor_right->get_right_sources(), reactor_right->get_smoke_direction(SMOKE_RIGHT));
	smoke_right->set_init_dir(reactor_right->get_smoke_direction(SMOKE_RIGHT));

	smoke_shader = new Shader("../shaders/smoke/vertex.glsl", "../shaders/smoke/fragment.glsl", "../shaders/smoke/geometry.glsl");
	smoke_shader->use();
	smoke_shader->set_Matrix("model", glm::mat4(1.0f));
	smoke_shader->set_Matrix("view", cam->get_view());
	smoke_shader->set_Matrix("proj", cam->get_projection());
	
	smoke_shader->set_texture("../assets/textures/combustion/f1.png", 11, "f1", true);
	smoke_shader->set_texture("../assets/textures/combustion/f2.png", 12, "f2", true);
	smoke_shader->set_texture("../assets/textures/combustion/f3.png", 13, "f3", true);
	smoke_shader->set_texture("../assets/textures/combustion/f4.png", 14, "f4", true);
}

void Podracer::reset()
{
	// engine variables
	power_coupling_on = false;
	electric_engine_on = false;
	afterburn_on = false;
	turbojet_on = false;
	fuel_power = 0.0f;
	speed = 0.0f;
	heat = 42.0f;
	overheat = 750.0f;
	max_speed = 850.0f;
    collide_terrain = false;
    collide_ground = false;
}

Podracer::~Podracer()
{
	delete(chariot);
	delete(dir_left);
	delete(dir_right);
    delete(cable_left);
    delete(cable_right);
    delete(reactor_left);
    delete(reactor_right);
    delete(rotor_left);
    delete(rotor_right);
    delete(air_scoops_left1);
    delete(air_scoops_left2);
    delete(air_scoops_left3);
    delete(air_scoops_left_hinge1);
    delete(air_scoops_left_hinge2);
    delete(air_scoops_left_hinge3);
    delete(air_scoops_right1);
    delete(air_scoops_right2);
    delete(air_scoops_right3);
    delete(air_scoops_right_hinge1);
    delete(air_scoops_right_hinge2);
    delete(air_scoops_right_hinge3);
    delete(pod_shader);
    delete(smoke_left);
	delete(smoke_right);
	delete(smoke_shader);
    delete(power);
    delete(power_shader);
}

void Podracer::draw(bool shadowPass, bool depthPass, bool smokePass)
{
    // translate chariot
    static float tr_chariot = 0.0f;
    static float tr_rate = 0.0012f;
    float tr_clamp = 0.25f;
    glm::mat4 tr_left = glm::mat4(1.0f);
    glm::mat4 tr_right = glm::mat4(1.0f);
 
    if(g->user_actions.key_left)
    {
        tr_chariot += tr_rate;
        if(tr_chariot > tr_clamp)
            tr_chariot = tr_clamp;
        tr_left = glm::translate(tr_left, glm::vec3(tr_chariot, tr_chariot / 2.0f, 0.0f));
    }
    if(g->user_actions.key_right)
    {
        tr_chariot -= tr_rate;
        if(tr_chariot < -tr_clamp)
            tr_chariot = -tr_clamp;
        tr_right = glm::translate(tr_right, glm::vec3(tr_chariot, -tr_chariot / 2.0f, 0.0f));
    }
    if(!g->user_actions.key_left && !g->user_actions.key_right)
    {
        if(tr_chariot < 0.0f)
        {
            tr_chariot += tr_rate;
            if(tr_chariot > 0.0f)
                tr_chariot = 0.0f;
            if(tr_chariot == 0.0f)
            {
                tr_left = glm::mat4(1.0f);
            }
            tr_right = glm::translate(tr_right, glm::vec3(tr_chariot, -tr_chariot / 2.0f, 0.0f));
        }
        else if(tr_chariot > 0.0f)
        {
            tr_chariot -= tr_rate;
            if(tr_chariot < 0.0f)
                tr_chariot= 0.0f;
            if(tr_chariot == 0.0f)
            {
                tr_left = glm::mat4(1.0f);
            }
            tr_left = glm::translate(tr_left, glm::vec3(tr_chariot, tr_chariot / 2.0f, 0.0f));
        }
    }

    // rotate podracer's cables
    static float turn_angle = 0.0f;
    static float turn_rate = 0.1f;
    glm::mat4 turn_left = glm::mat4(1.0f);
    glm::mat4 turn_right = glm::mat4(1.0f);
    
    if(g->user_actions.key_left)
    {
        turn_angle -= turn_rate;
        if(turn_angle < -22.0f)
            turn_angle = -22.0f;
        turn_left = glm::rotate(turn_left, glm::radians(turn_angle), g->tatooine->get_pod_direction());
    }
    if(g->user_actions.key_right)
    {
        turn_angle += turn_rate;
        if(turn_angle > 22.0f)
            turn_angle = 22.0f;
        turn_right = glm::rotate(turn_right, glm::radians(turn_angle), g->tatooine->get_pod_direction());
    }
    if(!g->user_actions.key_left && !g->user_actions.key_right)
    {
        if(turn_angle < 0.0f)
        {
            turn_angle += turn_rate;
            if(turn_angle > 0.0f)
                turn_angle = 0.0f;
            if(turn_angle == 0.0f)
            {
                turn_left = glm::mat4(1.0f);
            }
            turn_left = glm::rotate(turn_left, glm::radians(turn_angle), g->tatooine->get_pod_direction());
        }
        else if(turn_angle > 0.0f)
        {
            turn_angle -= turn_rate;
            if(turn_angle < 0.0f)
                turn_angle = 0.0f;
            if(turn_angle == 0.0f)
            {
                turn_right = glm::mat4(1.0f);
            }
            turn_right = glm::rotate(turn_right, glm::radians(turn_angle), g->tatooine->get_pod_direction());
        }
    }

    glm::vec3 tr;
    glm::quat q;
    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 persp;
    glm::decompose(g->tatooine->reactors_model, scale, q, tr, skew, persp);

    glm::mat4 cable_to_origin(1.0f);
    cable_to_origin = glm::translate(cable_to_origin, -tr);
    glm::mat4 cable_back(1.0f);
    cable_back = glm::translate(cable_back, tr);
    
	// set cam ptr
	cam = g->cam;
	if(cam->get_type() != Camera::POD_SPECS)
    {
        pod_shader->use();
	    pod_shader->set_vec3f("sun.color", g->env->get_sun_color());
	    pod_shader->set_int("race", 1);
    }

    // draw
	if(!shadowPass)
	{
		power_shader->use();
		power_shader->set_Matrix("model", glm::mat4(1.0f));
		power_shader->set_Matrix("view", cam->get_view());
	
		smoke_shader->use();
		smoke_shader->set_Matrix("model", glm::mat4(1.0f));
		smoke_shader->set_Matrix("view", cam->get_view());

		pod_shader->use();
		pod_shader->set_int("shadowPass", 0);
		pod_shader->set_Matrix("model", glm::mat4(1.0f));
		pod_shader->set_vec3f("view_pos", cam->get_position());
		pod_shader->set_Matrix("view", cam->get_view());
		pod_shader->set_Matrix("prevCamView", g->prev_camera_view);
		pod_shader->set_Matrix("sunlightSpaceMatrix_env", g->env->get_sun_spaceMatrix(false));
		pod_shader->set_Matrix("sunlightSpaceMatrix_pod", g->env->get_sun_spaceMatrix(true));

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, g->shadowMap);
		pod_shader->set_int("envShadowMap", 10);
		
        glActiveTexture(GL_TEXTURE20);
		glBindTexture(GL_TEXTURE_2D, g->shadowMap2);
		pod_shader->set_int("podShadowMap", 20);
	}
	else
	{
		pod_shader->use();
		pod_shader->set_int("shadowPass", 1);
		pod_shader->set_Matrix("model", glm::mat4(1.0f));
		pod_shader->set_vec3f("view_pos", cam->get_position());
		pod_shader->set_Matrix("view", cam->get_view());
		pod_shader->set_Matrix("prevCamView", g->prev_camera_view);
		if(depthPass)
        {
			pod_shader->set_Matrix("sunlightSpaceMatrix_env", cam->get_projection() * cam->get_view());
			pod_shader->set_Matrix("sunlightSpaceMatrix_pod", cam->get_projection() * cam->get_view());
        }
		else
        {
			pod_shader->set_Matrix("sunlightSpaceMatrix_env", g->env->get_sun_spaceMatrix(false));
			pod_shader->set_Matrix("sunlightSpaceMatrix_pod", g->env->get_sun_spaceMatrix(true));
        }
		
        glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, g->shadowMap);
		pod_shader->set_int("envShadowMap", 10);
        
        glActiveTexture(GL_TEXTURE20);
		glBindTexture(GL_TEXTURE_2D, g->shadowMap2);
		pod_shader->set_int("podShadowMap", 20);
		
        if(g->process_pod_shadowPass)
            pod_shader->set_int("process_pod_shadowPass", 1);
        else
            pod_shader->set_int("process_pod_shadowPass", 0);
	}

	if(!smokePass)
    {
        if(cam->get_type() == Camera::POD_SPECS)
        {
            static float rotor_angle = 0.0f;
            rotor_angle += 18.0f;
            if(rotor_angle > 360.0f)
                rotor_angle = 0.0f;

            glm::mat4 chariot_model = glm::mat4(1.0f);
            chariot_model = glm::translate(chariot_model, glm::vec3(0.0f, 2.25f, -3.6f));
            
            glm::mat4 reactor_model = glm::mat4(1.0f);
            reactor_model = glm::translate(reactor_model, glm::vec3(0.0f, 1.5f, 1.5f));
            
            glm::mat4 rotor_left_origin = glm::mat4(1.0f);
            rotor_left_origin = glm::translate(rotor_left_origin, -rotor_left_translate);
            glm::mat4 rotor_left_rotate = glm::mat4(1.0f);
            rotor_left_rotate = glm::rotate(rotor_left_rotate, glm::radians(rotor_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 rotor_left_shift = glm::mat4(1.0f);
            rotor_left_shift = glm::translate(rotor_left_shift, rotor_left_translate);
            glm::mat4 rotor_left_model = glm::mat4(1.0f);
            rotor_left_model = glm::translate(rotor_left_model, glm::vec3(0.0f, 1.5f, 1.5f));
            
            glm::mat4 rotor_right_origin = glm::mat4(1.0f);
            rotor_right_origin = glm::translate(rotor_right_origin, -rotor_right_translate);
            glm::mat4 rotor_right_rotate = glm::mat4(1.0f);
            rotor_right_rotate = glm::rotate(rotor_right_rotate, glm::radians(-rotor_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 rotor_right_shift = glm::mat4(1.0f);
            rotor_right_shift = glm::translate(rotor_right_shift, rotor_right_translate);
            glm::mat4 rotor_right_model = glm::mat4(1.0f);
            rotor_right_model = glm::translate(rotor_right_model, glm::vec3(0.0f, 1.5f, 1.5f));
		    
		    pod_shader->set_Matrix("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.22f, 3.3f)) * chariot_model);
            cable_left->draw(*pod_shader);
            cable_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", chariot_model * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.02f, 0.3f)));
            chariot->draw(*pod_shader);
            dir_left->draw(*pod_shader);
            dir_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", reactor_model);
            reactor_left->draw(*pod_shader);
            reactor_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", rotor_left_model * rotor_left_shift * rotor_left_rotate * rotor_left_origin);
            rotor_left->draw(*pod_shader);
		    pod_shader->set_Matrix("model", rotor_right_model * rotor_right_shift * rotor_right_rotate * rotor_right_origin);
            rotor_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", reactor_model);
            air_scoops_left1->draw(*pod_shader);
            air_scoops_left2->draw(*pod_shader);
            air_scoops_left3->draw(*pod_shader);
            air_scoops_left_hinge1->draw(*pod_shader);
            air_scoops_left_hinge2->draw(*pod_shader);
            air_scoops_left_hinge3->draw(*pod_shader);
            air_scoops_right1->draw(*pod_shader);
            air_scoops_right2->draw(*pod_shader);
            air_scoops_right3->draw(*pod_shader);
            air_scoops_right_hinge1->draw(*pod_shader);
            air_scoops_right_hinge2->draw(*pod_shader);
            air_scoops_right_hinge3->draw(*pod_shader);
        }
        else
        { 
		    pod_shader->set_Matrix("model", g->tatooine->chariot_model * tr_left * tr_right * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.02f, 0.6f)));
		    chariot->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->chariot_model * tr_left * tr_right * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.02f, 0.6f)) * g->tatooine->dir_left_model);
            dir_left->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->chariot_model * tr_left * tr_right * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.02f, 0.6f)) * g->tatooine->dir_right_model);
            dir_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", cable_back * turn_right * turn_left * cable_to_origin);
            cable_left->draw(*pod_shader);
            cable_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model);
            reactor_left->draw(*pod_shader);
            reactor_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->rotor_left_model);
            rotor_left->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->rotor_right_model);
            rotor_right->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_left1_model);
            air_scoops_left1->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_left2_model);
            air_scoops_left2->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_left3_model);
            air_scoops_left3->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_left_hinge1_model);
            air_scoops_left_hinge1->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_left_hinge2_model);
            air_scoops_left_hinge2->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_left_hinge3_model);
            air_scoops_left_hinge3->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_right1_model);
            air_scoops_right1->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_right2_model);
            air_scoops_right2->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_right3_model);
            air_scoops_right3->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_right_hinge1_model);
            air_scoops_right_hinge1->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_right_hinge2_model);
            air_scoops_right_hinge2->draw(*pod_shader);
		    pod_shader->set_Matrix("model", g->tatooine->reactors_model * g->tatooine->air_scoops_right_hinge3_model);
            air_scoops_right_hinge3->draw(*pod_shader);
        }
    }
	if(!shadowPass)
    {
		draw_aux(depthPass, smokePass);
    }
}

inline void Podracer::draw_aux(bool depthPass, bool smokePass)
{
	if(cam->get_type() == Camera::POD_SPECS)
	{
		// power_coupling
        glm::mat4 reactor_model = glm::mat4(1.0f);
        reactor_model = glm::translate(reactor_model, glm::vec3(0.0f, 1.5f, 1.5f));
		power_shader->use();
		power_shader->set_int("depth", 0);
		power_shader->set_Matrix("model", reactor_model);
		power->draw(power_shader);
	}
	else
	{
        smoke_shader->use();
		smoke_shader->set_Matrix("model", g->tatooine->reactors_model * glm::mat4(1.0f));
        power_shader->use();
		power_shader->set_Matrix("model", g->tatooine->reactors_model * glm::mat4(1.0f));
		if(g->pod->speed > 0.0f)
		{
			// smoke
			if(smokePass)
			{
				smoke_shader->use();
				smoke_shader->set_Matrix("view", cam->get_view());

				if(depthPass)
					smoke_shader->set_int("depthPass", 1);
				else
					smoke_shader->set_int("depthPass", 0);
			
				smoke_left->draw(g->delta, smoke_shader);
				smoke_right->draw(g->delta, smoke_shader);
			}

			// power_coupling
			if(!smokePass)
			{
				power_shader->use();
				if(depthPass)
					power_shader->set_int("depth", 1);
				else
					power_shader->set_int("depth", 0);
				power->draw(power_shader);
			}
		}
		else if(power_coupling_on && !electric_engine_on)
		{
			// power_coupling
			if(!smokePass)
			{
				power_shader->use();
				if(depthPass)
					power_shader->set_int("depth", 1);
				else
					power_shader->set_int("depth", 0);
				power->draw(power_shader);
			}
		}
		else if(power_coupling_on && electric_engine_on)
		{
			// power_coupling
			if(!smokePass)
			{
				power_shader->use();
				if(depthPass)
					power_shader->set_int("depth", 1);
				else
					power_shader->set_int("depth", 0);
				power->draw(power_shader);
			}
		}
	}
}

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

Environment::Environment(std::string p_name, Game* p_g)
{
	name = p_name;
	
	// game ptr
	g = p_g;

	// camera ptr
	cam = g->cam;
	
	// create sun light
	sun_color = glm::vec3(8.0f, 7.8f, 6.5f);
	sun_direction = glm::vec3(2.0f, -2.0f, 0.0f);

	// sunlight space transform
	float near_plane = 1.0f;
	float far_plane;
	far_plane = 4500.0f;
    sunlightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glm::vec3 sun_pos = cam->get_position() + glm::vec3(-3.5f, 2.75f, 7.5f);
	glm::vec3 sun_target = sun_pos + sun_direction;
	glm::vec3 sun_up(0.0f, 1.0f, 0.0f);
	sunlightView = glm::lookAt(sun_pos, sun_target, sun_up);
	sunlightSpaceMatrix = sunlightProj * sunlightView;

	// env model matrix
	model_env = glm::mat4(1.0f);
	
	// load env and set env shader
	env.push_back(new Object("../assets/environment/mos_espa.obj"));
	env.push_back(new Object("../assets/environment/mos_espa_ground.obj"));
	env.push_back(new Object("../assets/environment/mos_espa_lap_count.obj", false, true));
    
    env_shader = new Shader("../shaders/env/vertex.glsl", "../shaders/env/fragment.glsl", "../shaders/env/geometry.glsl");
	env_shader->use();
	env_shader->set_Matrix("model", model_env);
	env_shader->set_Matrix("view", cam->get_view());
	env_shader->set_Matrix("proj", cam->get_projection());
	env_shader->set_vec3f("sun.dir", sun_direction);
	env_shader->set_vec3f("sun.color", sun_color);
	env_shader->set_Matrix("sunlightSpaceMatrix", sunlightSpaceMatrix);
	env_shader->set_int("animation", 0);
    
    // create mesh collection
    for(int i = 0; i < env.size(); i++)
    {
        std::vector<Mesh*> c = env.at(i)->get_mesh_collection();
        collection.insert(collection.end(), c.begin(), c.end());
    }
}

Environment::Environment(std::string p_name, std::string file_path, Game* p_g)
{
	name = p_name;
	
	// game ptr
	g = p_g;

	// camera ptr
	cam = g->cam;
	
	// create sun light
	sun_color = glm::vec3(8.0f, 7.8f, 6.5f);
	sun_direction = glm::vec3(2.0f, -2.0f, 0.0f);

	// sunlight space transform
	float near_plane = 0.1f;
	float far_plane;
	if(std::strcmp(name.c_str(), "Tatooine") == 0)
		far_plane = 4500.0f;
	else
		far_plane = 1000.0f;
    sunlightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glm::vec3 sun_pos = cam->get_position() + glm::vec3(-3.5f, 2.75f, 7.5f);
	glm::vec3 sun_target = sun_pos + sun_direction;
	glm::vec3 sun_up(0.0f, 1.0f, 0.0f);
	sunlightView = glm::lookAt(sun_pos, sun_target, sun_up);
	sunlightSpaceMatrix = sunlightProj * sunlightView;

	// env model matrix
	model_env = glm::mat4(1.0f);
	
	// load env and set env shader
    if(std::strcmp(name.c_str(), "Platform") == 0)
	    env.push_back(new Object(file_path.c_str(), true));
    else
    {
	    env.push_back(new Object("../assets/environment/mos_espa.obj"));
	    env.push_back(new Object("../assets/environment/mos_espa_ground.obj"));
	    env.push_back(new Object("../assets/environment/mos_espa_lap_count.obj", false, true));
    }
	
    env_shader = new Shader("../shaders/env/vertex.glsl", "../shaders/env/fragment.glsl", "../shaders/env/geometry.glsl");
	env_shader->use();
	env_shader->set_Matrix("model", model_env);
	env_shader->set_Matrix("view", cam->get_view());
	env_shader->set_Matrix("proj", cam->get_projection());
	env_shader->set_vec3f("sun.dir", sun_direction);
	env_shader->set_vec3f("sun.color", sun_color);
	env_shader->set_Matrix("sunlightSpaceMatrix", sunlightSpaceMatrix);
	env_shader->set_int("animation", 0);
    
    // create mesh collection
    for(int i = 0; i < env.size(); i++)
    {
        std::vector<Mesh*> c = env.at(i)->get_mesh_collection();
        collection.insert(collection.end(), c.begin(), c.end());
    }
}

void Environment::reset()
{
	// sunlight space transform
	float near_plane = 0.1f;
	float far_plane = 1000.0f;
    sunlightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glm::vec3 sun_pos = cam->get_position() + glm::vec3(-3.5f, 2.75f, 7.5f);
	glm::vec3 sun_target = sun_pos + sun_direction;
	glm::vec3 sun_up(0.0f, 1.0f, 0.0f);
	sunlightView = glm::lookAt(sun_pos, sun_target, sun_up);
	sunlightSpaceMatrix = sunlightProj * sunlightView;
}

std::vector<Mesh*> Environment::get_mesh_collection(bool mos_espa, int index)
{
    if(!mos_espa)
        return collection;
    else
    {
        return env.at(index)->get_mesh_collection();
    }
}

void Environment::reset_drawable()
{
    #pragma omp for
    for(int i = 0; i < env.size(); i++)
    {
        env.at(i)->reset_drawable();
    }
}

glm::mat4 Environment::get_model_env()
{
    return model_env;
}

Environment::~Environment()
{
    for(int i = 0; i < env.size(); i++)
	    delete(env.at(i));
	delete(env_shader);
}

glm::vec3 Environment::get_sun_color() const
{
    if(g->cam->get_type() != Camera::POD_SPECS)
	    return sun_color;
    else
        return glm::vec3(10.0f);
}

glm::vec3 Environment::get_sun_direction() const
{
	return sun_direction;
}

glm::mat4 Environment::get_sun_spaceMatrix(bool pod) const
{
    if(pod)
	    return sunlightSpaceMatrix;
    else
    {
	    // env sunlight space transform
	    float near_plane = 0.1f;
	    float far_plane = 4500.0f;
	    glm::mat4 podSunlightProj = glm::ortho(-1200.0f, 1200.0f, -1200.0f, 1200.0f, near_plane, far_plane);
	    glm::vec3 sun_pos = cam->get_position() + glm::vec3(-3.5f * 400.0f, 2.75f * 400.0f, 0.0f);
	    glm::vec3 sun_target = sun_pos + sun_direction;
	    glm::vec3 sun_up(0.0f, 1.0f, 0.0f);
        glm::mat4 podSunlightView = glm::lookAt(sun_pos, sun_target, sun_up);
        glm::mat4 podSunlightSpaceMatrix = podSunlightProj * podSunlightView;
        return podSunlightSpaceMatrix;
    }
}

void Environment::draw(bool shadowPass, bool depthPass)
{
	// set cam ptr
	cam = g->cam;
	
	// update model matrix
	if(cam->get_type() == Camera::POD_SPECS)
	{
		model_env = glm::mat4(1.0f);

        glm::vec3 sun_pos = cam->get_position() + glm::vec3(-3.5f, 2.75f, 0.0f);
		glm::vec3 sun_target = sun_pos + sun_direction;
		glm::vec3 sun_up(0.0f, 1.0f, 0.0f);
		sunlightView = glm::lookAt(sun_pos, sun_target, sun_up);
		sunlightSpaceMatrix = sunlightProj * sunlightView;

        env_shader->use();
        env_shader->set_vec3f("sun.color", glm::vec3(10.0f));
	}

	if(cam->get_type() != Camera::POD_SPECS)
	{
		model_env = glm::mat4(1.0f);
		if(std::strcmp(name.c_str(), "Tatooine") == 0)
		{
			glm::vec3 sun_pos = cam->get_position() + glm::vec3(-3.5f, 2.75f, 7.5f);
			glm::vec3 sun_target = sun_pos + sun_direction;
			glm::vec3 sun_up(0.0f, 1.0f, 0.0f);
			sunlightView = glm::lookAt(sun_pos, sun_target, sun_up);
			sunlightSpaceMatrix = sunlightProj * sunlightView;
		}
	}
	
	// draw environment
	env_shader->use();
	env_shader->set_Matrix("model", model_env);
	env_shader->set_Matrix("view", cam->get_view());
	if(depthPass)
	{
		env_shader->set_Matrix("sunlightSpaceMatrix_env", cam->get_projection() * cam->get_view());
		env_shader->set_Matrix("sunlightSpaceMatrix_pod", cam->get_projection() * cam->get_view());
	}
	else
	{
		env_shader->set_Matrix("sunlightSpaceMatrix_env", g->env->get_sun_spaceMatrix(false));
		env_shader->set_Matrix("sunlightSpaceMatrix_pod", g->env->get_sun_spaceMatrix(true));
	}
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, g->shadowMap);
	env_shader->set_int("envShadowMap", 10);
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, g->shadowMap2);
	env_shader->set_int("podShadowMap", 20);

    if(g->process_pod_shadowPass)
        env_shader->set_int("process_pod_shadowPass", 1);
    else
        env_shader->set_int("process_pod_shadowPass", 0);

	if(shadowPass)
	{
		env_shader->set_int("shadowPass", 1);
		env_shader->set_int("platform_attenuation", 0);
	}
	else
	{
		env_shader->set_int("shadowPass", 0);
		if(std::strcmp(name.c_str(), "Tatooine") == 0)
			env_shader->set_int("platform_attenuation", 0);
		else
			env_shader->set_int("platform_attenuation", 1);
	}

	if(g->cast_shadows)
		env_shader->set_int("cast_shadows", 1);
	else
		env_shader->set_int("cast_shadows", 0);

	for(int i = 0; i < env.size(); i++)
        env.at(i)->draw(*env_shader);
}

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

Minimap::Minimap(const std::string & file_path, Game* p_g)
{
	minimap = new Object(file_path, true);
	red_dot = new Object("../assets/environment/red_dot.obj", true);
	g = p_g;
	shader = new Shader("../shaders/minimap/vertex.glsl", "../shaders/minimap/fragment.glsl", "../shaders/minimap/geometry.glsl");
	sun_pos = glm::vec3(0.0f, 100.0f, 0.0f);
	sun_dir = glm::vec3(0.0f, -1.0f, 0.0f);
	sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
	
	// red dot position
	model_dot = glm::mat4(1.0f);
	
	dot_last_pos = glm::vec3(g->racing_cam->get_position().x, 0.0f, g->racing_cam->get_position().z);
	dot_last_rotation = 0.0f;

	trans_accu = glm::mat4(1.0f);
	rot_accu = glm::mat4(1.0f);

	// minimap framebuffer
	glGenFramebuffers(1, &minimapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, minimapFBO);

	glGenTextures(1, &minimapTexture);
	glBindTexture(GL_TEXTURE_2D, minimapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MINIMAP_DIMENSIONS, MINIMAP_DIMENSIONS, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimapTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, MINIMAP_DIMENSIONS, MINIMAP_DIMENSIONS);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: minimap framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// minimap quad
	float minimap_quad[] =
	{
		0.553f, -1.0f, -0.55f, 1.0f, 1.0f,
		1.0f, -1.0f, -0.55f, 0.0f, 1.0f,
		0.553f, -0.105f, -0.55f, 1.0f, 0.0f,
		1.0f, -0.105f, -0.55f, 0.0f, 0.0f
	};
	
	// geometry
	int indices[] = {0, 1, 2, 2, 1, 3};

	// set VAO VBO and EBO
	glGenVertexArrays(1, &minimapVAO);
	glBindVertexArray(minimapVAO);

	glGenBuffers(1, &minimapVBO);
	glBindBuffer(GL_ARRAY_BUFFER, minimapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(minimap_quad), &minimap_quad, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &minimapEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, minimapEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
}

Minimap::~Minimap()
{
	delete(minimap);
	delete(red_dot);
	delete(shader);
    
    glDeleteRenderbuffers(1, &RBO);
    glDeleteTextures(1, &minimapTexture);
    glDeleteFramebuffers(1, &minimapFBO);
}

void Minimap::update_framebuffer(int w, int h)
{
    float w_ratio = static_cast<float>(w) / WIDTH;
    float h_ratio = static_cast<float>(h) / HEIGHT;

    int minimap_w = static_cast<int>(w_ratio * static_cast<float>(MINIMAP_DIMENSIONS));
    int minimap_h = static_cast<int>(h_ratio * static_cast<float>(MINIMAP_DIMENSIONS));

	// ----- start delete minimap framebuffer -----
    glDeleteRenderbuffers(1, &RBO);
    glDeleteTextures(1, &minimapTexture);
    glDeleteFramebuffers(1, &minimapFBO);
    // ----- end delete minimap framebuffer -----
    
    // recreate minimap framebuffer
	glGenFramebuffers(1, &minimapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, minimapFBO);

	glGenTextures(1, &minimapTexture);
	glBindTexture(GL_TEXTURE_2D, minimapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, minimap_w, minimap_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimapTexture, 0);

	// depth and stencil renderbuffer object
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, minimap_w, minimap_h);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	// check if framebuffer is complete
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: minimap framebuffer is incomplete !" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

glm::mat4 Minimap::update_dot_model()
{
	// update arrow model matrix from camera data
	glm::mat4 model_dot_move = glm::mat4(1.0f);
	if(g->user_actions.key_up || g->user_actions.key_right || g->user_actions.key_left)
	{
		dot_current_rotation = g->racing_cam->get_yaw();
		dot_rotation = dot_current_rotation - dot_last_rotation;
		dot_last_rotation = dot_current_rotation;
		rot_accu = glm::rotate(rot_accu, -dot_rotation, glm::vec3(0.0f, 1.0f, 0.0f));

		dot_new_pos = glm::vec3(g->racing_cam->get_position().x, 0.0f, g->racing_cam->get_position().z);
		dot_translate = dot_new_pos - dot_last_pos;
		dot_last_pos = dot_new_pos;
		trans_accu = glm::translate(trans_accu, dot_translate);

		model_dot_move = trans_accu * rot_accu; 
	}
	else
	{
		dot_new_pos = glm::vec3(g->racing_cam->get_position().x, 0.0f, g->racing_cam->get_position().z);
		dot_translate = dot_new_pos - dot_last_pos;
		dot_last_pos = dot_new_pos;
		trans_accu = glm::translate(trans_accu, dot_translate);
		model_dot_move = trans_accu * rot_accu;
	}

	return model_dot_move;
}

void Minimap::draw(bool quit_game)
{
	// bind minimap framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, minimapFBO);
	glEnable(GL_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// draw
	g->cam = g->minimap_cam;
	shader->use();
	model_dot = update_dot_model();

    shader->set_Matrix("model", glm::mat4(1.0f));
	shader->set_Matrix("view", g->cam->get_view());
	shader->set_Matrix("proj", g->cam->get_projection());
	shader->set_vec3f("sun.dir", sun_dir);
	shader->set_vec3f("sun.color", sun_color);
	minimap->draw(*shader);
    shader->set_Matrix("model", model_dot);
	red_dot->draw(*shader);
	
    g->cam = g->racing_cam;

	if(!quit_game)
	{
		// bind default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ui shader
		g->ui_shader->use();
		g->ui_shader->set_float("alpha", 1.0f);

		// draw minimap square
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, minimapTexture);
		g->ui_shader->set_int("img", 0);
		glBindVertexArray(minimapVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	else
	{
		// bind default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, g->colorFBO);

		// ui shader
		g->ui_shader->use();
		g->ui_shader->set_float("alpha", 1.0f);

		// draw minimap square
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, minimapTexture);
		g->ui_shader->set_int("img", 0);
		glBindVertexArray(minimapVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
}

// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

int cmp_vertex(const void * a, const void * b)
{
    const Vertex * v1 = reinterpret_cast<const Vertex *>(a);
    const Vertex * v2 = reinterpret_cast<const Vertex *>(b);
    if(v1->position.z > v2->position.z)
        return 1;
    else if(v1->position.z < v2->position.z)
        return -1;
    else
        return 0;
}

struct RayCallback : public btCollisionWorld::RayResultCallback
{
    RayCallback()
    {
        distance = 0.0f;
    }

    ~RayCallback(){}
    
    btScalar addSingleResult(btCollisionWorld::LocalRayResult & rayResult, bool normalInWorldSpace)
    {
        const btCollisionObject * obj = rayResult.m_collisionObject;
        for(int i = 0; i < ground_objs.size(); i++)
        {
            if(ground_objs.at(i) == obj)
            {
                distance = 0.0f;
                break;
            }
        }
        return btScalar(1.0f);
    }

    // members
    float distance;
    std::vector<btCollisionObject*> ground_objs;
};

struct ContactCallback : public btCollisionWorld::ContactResultCallback
{
    ContactCallback()
    {
        lap_increase = false;
        collide_terrain = false;
        collide_ground = false;
    }

    ~ContactCallback(){}

    btScalar addSingleResult(btManifoldPoint & cp, const btCollisionObjectWrapper * colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper * colObj1Wrap, int partId1, int index1)
    {
        const btCollisionObject* obj2 = colObj1Wrap->getCollisionObject();

        if(obj2 == lap_count_obj)
            lap_increase = true;
        if(obj2 != reactors_obj && obj2 != chariot_obj && obj2 != cable_left_obj && obj2 != cable_right_obj && obj2 != lap_count_obj)
        {
            for(int i = 0; i < ground_objs.size(); i++)
            {
                if(ground_objs.at(i) == obj2)
                {
                    collide_ground = true;
                    return btScalar(1.0f);
                }
            }
            collide_terrain = true;
        }
        return btScalar(1.0f);
    }

    // members
    bool lap_increase;
    bool collide_terrain;
    bool collide_ground;
    btCollisionObject * lap_count_obj;
    btCollisionObject * reactors_obj;
    btCollisionObject * chariot_obj;
    btCollisionObject * cable_left_obj;
    btCollisionObject * cable_right_obj;
    std::vector<btCollisionObject*> ground_objs;
};

RayCallback ray_reactors;
RayCallback ray_chariot;
ContactCallback contact;
ContactCallback contact_lap;

WorldPhysics::WorldPhysics(Game* p_g) :
    engineForce(0.0f),
    engineForceIncrement(64.0f),
    maxEngineForce(4000.0f),
    breakingForce(0.0f),
    maxBreakingForce(70.0f),
    vehicleSteering(0.0f),
    steeringIncrement(0.0025f),
    steeringClamp(0.05f)
{
    // Game pointer
    g = p_g;

    // init model matrices
    chariot_model = glm::mat4(1.0f);
    dir_left_model = glm::mat4(1.0f);
    dir_right_model = glm::mat4(1.0f);
    reactors_model = glm::mat4(1.0f);
    rotor_left_model = glm::mat4(1.0f);
    rotor_right_model = glm::mat4(1.0f);
    air_scoops_left1_model = glm::mat4(1.0f);
    air_scoops_left2_model = glm::mat4(1.0f);
    air_scoops_left3_model = glm::mat4(1.0f);
    air_scoops_right1_model = glm::mat4(1.0f);
    air_scoops_right2_model = glm::mat4(1.0f);
    air_scoops_right3_model = glm::mat4(1.0f);
    air_scoops_left_hinge1_model = glm::mat4(1.0f);
    air_scoops_left_hinge2_model = glm::mat4(1.0f);
    air_scoops_left_hinge3_model = glm::mat4(1.0f);
    air_scoops_right_hinge1_model = glm::mat4(1.0f);
    air_scoops_right_hinge2_model = glm::mat4(1.0f);
    air_scoops_right_hinge3_model = glm::mat4(1.0f);

    // ----- bullet init start -----
    collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btSoftRigidDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0.0f, -9.8f, 0.0f));

    softBody_worldInfo = new btSoftBodyWorldInfo();
    softBody_worldInfo->m_broadphase = overlappingPairCache;
    softBody_worldInfo->m_dispatcher = dispatcher;
    softBody_worldInfo->m_sparsesdf.Initialize();

    // ----- bullet init end -----

    // ----- create boonta eve static rigid body -----
    std::vector<Mesh*> env_meshes = g->env->get_mesh_collection(true, 0);

    for(int i = 0; i < env_meshes.size(); i++)
    {
        Mesh * m = env_meshes.at(i);
        const std::vector<Vertex> & vertices = m->get_vertex_list();
        const std::vector<int> & indices = m->get_index_list();

        btTriangleMesh * triangleMesh = new btTriangleMesh();
        for(int j = 0; j < indices.size(); j += 3)
        {
            btVector3 v1(vertices.at(indices.at(j)).position.x, vertices.at(indices.at(j)).position.y, vertices.at(indices.at(j)).position.z);
            btVector3 v2(vertices.at(indices.at(j+1)).position.x, vertices.at(indices.at(j+1)).position.y, vertices.at(indices.at(j+1)).position.z);
            btVector3 v3(vertices.at(indices.at(j+2)).position.x, vertices.at(indices.at(j+2)).position.y, vertices.at(indices.at(j+2)).position.z);
            
            triangleMesh->addTriangle(v1, v2, v3);
        }
        btBvhTriangleMeshShape * env_shape = new btBvhTriangleMeshShape(triangleMesh, true);
        collisionShapes.push_back(env_shape);

        btTransform env_transform;
        env_transform.setIdentity();
        env_transform.setOrigin(btVector3(0, 0, 0));

        btScalar env_mass(0.0);
        btVector3 env_localInertia(0, 0, 0);
        btDefaultMotionState * env_motionState = new btDefaultMotionState(env_transform);
        btRigidBody::btRigidBodyConstructionInfo env_rbInfo(env_mass, env_motionState, env_shape, env_localInertia);
        btRigidBody * env_body = new btRigidBody(env_rbInfo);

        dynamicsWorld->addRigidBody(env_body, COLLISION_GROUP::ENV, COLLISION_GROUP::POD | COLLISION_GROUP::ENV);
    }
    
    std::vector<Mesh*> env_meshes_ground = g->env->get_mesh_collection(true, 1);

    for(int i = 0; i < env_meshes_ground.size(); i++)
    {
        Mesh * m = env_meshes_ground.at(i);
        const std::vector<Vertex> & vertices = m->get_vertex_list();
        const std::vector<int> & indices = m->get_index_list();

        btTriangleMesh * triangleMesh = new btTriangleMesh();
        for(int j = 0; j < indices.size(); j += 3)
        {
            btVector3 v1(vertices.at(indices.at(j)).position.x, vertices.at(indices.at(j)).position.y, vertices.at(indices.at(j)).position.z);
            btVector3 v2(vertices.at(indices.at(j+1)).position.x, vertices.at(indices.at(j+1)).position.y, vertices.at(indices.at(j+1)).position.z);
            btVector3 v3(vertices.at(indices.at(j+2)).position.x, vertices.at(indices.at(j+2)).position.y, vertices.at(indices.at(j+2)).position.z);
            
            triangleMesh->addTriangle(v1, v2, v3);
        }
        btBvhTriangleMeshShape * env_shape = new btBvhTriangleMeshShape(triangleMesh, true);
        collisionShapes.push_back(env_shape);

        btTransform env_transform;
        env_transform.setIdentity();
        env_transform.setOrigin(btVector3(0, 0, 0));

        btScalar env_mass(0.0);
        btVector3 env_localInertia(0, 0, 0);
        btDefaultMotionState * env_motionState = new btDefaultMotionState(env_transform);
        btRigidBody::btRigidBodyConstructionInfo env_rbInfo(env_mass, env_motionState, env_shape, env_localInertia);
        btRigidBody * env_body = new btRigidBody(env_rbInfo);

        dynamicsWorld->addRigidBody(env_body, COLLISION_GROUP::ENV, COLLISION_GROUP::POD | COLLISION_GROUP::ENV);

        // distance
        btCollisionObject * obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];
        ray_reactors.ground_objs.push_back(obj);
        ray_chariot.ground_objs.push_back(obj);
        contact.ground_objs.push_back(obj);
        contact_lap.ground_objs.push_back(obj);
    }

    std::vector<Mesh*> env_mesh_lap = g->env->get_mesh_collection(true, 2);

    for(int i = 0; i < env_mesh_lap.size(); i++)
    {
        Mesh * m = env_mesh_lap.at(i);
        const std::vector<Vertex> & vertices = m->get_vertex_list();
        const std::vector<int> & indices = m->get_index_list();

        btTriangleMesh * triangleMesh = new btTriangleMesh();
        for(int j = 0; j < indices.size(); j += 3)
        {
            btVector3 v1(vertices.at(indices.at(j)).position.x, vertices.at(indices.at(j)).position.y, vertices.at(indices.at(j)).position.z);
            btVector3 v2(vertices.at(indices.at(j+1)).position.x, vertices.at(indices.at(j+1)).position.y, vertices.at(indices.at(j+1)).position.z);
            btVector3 v3(vertices.at(indices.at(j+2)).position.x, vertices.at(indices.at(j+2)).position.y, vertices.at(indices.at(j+2)).position.z);
            
            triangleMesh->addTriangle(v1, v2, v3);
        }
        btBvhTriangleMeshShape * env_shape = new btBvhTriangleMeshShape(triangleMesh, true);
        collisionShapes.push_back(env_shape);

        btTransform env_transform;
        env_transform.setIdentity();
        env_transform.setOrigin(btVector3(0, 0, 0));

        btScalar env_mass(0.0);
        btVector3 env_localInertia(0, 0, 0);
        btDefaultMotionState * env_motionState = new btDefaultMotionState(env_transform);
        btRigidBody::btRigidBodyConstructionInfo env_rbInfo(env_mass, env_motionState, env_shape, env_localInertia);
        btRigidBody * env_body = new btRigidBody(env_rbInfo);

        dynamicsWorld->addRigidBody(env_body, COLLISION_GROUP::NONE, COLLISION_GROUP::NONE);

        // lap count
        btCollisionObject * obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];
        contact.lap_count_obj = obj;
        contact_lap.lap_count_obj = obj;
    }

    // ----- create podracer dynamic rigid body -----
    // reactors
    {
        btVector3 moveY(0.0, 1.5, 0.0);
        btVector3 moveZ(0.0, 0.0, 1.5);
        btVector3 scaleXYZ(1.8, 0.4, 2.0);
        btBoxShape* shape = new btBoxShape(btVector3(1.0, 1.0, 1.0) * scaleXYZ);
        collisionShapes.push_back(shape);

        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(moveY + moveZ);

        btScalar mass(1000.0f);
        btVector3 localInertia(0, 0, 0);
        shape->calculateLocalInertia(mass, localInertia);
        btDefaultMotionState * myMotionState = new btDefaultMotionState(trans);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
        reactors_body = new btRigidBody(rbInfo);

        // add the body to the dynamics world
        dynamicsWorld->addRigidBody(reactors_body, COLLISION_GROUP::POD, COLLISION_GROUP::POD | COLLISION_GROUP::ENV);
        reactors_body->setActivationState(DISABLE_DEACTIVATION);

    }
    reactors_colObj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];
    contact.reactors_obj = reactors_colObj;
    contact_lap.reactors_obj = reactors_colObj;
    
    // chariot
    {
        btVector3 moveY(0.0, 2.25, 0.0);
        btVector3 moveZ(0.0, 0.0, -3.6);
        btVector3 scaleXYZ(0.65, 0.25, 1.0);
        btBoxShape* shape = new btBoxShape(btVector3(1.0, 1.0, 1.0) * scaleXYZ);
        collisionShapes.push_back(shape);

        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(moveY + moveZ);

        btScalar mass(1.0);
        btVector3 localInertia(0, 0, 0);
        shape->calculateLocalInertia(mass, localInertia);
        btDefaultMotionState * myMotionState = new btDefaultMotionState(trans);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
        chariot_body = new btRigidBody(rbInfo);

        // add the body to the dynamics world
        dynamicsWorld->addRigidBody(chariot_body, COLLISION_GROUP::POD, COLLISION_GROUP::POD | COLLISION_GROUP::ENV);
        chariot_body->setActivationState(DISABLE_DEACTIVATION);
    }
    
    contact.chariot_obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];
    contact_lap.chariot_obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];

    // ----------********** START CREATE RAYCAST VEHICLE **********----------
    raycaster = new btDefaultVehicleRaycaster(dynamicsWorld);
    vehicle = new btRaycastVehicle(tuning, reactors_body, raycaster);

    // add vehicle to the world
    dynamicsWorld->addVehicle(vehicle);

    // choose coordinate system
    vehicle->setCoordinateSystem(0, 1, 2);

    // vehicle data
    bool isFrontWheel = true;
    float wheelWidth = 0.25f;
    float wheelFriction = 1000.0f;
    float suspensionStiffness = 20.0f;
    float suspensionDamping = 2.3f;
    float suspensionCompression = 4.4f;
    float rollInfluence = 0.01f;
    btVector3 wheelDirectionCSO(0.0f, -1.0f, 0.0f);
    btVector3 wheelAxleCS(-1.0f, 0.0f, 0.0f);
    btScalar suspensionRestLength(0.1f);
    btScalar wheelRadius(0.5f);

    // add first wheel (front left)
    btVector3 connectionPointCSO(1.5f, -1.0f, 1.6f);
    vehicle->addWheel(connectionPointCSO, wheelDirectionCSO, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel);
    
    // add second wheel (front right)
    connectionPointCSO = btVector3(-1.5f, -1.0f, 1.6f);
    vehicle->addWheel(connectionPointCSO, wheelDirectionCSO, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel);
    
    // add third wheel (back left)
    isFrontWheel = false;
    connectionPointCSO = btVector3(1.5f, -1.0f, -1.6f);
    vehicle->addWheel(connectionPointCSO, wheelDirectionCSO, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel);
    
    // add fourth wheel (back right)
    connectionPointCSO = btVector3(-1.5f, -1.0f, -1.6f);
    vehicle->addWheel(connectionPointCSO, wheelDirectionCSO, wheelAxleCS, suspensionRestLength, wheelRadius, tuning, isFrontWheel);

    for(int i = 0; i < vehicle->getNumWheels(); i++)
    {
        btWheelInfo & wheelInfo = vehicle->getWheelInfo(i);
        wheelInfo.m_suspensionStiffness = suspensionStiffness;
        wheelInfo.m_wheelsDampingRelaxation = suspensionDamping;
        wheelInfo.m_wheelsDampingCompression = suspensionCompression;
        wheelInfo.m_frictionSlip = wheelFriction;
        wheelInfo.m_rollInfluence = rollInfluence;
        wheelInfo.m_maxSuspensionForce = 36000.0f;
    }

    // ----------********** END CREATE RAYCAST VEHICLE **********----------
    
    // attach chariot to reactors
    btTransform global_contact;
    global_contact.setIdentity();
    global_contact.setOrigin(btVector3(0.0f, 1.5f, 0.0f));
    btTransform localFrameInR = reactors_body->getWorldTransform().inverse() * global_contact;
    btTransform localFrameInC = chariot_body->getWorldTransform().inverse() * global_contact;
    btGeneric6DofConstraint * connection = new btGeneric6DofConstraint(*reactors_body, *chariot_body, localFrameInR, localFrameInC, true);

    dynamicsWorld->addConstraint(connection);

    connection->setLinearLowerLimit(btVector3(0.0f, 0.0f, 0.0f));
    connection->setLinearUpperLimit(btVector3(0.0f, 0.15f, 0.15f));
    connection->setAngularLowerLimit(btVector3(-0.15f, 0.0f, -0.02f));
    connection->setAngularUpperLimit(btVector3(0.0f, 0.0f, 0.02f));
    
    // ----------********** CABLES CREATION
    // step simulation
    dynamicsWorld->stepSimulation(1.0f/60.0f, 10);
    
    // chariot model
    btTransform transform_chariot;
    if(chariot_body && chariot_body->getMotionState())
        chariot_body->getMotionState()->getWorldTransform(transform_chariot);
    transform_chariot.getOpenGLMatrix(glm::value_ptr(chariot_model));
    chariot_initialTransform = transform_chariot;

    // reactors model
    btTransform transform_reactors;
    if(reactors_body && reactors_body->getMotionState())
        reactors_body->getMotionState()->getWorldTransform(transform_reactors);
    transform_reactors.getOpenGLMatrix(glm::value_ptr(reactors_model));
    reactors_initialTransform = transform_reactors;
    
    // -----***** create cable left soft body
    Mesh* cable_left_mesh = g->pod->cable_left->get_mesh_collection().at(0);
    // get rid of duplicated vertices
    std::vector<Vertex> vertices = cable_left_mesh->get_vertex_list();
        for(int i = 0; i < vertices.size(); i++)
        {
            glm::vec3 before = vertices.at(i).position;
            glm::vec4 before2(before.x, before.y, before.z, 1.0f);
            glm::vec4 after = chariot_model * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.02f, 0.3f)) * before2;
            vertices.at(i).position = glm::vec3(after.x, after.y, after.z);
        }
        std::vector<int> indices = cable_left_mesh->get_index_list();

        std::vector<Vertex> fixed_left_vertices;
        std::vector<int> fixed_left_indices;

        std::vector<std::pair<glm::vec3, int>> couple;

        for(int i = 0; i < indices.size(); i += 3) // for each face
        {
            // get the 3 couples of the current face
            glm::vec3 a_pos = vertices.at(indices.at(i)).position;
            int a_index = indices.at(i);
        
            glm::vec3 b_pos = vertices.at(indices.at(i+1)).position;
            int b_index = indices.at(i+1);
        
            glm::vec3 c_pos = vertices.at(indices.at(i+2)).position;
            int c_index = indices.at(i+2);

            // check for each couple if its position is already defined by another couple in the clean array
            int a_def_index = get_vertex_defined_index(a_pos, a_index, couple);
            int b_def_index = get_vertex_defined_index(b_pos, b_index, couple);
            int c_def_index = get_vertex_defined_index(c_pos, c_index, couple);

            if(a_def_index == -1)
                couple.push_back(std::make_pair(a_pos, a_index));
            if(b_def_index == -1)
                couple.push_back(std::make_pair(b_pos, b_index));
            if(c_def_index == -1)
                couple.push_back(std::make_pair(c_pos, c_index));
        }

        for(int i = 0; i < couple.size(); i++)
        {
            int couple_index = couple.at(i).second;
            fixed_left_vertices.push_back(vertices.at(couple_index));
        }

        for(int i = 0; i < indices.size(); i += 3)
        {
            glm::vec3 a_pos = vertices.at(indices.at(i)).position;
            glm::vec3 b_pos = vertices.at(indices.at(i+1)).position;
            glm::vec3 c_pos = vertices.at(indices.at(i+2)).position;

            fixed_left_indices.push_back(retrieve_correct_index(a_pos, couple));
            fixed_left_indices.push_back(retrieve_correct_index(b_pos, couple));
            fixed_left_indices.push_back(retrieve_correct_index(c_pos, couple));
        }

        // create left soft body
        cable_left = btSoftBodyHelpers::CreateFromTriMesh(*softBody_worldInfo, vertex_list_2_btScalarArray(fixed_left_vertices), fixed_left_indices.data(), fixed_left_indices.size() / 3);

        // create left soft body material
        btSoftBody::Material* left_material = cable_left->appendMaterial();
        left_material->m_kLST = 0.75f;
        left_material->m_kAST = 1.0f;
        left_material->m_kVST = 1.0f;

        cable_left->generateBendingConstraints(5, left_material);
        cable_left->generateClusters(0);
        cable_left->setPose(true, false);
        cable_left->setTotalMass(0.65f, true);
        cable_left->m_cfg.piterations = 10;
        cable_left->randomizeConstraints();

        // add left soft body to the dynamics world
        dynamicsWorld->addSoftBody(cable_left);

        contact.cable_left_obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];
        contact_lap.cable_left_obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];

        // attach left soft body to chariot and reactors
        btSoftBody::tNodeArray left_nodes = cable_left->m_nodes;

        std::qsort(fixed_left_vertices.data(), fixed_left_vertices.size(), sizeof(Vertex), cmp_vertex);

        for(int i = 0; i < left_nodes.size(); i++)
        {
            btVector3 pos = left_nodes.at(i).m_x;
            for(int j = 0; j < 21; j++)
            {
                glm::vec3 v_pos = fixed_left_vertices.at(j).position;
                if(pos.x() == v_pos.x && pos.y() == v_pos.y && pos.z() && v_pos.z)
                {
                    cable_left->appendAnchor(i, chariot_body);
                }
            }
        }
    
        for(int i = 0; i < left_nodes.size(); i++)
        {
            btVector3 pos = left_nodes.at(i).m_x;
            for(int j = fixed_left_vertices.size() - 1; j > fixed_left_vertices.size() - 22; j--)
            {
                glm::vec3 v_pos = fixed_left_vertices.at(j).position;
                if(pos.x() == v_pos.x && pos.y() == v_pos.y && pos.z() && v_pos.z)
                {
                    cable_left->appendAnchor(i, reactors_body);
                }
            }
        }

        // recreate cable left mesh
        prev_c_left_vertices = init_previous_vertices(vertices, fixed_left_vertices);
        initial_c_left_vertices = prev_c_left_vertices;
        initial_c_left_indices = fixed_left_indices;
        g->pod->cable_left->get_mesh_collection().at(0)->recreate(prev_c_left_vertices, fixed_left_indices);
        
        // -----***** create cable right soft body
        Mesh* cable_right_mesh = g->pod->cable_right->get_mesh_collection().at(0);
        // get rid of duplicated vertices
        vertices.clear(); indices.clear();
        vertices = cable_right_mesh->get_vertex_list();
        for(int i = 0; i < vertices.size(); i++)
        {
            glm::vec3 before = vertices.at(i).position;
            glm::vec4 before2(before.x, before.y, before.z, 1.0f);
            glm::vec4 after = chariot_model * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.02f, 0.3f)) * before2;
            vertices.at(i).position = glm::vec3(after.x, after.y, after.z);
        }
        indices = cable_right_mesh->get_index_list();
    
        std::vector<Vertex> fixed_right_vertices;
        std::vector<int> fixed_right_indices;

        couple.clear();
    
        for(int i = 0; i < indices.size(); i += 3) // for each face
        {
            // get the 3 couples of the current face
            glm::vec3 a_pos = vertices.at(indices.at(i)).position;
            int a_index = indices.at(i);
        
            glm::vec3 b_pos = vertices.at(indices.at(i+1)).position;
            int b_index = indices.at(i+1);
        
            glm::vec3 c_pos = vertices.at(indices.at(i+2)).position;
            int c_index = indices.at(i+2);

            // check for each couple if its position is already defined by another couple in the clean array
            int a_def_index = get_vertex_defined_index(a_pos, a_index, couple);
            int b_def_index = get_vertex_defined_index(b_pos, b_index, couple);
            int c_def_index = get_vertex_defined_index(c_pos, c_index, couple);

            if(a_def_index == -1)
                couple.push_back(std::make_pair(a_pos, a_index));
            if(b_def_index == -1)
                couple.push_back(std::make_pair(b_pos, b_index));
            if(c_def_index == -1)
                couple.push_back(std::make_pair(c_pos, c_index));
        }

        for(int i = 0; i < couple.size(); i++)
        {
            int couple_index = couple.at(i).second;
            fixed_right_vertices.push_back(vertices.at(couple_index));
        }

        for(int i = 0; i < indices.size(); i += 3)
        {
            glm::vec3 a_pos = vertices.at(indices.at(i)).position;
            glm::vec3 b_pos = vertices.at(indices.at(i+1)).position;
            glm::vec3 c_pos = vertices.at(indices.at(i+2)).position;

            fixed_right_indices.push_back(retrieve_correct_index(a_pos, couple));
            fixed_right_indices.push_back(retrieve_correct_index(b_pos, couple));
            fixed_right_indices.push_back(retrieve_correct_index(c_pos, couple));
        }

        // create right soft body
        cable_right = btSoftBodyHelpers::CreateFromTriMesh(*softBody_worldInfo, vertex_list_2_btScalarArray(fixed_right_vertices), fixed_right_indices.data(), fixed_right_indices.size() / 3);

        // create right soft body material
        btSoftBody::Material* right_material = cable_right->appendMaterial();
        right_material->m_kLST = 0.75f;
        right_material->m_kAST = 1.0f;
        right_material->m_kVST = 1.0f;

        cable_right->generateBendingConstraints(5, right_material); // 2
        cable_right->generateClusters(0);
        cable_right->setPose(true, false);
        cable_right->setTotalMass(0.65f, true);
        cable_right->m_cfg.piterations = 10;
        cable_right->randomizeConstraints();

        // add right soft body to the dynamics world
        dynamicsWorld->addSoftBody(cable_right);
        
        contact.cable_right_obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];
        contact_lap.cable_right_obj = dynamicsWorld->getCollisionObjectArray()[collisionShapes.size() - 1];
        
        // attach right soft body to chariot and reactors
        btSoftBody::tNodeArray right_nodes = cable_right->m_nodes;
    
        std::qsort(fixed_right_vertices.data(), fixed_right_vertices.size(), sizeof(Vertex), cmp_vertex);
    
        for(int i = 0; i < right_nodes.size(); i++)
        {
            btVector3 pos = right_nodes.at(i).m_x;
            for(int j = 0; j < 21; j++)
            {
                glm::vec3 v_pos = fixed_right_vertices.at(j).position;
                if(pos.x() == v_pos.x && pos.y() == v_pos.y && pos.z() && v_pos.z)
                {
                    cable_right->appendAnchor(i, chariot_body);
                }
            }
        }
    
        for(int i = 0; i < right_nodes.size(); i++)
        {
            btVector3 pos = right_nodes.at(i).m_x;
            for(int j = fixed_right_vertices.size() - 1; j > fixed_right_vertices.size() - 22; j--)
            {
                glm::vec3 v_pos = fixed_right_vertices.at(j).position;
                if(pos.x() == v_pos.x && pos.y() == v_pos.y && pos.z() && v_pos.z)
                {
                    cable_right->appendAnchor(i, reactors_body);
                }
            }
        }
        
        // recreate cable right mesh
        prev_c_right_vertices = init_previous_vertices(vertices, fixed_right_vertices);
        initial_c_right_vertices = prev_c_right_vertices;
        initial_c_right_indices = fixed_right_indices;
        g->pod->cable_right->get_mesh_collection().at(0)->recreate(prev_c_right_vertices, fixed_right_indices);
}

WorldPhysics::~WorldPhysics()
{
    // remove rigidbodies from the dynamics world and delete them
    for(int i = 0; i < dynamicsWorld->getNumCollisionObjects(); i++)
    {
        btCollisionObject * obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody * body = btRigidBody::upcast(obj);
        if(body && body->getMotionState())
        {
            delete(body->getMotionState());
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete(obj);
    }

    // delete collision shapes
    for(int i = 0; i < collisionShapes.size(); i++)
    {
        btCollisionShape * shape = collisionShapes[i];
        collisionShapes[i] = 0;
        delete(shape);
    }
    
    // delete dynamics world
    delete(dynamicsWorld);
    
    // delete solver
    delete(solver);
    
    // delete broadphase
    delete(overlappingPairCache);
    
    // delete dispatcher
    delete(dispatcher);

    delete(collisionConfiguration);
}

void WorldPhysics::reset()
{
    // model matrices
    chariot_model = glm::mat4(1.0f);
    dir_left_model = glm::mat4(1.0f);
    dir_right_model = glm::mat4(1.0f);
    reactors_model = glm::mat4(1.0f);
    rotor_left_model = glm::mat4(1.0f);
    rotor_right_model = glm::mat4(1.0f);
    air_scoops_left1_model = glm::mat4(1.0f);
    air_scoops_left2_model = glm::mat4(1.0f);
    air_scoops_left3_model = glm::mat4(1.0f);
    air_scoops_right1_model = glm::mat4(1.0f);
    air_scoops_right2_model = glm::mat4(1.0f);
    air_scoops_right3_model = glm::mat4(1.0f);
    air_scoops_left_hinge1_model = glm::mat4(1.0f);
    air_scoops_left_hinge2_model = glm::mat4(1.0f);
    air_scoops_left_hinge3_model = glm::mat4(1.0f);
    air_scoops_right_hinge1_model = glm::mat4(1.0f);
    air_scoops_right_hinge2_model = glm::mat4(1.0f);
    air_scoops_right_hinge3_model = glm::mat4(1.0f);

    // reset cables
    g->pod->cable_left->get_mesh_collection().at(0)->recreate(initial_c_left_vertices, initial_c_left_indices);
    g->pod->cable_right->get_mesh_collection().at(0)->recreate(initial_c_right_vertices, initial_c_right_indices);

    // clear forces and velocities
    btVector3 zeroVector(0, 0, 0);

    reactors_body->clearForces();
    reactors_body->setLinearVelocity(zeroVector);
    reactors_body->setAngularVelocity(zeroVector);
    reactors_body->setWorldTransform(reactors_initialTransform);

    chariot_body->clearForces();
    chariot_body->setLinearVelocity(zeroVector);
    chariot_body->setAngularVelocity(zeroVector);
    chariot_body->setWorldTransform(chariot_initialTransform);
}

void WorldPhysics::update_dynamics()
{
    // vehicle stuff
    btVector3 vehicle_direction = vehicle->getForwardVector();
    btVector3 vehicle_right = vehicle_direction.cross(btVector3(0.0f, 1.0f, 0.0f));
    float vehicle_speed = vehicle->getCurrentSpeedKmHour();
    if(vehicle_speed < 0.0f)
        vehicle_speed = 0.0f;

    if(vehicle_speed < 100.0f)
        steeringClamp = exp(-vehicle_speed / 35.0f);
	else
        steeringClamp = 0.05f;

    if(!g->pod->power_coupling_on && g->user_actions.key_c)
	{
		g->pod->power_coupling_on = true;
	}
	if(g->pod->power_coupling_on && g->user_actions.key_v)
	{
		g->pod->electric_engine_on = true;
	}
	if(g->pod->electric_engine_on && g->user_actions.key_up && (g->user_actions.key_left || g->user_actions.key_right))
	{
		g->pod->turbojet_on = true;
        if(vehicle_speed <= 850.0f)
        {
            g->pod->speed = vehicle_speed;
            breakingForce = 0.0f;
            engineForce += engineForceIncrement;
            if(engineForce > maxEngineForce)
                engineForce = maxEngineForce;
        }
    }
    else if(g->pod->electric_engine_on && g->user_actions.key_up)
    {
		g->pod->turbojet_on = true;
        if(vehicle_speed <= 850.0f)
        {
            g->pod->speed = vehicle_speed;
            breakingForce = 0.0f;
            engineForce += engineForceIncrement;
            if(engineForce > maxEngineForce)
                engineForce = maxEngineForce;
            if(vehicleSteering < 0.0f)
            {
                vehicleSteering += steeringIncrement;
                if(vehicleSteering > 0.0f)
                    vehicleSteering = 0.0f;
            }
            else if(vehicleSteering > 0.0f)
            {
                vehicleSteering -= steeringIncrement;
                if(vehicleSteering < 0.0f)
                    vehicleSteering = 0.0f;

            }
        }
    }
    else if(g->pod->electric_engine_on && g->user_actions.key_space && !g->user_actions.key_up && (g->user_actions.key_left || g->user_actions.key_right))
    {
		g->pod->turbojet_on = true;
        if(vehicle_speed <= 850.0f)
        {
            g->pod->speed = vehicle_speed;
            breakingForce = 0.0f;
            engineForce += (engineForceIncrement * 40.0f);
            if(engineForce > (maxEngineForce * 3.0f))
                engineForce = maxEngineForce * 3.0f;

            // prevent pod from rolling
            reactors_body->setAngularFactor(0.0001f * vehicle_right);
        }
    }
    else if(g->pod->electric_engine_on && g->user_actions.key_space && !g->user_actions.key_up)
    {
		g->pod->turbojet_on = true;
        if(vehicle_speed <= 850.0f)
        {
            g->pod->speed = vehicle_speed;
            breakingForce = 0.0f;
            engineForce += (engineForceIncrement * 40.0f);
            if(engineForce > (maxEngineForce * 3.0f))
                engineForce = maxEngineForce * 3.0f;
            
            // prevent pod from rolling
            reactors_body->setAngularFactor(0.0001f * vehicle_right);
            
            if(vehicleSteering < 0.0f)
            {
                vehicleSteering += steeringIncrement;
                if(vehicleSteering > 0.0f)
                    vehicleSteering = 0.0f;
            }
            else if(vehicleSteering > 0.0f)
            {
                vehicleSteering -= steeringIncrement;
                if(vehicleSteering < 0.0f)
                    vehicleSteering = 0.0f;

            }
        }
    }
    if(g->user_actions.key_down)
    {
        g->pod->speed = vehicle_speed;
        breakingForce = maxBreakingForce;
    }
    else
    {
        g->pod->speed = vehicle_speed;
        breakingForce = 0.0f;
    }
    if(g->user_actions.key_right)
    {
        g->pod->speed = vehicle_speed;
        vehicleSteering -= steeringIncrement;
        if(vehicleSteering < -steeringClamp)
            vehicleSteering = -steeringClamp;
    }
    if(g->user_actions.key_left)
    {
        g->pod->speed = vehicle_speed;
        vehicleSteering += steeringIncrement;
        if(vehicleSteering > steeringClamp)
            vehicleSteering = steeringClamp;
    }
    if(!g->user_actions.key_up && !g->user_actions.key_space && !g->user_actions.key_down && !g->user_actions.key_right && !g->user_actions.key_left)
    {
        g->pod->speed = vehicle_speed;
        engineForce = 0.0f;
        if(vehicleSteering < 0.0f)
        {
            vehicleSteering += steeringIncrement;
            if(vehicleSteering > 0.0f)
                vehicleSteering = 0.0f;
        }
        else if(vehicleSteering > 0.0f)
        {
            vehicleSteering -= steeringIncrement;
            if(vehicleSteering < 0.0f)
                vehicleSteering = 0.0f;
        }
    }
    if(!g->pod->turbojet_on)
    {
        g->pod->speed = 0.0f;
    }

    int wheelIndex = 2;
    vehicle->applyEngineForce(engineForce, wheelIndex);
    vehicle->setBrake(breakingForce, wheelIndex);
    wheelIndex = 3;
    vehicle->applyEngineForce(engineForce, wheelIndex);
    vehicle->setBrake(breakingForce, wheelIndex);
    wheelIndex = 0;
    vehicle->setSteeringValue(vehicleSteering, wheelIndex);
    wheelIndex = 1;
    vehicle->setSteeringValue(vehicleSteering, wheelIndex);

    // step simulation
    dynamicsWorld->stepSimulation(1.0f/60.0f, 10);

    //distance from pod reactors to ground
    //btVector3 reactors_center_of_mass_pos = reactors_body->getCenterOfMassPosition();
    //dynamicsWorld->rayTest(reactors_initial_center + reactors_center_of_mass_pos, reactors_initial_center + reactors_center_of_mass_pos + btVector3(0.0f, -1.5f, 0.0f), ray_reactors);
    
    //distance from pod chariot to ground
    //btVector3 chariot_center_of_mass_pos = chariot_body->getCenterOfMassPosition();
    //dynamicsWorld->rayTest(chariot_initial_center + chariot_center_of_mass_pos, chariot_initial_center + chariot_center_of_mass_pos + btVector3(0.0f, -2.3f, 0.0f), ray_chariot);
    
    // turn podracer
    static float turn_angle = 0.0f;
    static float turn_rate = 0.5f;
    glm::mat4 turn_left = glm::mat4(1.0f);
    glm::mat4 turn_right = glm::mat4(1.0f);
    
    glm::mat4 r_turn_left = glm::mat4(1.0f);
    glm::mat4 r_turn_right = glm::mat4(1.0f);
    if(g->user_actions.key_left)
    {
        turn_angle -= turn_rate;
        if(turn_angle < -18.0f)
            turn_angle = -18.0f;
        turn_left = glm::rotate(turn_left, glm::radians(turn_angle), glm::vec3(0.0f, 0.0f, 1.0f));
        r_turn_left = glm::rotate(r_turn_left, glm::radians(-turn_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if(g->user_actions.key_right)
    {
        turn_angle += turn_rate;
        if(turn_angle > 18.0f)
            turn_angle = 18.0f;
        turn_right = glm::rotate(turn_right, glm::radians(turn_angle), glm::vec3(0.0f, 0.0f, 1.0f));
        r_turn_right = glm::rotate(r_turn_right, glm::radians(-turn_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if(!g->user_actions.key_left && !g->user_actions.key_right)
    {
        if(turn_angle < 0.0f)
        {
            turn_angle += turn_rate;
            if(turn_angle > 0.0f)
                turn_angle = 0.0f;
            if(turn_angle == 0.0f)
            {
                turn_left = glm::mat4(1.0f);
            }
            turn_left = glm::rotate(turn_left, glm::radians(turn_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            r_turn_left = glm::rotate(r_turn_left, glm::radians(-turn_angle), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if(turn_angle > 0.0f)
        {
            turn_angle -= turn_rate;
            if(turn_angle < 0.0f)
                turn_angle = 0.0f;
            if(turn_angle == 0.0f)
            {
                turn_right = glm::mat4(1.0f);
            }
            turn_right = glm::rotate(turn_right, glm::radians(turn_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            r_turn_left = glm::rotate(r_turn_left, glm::radians(-turn_angle), glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    // chariot model
    btTransform transform_chariot;
    if(chariot_body && chariot_body->getMotionState())
        chariot_body->getMotionState()->getWorldTransform(transform_chariot);
    transform_chariot.getOpenGLMatrix(glm::value_ptr(chariot_model));

    chariot_model = chariot_model * turn_left;
    chariot_model = chariot_model * turn_right;
    
    // reactors model
    btTransform transform_reactors;
    if(reactors_body && reactors_body->getMotionState())
        reactors_body->getMotionState()->getWorldTransform(transform_reactors);
    transform_reactors.getOpenGLMatrix(glm::value_ptr(reactors_model));
   
    reactors_model = reactors_model * turn_left * r_turn_left;
    reactors_model = reactors_model * turn_right * r_turn_right;

    // rotors
    static float rotor_angle = 0.0f;
    rotor_angle += 36.0f;
    if(rotor_angle > 360.0f)
        rotor_angle = 0.0f;

    glm::mat4 chariot_model = glm::mat4(1.0f);
    chariot_model = glm::translate(chariot_model, glm::vec3(0.0f, 2.25f, -3.6f));

    glm::mat4 reactor_model = glm::mat4(1.0f);
    reactor_model = glm::translate(reactor_model, glm::vec3(0.0f, 1.5f, 1.5f));
     
    // rotor left model
    glm::mat4 rotor_left_origin = glm::mat4(1.0f);
    rotor_left_origin = glm::translate(rotor_left_origin, -g->pod->rotor_left_translate);
    glm::mat4 rotor_left_rotate = glm::mat4(1.0f);
    rotor_left_rotate = glm::rotate(rotor_left_rotate, glm::radians(rotor_angle), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotor_left_shift = glm::mat4(1.0f);
    rotor_left_shift = glm::translate(rotor_left_shift, g->pod->rotor_left_translate);
    if(g->pod->electric_engine_on)
        rotor_left_model = reactors_model * rotor_left_shift * rotor_left_rotate * rotor_left_origin;
    else
        rotor_left_model = reactors_model;

    // rotor right model
    glm::mat4 rotor_right_origin = glm::mat4(1.0f);
    rotor_right_origin = glm::translate(rotor_right_origin, -g->pod->rotor_right_translate);
    glm::mat4 rotor_right_rotate = glm::mat4(1.0f);
    rotor_right_rotate = glm::rotate(rotor_right_rotate, glm::radians(-rotor_angle), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotor_right_shift = glm::mat4(1.0f);
    rotor_right_shift = glm::translate(rotor_right_shift, g->pod->rotor_right_translate);
    if(g->pod->electric_engine_on)
        rotor_right_model = reactors_model * rotor_right_shift * rotor_right_rotate * rotor_right_origin;
    else
        rotor_right_model = reactors_model;

    // update cable left shape
    btSoftBody::tFaceArray & faces_left = cable_left->m_faces;
    updated_c_left_vertices.clear();
    for(int i = 0; i < faces_left.size(); i++)
    {
        glm::vec3 v1(faces_left[i].m_n[0]->m_x.x(), faces_left[i].m_n[0]->m_x.y(), faces_left[i].m_n[0]->m_x.z());
        glm::vec3 v2(faces_left[i].m_n[1]->m_x.x(), faces_left[i].m_n[1]->m_x.y(), faces_left[i].m_n[1]->m_x.z());
        glm::vec3 v3(faces_left[i].m_n[2]->m_x.x(), faces_left[i].m_n[2]->m_x.y(), faces_left[i].m_n[2]->m_x.z());

        glm::vec3 v1_prev(faces_left[i].m_n[0]->m_q.x(), faces_left[i].m_n[0]->m_q.y(), faces_left[i].m_n[0]->m_q.z());
        glm::vec3 v2_prev(faces_left[i].m_n[1]->m_q.x(), faces_left[i].m_n[1]->m_q.y(), faces_left[i].m_n[1]->m_q.z());
        glm::vec3 v3_prev(faces_left[i].m_n[2]->m_q.x(), faces_left[i].m_n[2]->m_q.y(), faces_left[i].m_n[2]->m_q.z());

        glm::vec3 v1_n(faces_left[i].m_n[0]->m_n.x(), faces_left[i].m_n[0]->m_n.y(), faces_left[i].m_n[0]->m_n.z());
        glm::vec3 v2_n(faces_left[i].m_n[1]->m_n.x(), faces_left[i].m_n[1]->m_n.y(), faces_left[i].m_n[1]->m_n.z());
        glm::vec3 v3_n(faces_left[i].m_n[2]->m_n.x(), faces_left[i].m_n[2]->m_n.y(), faces_left[i].m_n[2]->m_n.z());

        Vertex vert1 = get_vertex(v1, v1_n, v1_prev, prev_c_left_vertices);
        Vertex vert2 = get_vertex(v2, v2_n, v2_prev, prev_c_left_vertices);
        Vertex vert3 = get_vertex(v3, v3_n, v3_prev, prev_c_left_vertices);

        updated_c_left_vertices.push_back(vert1);
        updated_c_left_vertices.push_back(vert2);
        updated_c_left_vertices.push_back(vert3);
    }
    prev_c_left_vertices.clear();
    prev_c_left_vertices = updated_c_left_vertices;

    g->pod->cable_left->get_mesh_collection().at(0)->update_VBO(updated_c_left_vertices);
 
    // update cable right shape
    btSoftBody::tFaceArray & faces_right = cable_right->m_faces;
    updated_c_right_vertices.clear();
    for(int i = 0; i < faces_right.size(); i++)
    {
        glm::vec3 v1(faces_right[i].m_n[0]->m_x.x(), faces_right[i].m_n[0]->m_x.y(), faces_right[i].m_n[0]->m_x.z());
        glm::vec3 v2(faces_right[i].m_n[1]->m_x.x(), faces_right[i].m_n[1]->m_x.y(), faces_right[i].m_n[1]->m_x.z());
        glm::vec3 v3(faces_right[i].m_n[2]->m_x.x(), faces_right[i].m_n[2]->m_x.y(), faces_right[i].m_n[2]->m_x.z());

        glm::vec3 v1_prev(faces_right[i].m_n[0]->m_q.x(), faces_right[i].m_n[0]->m_q.y(), faces_right[i].m_n[0]->m_q.z());
        glm::vec3 v2_prev(faces_right[i].m_n[1]->m_q.x(), faces_right[i].m_n[1]->m_q.y(), faces_right[i].m_n[1]->m_q.z());
        glm::vec3 v3_prev(faces_right[i].m_n[2]->m_q.x(), faces_right[i].m_n[2]->m_q.y(), faces_right[i].m_n[2]->m_q.z());

        glm::vec3 v1_n(faces_right[i].m_n[0]->m_n.x(), faces_right[i].m_n[0]->m_n.y(), faces_right[i].m_n[0]->m_n.z());
        glm::vec3 v2_n(faces_right[i].m_n[1]->m_n.x(), faces_right[i].m_n[1]->m_n.y(), faces_right[i].m_n[1]->m_n.z());
        glm::vec3 v3_n(faces_right[i].m_n[2]->m_n.x(), faces_right[i].m_n[2]->m_n.y(), faces_right[i].m_n[2]->m_n.z());

        Vertex vert1 = get_vertex(v1, v1_n, v1_prev, prev_c_right_vertices);
        Vertex vert2 = get_vertex(v2, v2_n, v2_prev, prev_c_right_vertices);
        Vertex vert3 = get_vertex(v3, v3_n, v3_prev, prev_c_right_vertices);

        updated_c_right_vertices.push_back(vert1);
        updated_c_right_vertices.push_back(vert2);
        updated_c_right_vertices.push_back(vert3);
    }
    prev_c_right_vertices.clear();
    prev_c_right_vertices = updated_c_right_vertices;

    g->pod->cable_right->get_mesh_collection().at(0)->update_VBO(updated_c_right_vertices);

    // direction left model
    static float dir_left_angle = 0.0f;
    if(g->user_actions.key_left || g->user_actions.key_down)
    {
        dir_left_angle -= 2.0f;
        if(dir_left_angle < -30.0f)
            dir_left_angle = -30.0f;
        glm::mat4 dir_left_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-0.363f, 0.0f, 0.4282f));
        glm::mat4 dir_left_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(dir_left_angle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 dir_left_back = glm::translate(glm::mat4(1.0f), glm::vec3(0.363f, 0.0f, -0.4282f));
        dir_left_model = dir_left_back * dir_left_rotate * dir_left_origin;
    }
    else
    {
        if(dir_left_angle < 0.0f)
        {
            dir_left_angle += 2.0f;
            if(dir_left_angle > 0.0f)
                dir_left_angle = 0.0f;
            
            glm::mat4 dir_left_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-0.363f, 0.0f, 0.4282f));
            glm::mat4 dir_left_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(dir_left_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 dir_left_back = glm::translate(glm::mat4(1.0f), glm::vec3(0.363f, 0.0f, -0.4282f));
            dir_left_model = dir_left_back * dir_left_rotate * dir_left_origin;
        }
        else
            dir_left_model = glm::mat4(1.0f);
    }

    // direction right model
    static float dir_right_angle = 0.0f;
    if(g->user_actions.key_right || g->user_actions.key_down)
    {
        dir_right_angle += 2.0f;
        if(dir_right_angle > 30.0f)
            dir_right_angle = 30.0f;
        glm::mat4 dir_right_origin = glm::translate(glm::mat4(1.0f), glm::vec3(0.3618f, 0.0f, 0.4282f));
        glm::mat4 dir_right_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(dir_right_angle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 dir_right_back = glm::translate(glm::mat4(1.0f), glm::vec3(-0.3618f, 0.0f, -0.4282f));
        dir_right_model = dir_right_back * dir_right_rotate * dir_right_origin;
    }
    else
    {
        if(dir_right_angle > 0.0f)
        {
            dir_right_angle -= 2.0f;
            if(dir_right_angle < 0.0f)
                dir_right_angle = 0.0f;
            
            glm::mat4 dir_right_origin = glm::translate(glm::mat4(1.0f), glm::vec3(0.3618f, 0.0f, 0.4282f));
            glm::mat4 dir_right_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(dir_right_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 dir_right_back = glm::translate(glm::mat4(1.0f), glm::vec3(-0.3618f, 0.0f, -0.4282f));
            dir_right_model = dir_right_back * dir_right_rotate * dir_right_origin;
        }
        else
            dir_right_model = glm::mat4(1.0f);
    }

    // air scoops models
    static float air_scoops_angle = 0.0f;
    
    if(g->user_actions.key_left || g->user_actions.key_right)
    {
        air_scoops_angle += 2.0f;
        if(air_scoops_angle > 20.0f)
            air_scoops_angle = 20.0f;
        // --------------------LEFT
        glm::mat4 left_hinge1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.427f, -0.2601f, -1.023f));
        glm::mat4 left_hinge1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 left_hinge1_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.427f, 0.2601f, 1.023f));

        air_scoops_left_hinge1_model = left_hinge1_back * left_hinge1_rotate * left_hinge1_origin;
        
        glm::mat4 left1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.4274f, -0.4437f, -1.144f));
        glm::mat4 left1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 left1_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.4274f, 0.4437f, 1.144f));
        
        air_scoops_left1_model = left1_back * left1_rotate * left1_origin;
        
        glm::mat4 left_hinge2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.654f, 0.16f, -1.027f));
        glm::mat4 left_hinge2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 left_hinge2_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.654f, -0.16f, 1.027f));
        
        air_scoops_left_hinge2_model = left_hinge2_back * left_hinge2_rotate * left_hinge2_origin;
        
        glm::mat4 left2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.798f, 0.1989f, -1.144f));
        glm::mat4 left2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 left2_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.798f, -0.1989f, 1.144f));
        
        air_scoops_left2_model = left2_back * left2_rotate * left2_origin;
        
        glm::mat4 left_hinge3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.203f, 0.16f, -1.027f));
        glm::mat4 left_hinge3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 left_hinge3_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.203f, -0.16f, 1.027f));
        
        air_scoops_left_hinge3_model = left_hinge3_back * left_hinge3_rotate * left_hinge3_origin;
        
        glm::mat4 left3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.055f, 0.1989f, -1.144f));
        glm::mat4 left3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 left3_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.055f, -0.1989f, 1.144f));
        
        air_scoops_left3_model = left3_back * left3_rotate * left3_origin;
        
        // --------------------RIGHT
        glm::mat4 right_hinge1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.4256f, -0.2601f, -1.023f));
        glm::mat4 right_hinge1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 right_hinge1_back = glm::translate(glm::mat4(1.0f), glm::vec3(+1.4256f, 0.2601f, 1.023f));
        
        air_scoops_right_hinge1_model = right_hinge1_back * right_hinge1_rotate * right_hinge1_origin;
        
        glm::mat4 right1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.4256f, -0.4437f, -1.144f));
        glm::mat4 right1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 right1_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.4256f, 0.4437f, 1.144f));
        
        air_scoops_right1_model = right1_back * right1_rotate * right1_origin;
        
        glm::mat4 right_hinge2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.199f, 0.16f, -1.027f));
        glm::mat4 right_hinge2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 right_hinge2_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.199f, -0.16f, 1.027f));
        
        air_scoops_right_hinge2_model = right_hinge2_back * right_hinge2_rotate * right_hinge2_origin;
        
        glm::mat4 right2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.056f, 0.1989f, -1.144f));
        glm::mat4 right2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 right2_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.056f, -0.1989f, 1.144f));
        
        air_scoops_right2_model = right2_back * right2_rotate * right2_origin;
        
        glm::mat4 right_hinge3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.648f, 0.16f, -1.027f));
        glm::mat4 right_hinge3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 right_hinge3_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.648f, -0.16f, 1.027f));
        
        air_scoops_right_hinge3_model = right_hinge3_back * right_hinge3_rotate * right_hinge3_origin;
        
        glm::mat4 right3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.797f, 0.1989f, -1.144f));
        glm::mat4 right3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 right3_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.797f, -0.1989f, 1.144f));
        
        air_scoops_right3_model = right3_back * right3_rotate * right3_origin;
    }
    else
    {
        // air scoops
        if(air_scoops_angle > 0.0f)
        {
            air_scoops_angle -= 2.0f;
            if(air_scoops_angle < 0.0f)
                air_scoops_angle = 0.0f;
        // --------------------LEFT
        glm::mat4 left_hinge1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.427f, -0.2601f, -1.023f));
        glm::mat4 left_hinge1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 left_hinge1_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.427f, 0.2601f, 1.023f));

        air_scoops_left_hinge1_model = left_hinge1_back * left_hinge1_rotate * left_hinge1_origin;
        
        glm::mat4 left1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.4274f, -0.4437f, -1.144f));
        glm::mat4 left1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 left1_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.4274f, 0.4437f, 1.144f));
        
        air_scoops_left1_model = left1_back * left1_rotate * left1_origin;
        
        glm::mat4 left_hinge2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.654f, 0.16f, -1.027f));
        glm::mat4 left_hinge2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 left_hinge2_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.654f, -0.16f, 1.027f));
        
        air_scoops_left_hinge2_model = left_hinge2_back * left_hinge2_rotate * left_hinge2_origin;
        
        glm::mat4 left2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.798f, 0.1989f, -1.144f));
        glm::mat4 left2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 left2_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.798f, -0.1989f, 1.144f));
        
        air_scoops_left2_model = left2_back * left2_rotate * left2_origin;
        
        glm::mat4 left_hinge3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.203f, 0.16f, -1.027f));
        glm::mat4 left_hinge3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 left_hinge3_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.203f, -0.16f, 1.027f));
        
        air_scoops_left_hinge3_model = left_hinge3_back * left_hinge3_rotate * left_hinge3_origin;
        
        glm::mat4 left3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(-1.055f, 0.1989f, -1.144f));
        glm::mat4 left3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 left3_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.055f, -0.1989f, 1.144f));
        
        air_scoops_left3_model = left3_back * left3_rotate * left3_origin;
        
        // --------------------RIGHT
        glm::mat4 right_hinge1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.4256f, -0.2601f, -1.023f));
        glm::mat4 right_hinge1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 right_hinge1_back = glm::translate(glm::mat4(1.0f), glm::vec3(+1.4256f, 0.2601f, 1.023f));
        
        air_scoops_right_hinge1_model = right_hinge1_back * right_hinge1_rotate * right_hinge1_origin;
        
        glm::mat4 right1_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.4256f, -0.4437f, -1.144f));
        glm::mat4 right1_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 right1_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.4256f, 0.4437f, 1.144f));
        
        air_scoops_right1_model = right1_back * right1_rotate * right1_origin;
        
        glm::mat4 right_hinge2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.199f, 0.16f, -1.027f));
        glm::mat4 right_hinge2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 right_hinge2_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.199f, -0.16f, 1.027f));
        
        air_scoops_right_hinge2_model = right_hinge2_back * right_hinge2_rotate * right_hinge2_origin;
        
        glm::mat4 right2_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.056f, 0.1989f, -1.144f));
        glm::mat4 right2_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(air_scoops_angle), glm::vec3(6.4f, 11.4f, 0.0f));
        glm::mat4 right2_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.056f, -0.1989f, 1.144f));
        
        air_scoops_right2_model = right2_back * right2_rotate * right2_origin;
        
        glm::mat4 right_hinge3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.648f, 0.16f, -1.027f));
        glm::mat4 right_hinge3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 right_hinge3_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.648f, -0.16f, 1.027f));
        
        air_scoops_right_hinge3_model = right_hinge3_back * right_hinge3_rotate * right_hinge3_origin;
        
        glm::mat4 right3_origin = glm::translate(glm::mat4(1.0f), glm::vec3(1.797f, 0.1989f, -1.144f));
        glm::mat4 right3_rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-air_scoops_angle), glm::vec3(-6.4f, 11.4f, 0.0f));
        glm::mat4 right3_back = glm::translate(glm::mat4(1.0f), glm::vec3(-1.797f, -0.1989f, 1.144f));
        
        air_scoops_right3_model = right3_back * right3_rotate * right3_origin;
        }
        else
        {
            // --------------------LEFT
            air_scoops_left_hinge1_model = glm::mat4(1.0f);
            air_scoops_left1_model = glm::mat4(1.0f);
            air_scoops_left_hinge2_model = glm::mat4(1.0f);
            air_scoops_left2_model = glm::mat4(1.0f);
            air_scoops_left_hinge3_model = glm::mat4(1.0f);
            air_scoops_left3_model = glm::mat4(1.0f);
        
            // --------------------RIGHT
            air_scoops_right_hinge1_model = glm::mat4(1.0f);
            air_scoops_right1_model = glm::mat4(1.0f);
            air_scoops_right_hinge2_model = glm::mat4(1.0f);
            air_scoops_right2_model = glm::mat4(1.0f);
            air_scoops_right_hinge3_model = glm::mat4(1.0f);
            air_scoops_right3_model = glm::mat4(1.0f);
        }
    }

    // -----=====----- check if another lap is completed -----=====-----
    dynamicsWorld->contactPairTest(reactors_colObj, contact_lap.lap_count_obj, contact_lap);
    dynamicsWorld->contactTest(reactors_colObj, contact);
    if(contact_lap.lap_increase)
    {
        contact_lap.lap_increase = false;
        if(!g->hit_count_lap_wall)
        {
            g->hit_count_lap_wall = true;
            g->lap_iterate++;
        }
    }
    else
    {
        g->hit_count_lap_wall = false;
    }
    
    // -----=====----- check if pod collide terrain -----=====-----
    if(contact.collide_terrain)
    {
        contact.collide_terrain = false;
        g->pod->collide_terrain = true;
    }
    
    // -----=====----- check if pod collide ground -----=====-----
    if(contact.collide_ground)
    {
        contact.collide_ground = false;
        g->pod->collide_ground = true;
    }

    // reset pod angular factor
    reactors_body->setAngularFactor(btVector3(1.0f, 1.0f, 1.0f));
}

btRaycastVehicle* WorldPhysics::get_vehicle()
{
    return vehicle;
}

int WorldPhysics::get_vertex_defined_index(glm::vec3 ref_pos, int ref_index, const std::vector<std::pair<glm::vec3, int>> & couples)
{
   int res= -1;
   for(int i = 0; i < couples.size(); i++)
   {
       std::pair<glm::vec3, int> couple = couples.at(i);
       glm::vec3 pos = couple.first;
       if(pos.x == ref_pos.x && pos.y == ref_pos.y && pos.z == ref_pos.z)
       {
            res = couple.second;
            break;
       }
   }
   return res;
}

int WorldPhysics::retrieve_correct_index(glm::vec3 ref_pos, const std::vector<std::pair<glm::vec3, int>> & couple)
{
    for(int i = 0; i < couple.size(); i++)
    {
        glm::vec3 pos = couple.at(i).first;

        if(ref_pos.x == pos.x && ref_pos.y == pos.y && ref_pos.z == pos.z)
            return i;
    }
    return -1;
}

std::vector<Vertex> WorldPhysics::init_previous_vertices(std::vector<Vertex> ref_vertices, std::vector<Vertex> v)
{
    std::vector<Vertex> res;

    for(int i = 0; i < v.size(); i++)
    {
        for(int j = 0; j < ref_vertices.size(); j++)
        {
            glm::vec3 ref_pos = ref_vertices.at(j).position;
            if(ref_pos.x == v.at(i).position.x && ref_pos.y == v.at(i).position.y && ref_pos.z == v.at(i).position.z)
            {
                res.push_back(ref_vertices.at(j));
                break;
            }
        }
    }

    return res;
}

Vertex WorldPhysics::get_vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 prev_pos, const std::vector<Vertex> & prev_vertices)
{
    for(int i = 0; i < prev_vertices.size(); i++)
    {
        Vertex curr = prev_vertices.at(i);
        if(curr.position.x == prev_pos.x && curr.position.y == prev_pos.y && curr.position.z == prev_pos.z)
        {
            return Vertex(pos, normal, curr.texCoords, curr.bonesID, curr.bonesWeight);
        }
    }
}

btScalar* WorldPhysics::vertex_list_2_btScalarArray(std::vector<Vertex> const & vertices)
{
    int nb_vertices = vertices.size();
    int alloc_size = nb_vertices * 3;
    btScalar * v = new btScalar[alloc_size];

    for(int i = 0; i < nb_vertices; i++)
    {
        glm::vec3 pos = vertices.at(i).position;
        v[i*3] = pos.x;
        v[i*3 + 1] = pos.y;
        v[i*3 + 2] = pos.z;
    }

    return v;
}

glm::vec3 WorldPhysics::get_pod_direction()
{
    btVector3 vector = vehicle->getForwardVector();
    return glm::vec3(vector.x(), vector.y(), vector.z());
}
