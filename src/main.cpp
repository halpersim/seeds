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
#include <functional>

#define NOMINMAX

#include "Control/game.h"
#include "Control/Utils/timer.h"

#include "HI/user_input.h"

#include "Rendering/HUD/font.h"
#include "Rendering/HUD/cursor.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>




HI::user_input user_input;

void window_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void window_cursor_pos_callback(GLFWwindow* window, double x, double y);
void window_size_callback(GLFWwindow* window, int width, int height);

int main() {


	if (!glfwInit()) {
		printf("failed to init GLFW!");
		return 0;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(500, 500, "Seeds", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, window_key_callback);
	glfwSetMouseButtonCallback(window, window_mouse_button_callback);
	glfwSetCursorPosCallback(window, window_cursor_pos_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	Rendering::HUD::cursor_singleton::Instance().init(window);

	//glewExperimental = GL_TRUE;
	glewExperimental = GL_FALSE;

	if (glewInit()) {
		printf("failed to init GLEW!");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}
	log4cpp::PropertyConfigurator::configure("../seeds/config/log4cpp.properties");
	srand(time(NULL));

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, 500, 500);
	
	int height, width;
	glfwGetWindowSize(window, &width, &height);
	Control::game game(width, height);
	Control::timer timer;
	
	{
		using std::placeholders::_1;
		user_input.add_listener(std::bind(&Control::game::process_user_input, &game, _1));
		user_input.add_window_size_listener(std::bind(&Control::game::window_size_changed, &game, _1));
	}
	const float& one = 1.f;
	GLenum error;
	Rendering::HUD::font font(Rendering::HUD::font::CONSOLAS, glm::vec3(1.f), 20.f);
	std::string diff = "";
	std::stringstream ss;
	double d = 0.01;

	printf("GL_VERSION = [%s]\n", glGetString(GL_VERSION));

	my_utils::dropout_array<double, 50> last_frames;
	while (!glfwWindowShouldClose(window)) {
		timer.start(glfwGetTime());

		glfwPollEvents();

		glClearBufferfv(GL_DEPTH, 0, &one);

		Rendering::frame_data::delta_time = timer.diff;
		user_input.fire_events();

		game.update(timer.diff);
		game.render();


		ss.str("");
		ss << diff;
		std::string s = ss.str();
		font.render_string(s, glm::vec2(0.f, font.vertical_advance()));
		
		
		error = glGetError();
		if (error != GL_NO_ERROR) {
			printf("error = 0x%X\n", error);
		}


		glfwSwapBuffers(window);
		timer.end(glfwGetTime());

		last_frames.add(timer.diff);
		double avg_time = 0;
		last_frames.for_each([&avg_time](const double& d){avg_time += d; });
		ss.str("");
		ss << " ~" << std::setprecision(1) << std::fixed << last_frames.size()/avg_time << " fps - frame time = " << std::setprecision(1) << std::fixed <<  1000 * avg_time/last_frames.size() << " ms ";
		diff = ss.str();
		d += timer.diff;
	}

	glfwDestroyWindow(window),
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
	user_input.window_size_changed(width, height);
}


	/*

	GLuint shader[5];
	shader[0] = my_utils::shader::load("../seeds/media/shader/torus_vs.glsl", GL_VERTEX_SHADER, true);
	shader[1] = my_utils::shader::load("../seeds/media/shader/torus_tcs.glsl", GL_TESS_CONTROL_SHADER, true);
  shader[2] = my_utils::shader::load("../seeds/media/shader/torus_tes.glsl", GL_TESS_EVALUATION_SHADER, true);
	shader[3] = my_utils::shader::load("../seeds/media/shader/torus_gs.glsl", GL_GEOMETRY_SHADER, true);
	shader[4] = my_utils::shader::load("../seeds/media/shader/torus_fs.glsl", GL_FRAGMENT_SHADER, true);

	GLuint torus_program;
	torus_program = my_utils::program::link_from_shaders(shader, 5, true, true);

	GLuint mw_loc = glGetUniformLocation(torus_program, "mw");
	GLuint vp_loc = glGetUniformLocation(torus_program, "vp");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	cylinder c;
	double cylinder_inner_angle = 0.f;
	double cylinder_outer_angle = 3.f;
	float hole_size = 0.35f;
	glm::vec3 torus_plane_normal = glm::vec3(0, 1.f, 0);

	glm::vec3 hole_pos = point_on_torus(cylinder_inner_angle, cylinder_outer_angle);
	glm::vec3 hole_inner_mid = glm::normalize(hole_pos - torus_plane_normal * glm::dot(hole_pos, torus_plane_normal)) * 1.f;
	glm::vec3 rot_vector = glm::normalize(point_on_torus(cylinder_inner_angle + 1.5707f, cylinder_outer_angle) - hole_inner_mid);
	glm::vec3	offset_ref_point = hole_pos - rot_vector * hole_size;
	glm::vec3 offset_ref_vector = hole_inner_mid - offset_ref_point;
	glm::vec3 offset_vector = glm::normalize(hole_pos - hole_inner_mid) * (glm::length(offset_ref_vector) - 1);

	c.bottom_mid = hole_pos + offset_vector * 0.35f;
	c.rad = hole_size;
	c.height = offset_vector * -5.f;


	GLuint excluded_space_buffer;
	glGenBuffers(1, &excluded_space_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, excluded_space_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(cylinder), &c, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, excluded_space_buffer);

	glm::mat4 pers = glm::perspective<double>(90, 1, 0.1, 10);
	glm::mat4 lookAt = glm::lookAt(glm::vec3(0, 0, -2), glm::vec3(0), glm::vec3(0, 1, 0));

	const float bg[] = { 0.2f, 0.2f, 0.2f, 1.f };
	const float& one = 1.f;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearBufferfv(GL_COLOR, 0, bg);
		glClearBufferfv(GL_DEPTH, 0, &one);
		
		glUseProgram(torus_program);

		glm::mat4 rot = glm::rotate<float>(glm::mat4(1.f), glfwGetTime(), glm::vec3(1, 1, 1));

		double size = glfwGetTime() * 0.1;
		while (size > 0.5)
			size-=0.5;
		
		glBindBuffer(GL_UNIFORM_BUFFER, excluded_space_buffer);
		cylinder* ptr = (cylinder*)glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE);
		*ptr = gen_hole(5.7f, 5, 0.4);
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		glUniformMatrix4fv(mw_loc, 1, GL_FALSE, glm::value_ptr(rot));
		glUniformMatrix4fv(vp_loc, 1, GL_FALSE, glm::value_ptr(pers * lookAt));

		glDrawArrays(GL_PATCHES, 0, 4);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window),
	glfwTerminate();
}

glm::vec3 point_on_torus(double alpha, double theta) {
	return glm::vec3(
		(1 + 0.5 * cos(alpha)) * cos(theta),
		0.5 * sin(alpha),
		(1 + 0.5 * cos(alpha)) * sin(theta)
	);
}


cylinder gen_hole(double inner_angle, double outer_angle, float size) {
	glm::vec3 torus_plane_normal = glm::vec3(0, 1.f, 0);

	glm::vec3 hole_pos = point_on_torus(inner_angle, outer_angle);
	glm::vec3 hole_inner_mid = glm::normalize(hole_pos - torus_plane_normal * glm::dot(hole_pos, torus_plane_normal)) * 1.f;
	glm::vec3 rot_vector = glm::normalize(point_on_torus(inner_angle + 1.5707f, outer_angle) - hole_inner_mid);
	glm::vec3	offset_ref_point = hole_pos - rot_vector * size;
	glm::vec3 offset_ref_vector = hole_inner_mid - offset_ref_point;
	glm::vec3 offset_vector = glm::normalize(hole_pos - hole_inner_mid) * (glm::length(offset_ref_vector) - 0.5f);

	cylinder c;
	c.bottom_mid = hole_pos - offset_vector;
	c.rad = size;
	c.height = offset_vector * 2.f;
	return c;
}
*/

