#pragma warning(disable:4305 4244 4005) //double - float missmatch, Macro redefinition, 

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

#include <freetype-2.10.1/include/ft2build.h>
#include FT_FREETYPE_H

#include <my_utils/my_utils.h>
#include <iostream>
#include <time.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>

#define NOMINMAX

#include <Control/game.h>

#include <HI/user_input.h>

#include <MT/opengl_thread.h>

#include <Rendering/HUD/font.h>
#include <Rendering/HUD/cursor.h>

#include <Utils/general_utils.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



HI::user_input user_input;

void window_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void window_cursor_pos_callback(GLFWwindow* window, double x, double y);
void window_size_callback(GLFWwindow* window, int width, int height);

int main() {
	int height = 500, width = 500, frame = 0;
	log4cpp::PropertyConfigurator::configure("../seeds/config/log4cpp.properties");
	srand(time(NULL));

	if (!glfwInit()) {
		printf("failed to init GLFW!");
		return 0;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

	GLFWwindow* window = glfwCreateWindow(height, width, "Seeds", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, window_key_callback);
	glfwSetMouseButtonCallback(window, window_mouse_button_callback);
	glfwSetCursorPosCallback(window, window_cursor_pos_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	if(glewInit()){
		printf("failed to initialize GLEW!");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(NULL);
	
	MT::opengl_thread::Instance().init(window);
	Rendering::HUD::cursor_singleton::Instance().init(window);

	MT::opengl_thread::Instance().add_and_register_command(0, [height, width]{
		glViewport(0, 0, height, width);
		
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		//glEnable(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);

		Rendering::framebuffer::Instance().update_viewport_size(glm::ivec2(height, width));
	});
		
	Control::game game(width, height);
		
	{
		using std::placeholders::_1;
		user_input.add_listener(std::bind(&Control::game::process_user_input, &game, _1));
		user_input.add_window_size_listener(std::bind(&Control::game::window_size_changed, &game, _1));
	}
	
	MT::opengl_thread::Instance().close_frame_registration(frame);
	MT::opengl_thread::Instance().wait(); //wait for all initialisations 


	std::cout << "main thread = [" << std::this_thread::get_id() << "]\nworker_threads = ";
	for(std::thread::id& id : MT::thread_pool::Instance().get_thread_ids()){
		std::cout << "[" << id << "], ";
	}
	std::cout << std::endl;
	my_utils::timer timer;
	timer.start(glfwGetTime());

	while (!glfwWindowShouldClose(window)) {
		frame++;
		glfwPollEvents();
		timer.end(glfwGetTime());
		timer.start(glfwGetTime());

		user_input.fire_events();

		Rendering::frame_data::delta_time = timer.diff;
		game.update(timer.diff, frame);
		game.render(frame);

		MT::opengl_thread::Instance().close_frame_registration(frame);

		while(MT::thread_pool::Instance().queue_size() > 10){
			MT::thread_pool::Instance().steal_and_execute();
		}

		//printf("queue sizes = [pool: %d] [opengl: %d]\n", MT::thread_pool::Instance().queue_size(), MT::opengl_thread::Instance().queue_size());
	}

	MT::thread_pool::Instance().shutdown();
	MT::opengl_thread::Instance().shutdown();
	glfwMakeContextCurrent(window);
	game.shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
}


void window_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	user_input.key_event(key, action);
}

void window_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	user_input.mouse_button_event(window, button, action);
}

void window_cursor_pos_callback(GLFWwindow* window, double x, double y){
	user_input.mouse_move(x, y);
}

void window_size_callback(GLFWwindow* window, int width, int height){
	MT::opengl_thread::Instance().add_and_register_command_current_frame([width, height] {
		Rendering::framebuffer::Instance().update_viewport_size(glm::ivec2(width, height));
	});
	user_input.window_size_changed(width, height);
}

