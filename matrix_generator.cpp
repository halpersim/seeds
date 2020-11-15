/*
#include "matrix_generator.h"
#include <list>
#include "utils.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "soldier.h"
#include "constants.h"
#include "data_supplier.h"
#include <string>



static glm::mat4 align_to_axis(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z){
	float mat[16], * ptr;

	std::memset(mat, 0, sizeof(float) * 16);
	mat[15] = 1;
	ptr = mat;

	std::memcpy(ptr, glm::value_ptr(x), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(y), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(z), sizeof(float) * 3);

	return glm::make_mat4(mat);
}

static void generate_lookat_matrix(float* ptr, glm::vec3 pos, glm::vec3 dir, glm::vec3 normal) {
	glm::vec3 f = glm::normalize(dir);
	glm::vec3 u = glm::normalize(normal);
	glm::vec3 s = glm::normalize(glm::cross(f, u));
	u = glm::normalize(glm::cross(s, f));

	std::memcpy(ptr, glm::value_ptr(u), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(f), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(s), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(pos), sizeof(float) * 3);
}

static glm::mat4 generate_matrix(float* ptr, const DTO::defender* def) {
	glm::vec3 pos = def->host_planet->get_pos(def->coord, def->coord.z);
	glm::vec3 next = def->host_planet->get_pos(def->coord + def->direction, def->coord.z + def->direction.z);

	generate_lookat_matrix(ptr, pos, next - pos, def->host_planet->get_normal(def->coord));

	return glm::scale(glm::make_mat4(ptr), glm::vec3(1, 1 / def->damage, 1) * def->health * Constants::SOLDIER_SCALE);
}

static glm::mat4 generate_matrix(float* ptr, const DTO::attacker* att) {
	glm::vec3 pos, next, normal;

	if (att->host_planet) {
		pos = att->host_planet->get_pos(att->coord, att->coord.z);
		next = att->host_planet->get_pos(att->coord + att->direction, att->coord.z + att->direction.z);
		normal = att->host_planet->get_normal(att->coord);
	}
	else {
		pos = att->pos;
		next = att->pos + att->direction;
		normal = att->normal;
	}

	generate_lookat_matrix(ptr, pos, next - pos, normal);

	return glm::scale(glm::make_mat4(ptr), glm::vec3(att->health, att->damage, att->health) * Constants::SOLDIER_SCALE);
}

template<class T>
static glm::mat4 generate_matrix(float* ptr, DTO::tree<T>* tree) {
	glm::vec3 s = glm::normalize(tree->host_planet.get_tangent_alpha(tree->ground.coords));
	glm::vec3 u = glm::normalize(tree->ground.height);
	glm::vec3 f = glm::normalize(tree->host_planet.get_tangent_theta(tree->ground.coords));

	std::memcpy(ptr, glm::value_ptr(s), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(u), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(f), sizeof(float) * 3);
	ptr += 4;
	std::memcpy(ptr, glm::value_ptr(tree->ground.bottom_mid), sizeof(float) * 3);

	return glm::scale(glm::make_mat4(ptr), glm::vec3(1, (int)(tree->size) % 5, 1));
}

template<class T>
std::list<glm::mat4> Rendering::generate_matrix_pallet(const std::list<T*>& in) {
	std::list<glm::mat4> ret;
	float mat[16];

	memset(mat, 0, sizeof(float) * 16);
	mat[15] = 1.f;
	for (T* dto : in) {
		ret.push_back(generate_matrix(mat, dto));
	}
	return ret;
}


static glm::mat4 get_tree_node_side_data(Loki::Type2Type<DTO::attacker> dummy, int side){
	glm::vec3 y = glm::make_vec3(&Rendering::Models::data_supplier<DTO::attacker>::normals[(side+1)*3]);
	glm::vec3 pos = glm::vec3(0.f);
	glm::vec3 x_ref;

	for(int i = 0; i<3; i++){	
		pos += glm::make_vec3(&Rendering::Models::data_supplier<DTO::attacker>::positions
													[(Rendering::Models::data_supplier<DTO::attacker>::position_indices[(side+2)*3 + i])*3]);
		if(i == 0)
			x_ref = pos;
	}
	pos *= 0.33333f;
	glm::vec3 x = glm::normalize(x_ref - pos);
	glm::vec3 z = glm::normalize(glm::cross(x, y));

	return glm::translate(align_to_axis(x, y, z), pos);
}

template<class T>
static std::list<glm::mat4> generate_matrix_tree_recursive(const std::vector<DTO::tree<T>>& other, DTO::tree_node<T> me, int side = -1){
	glm::mat4 my_transform = me.id ? glm::scale(get_tree_node_side_data(Loki::Type2Type<T>, side), glm::vec3(me.size/5)) : glm::mat4(1.f);
	std::list<glm::mat4> pallet;
		
	for(int i = 0; i< me.branch_size; i++){
		if(me.next[i] != -1){
			std::list<glm::mat4> value = generate_matrix_tree_recursive(other, other.at(me.next[i]), i);
			std::copy(value.begin(), value.end(), std::back_inserter(pallet));
		}
	}

	std::for_each(pallet.begin(), pallet.end(), [&my_transform](glm::mat4& m){ m = my_transform * m; });
	return pallet;
}		

template<class T>
static std::list<glm::mat4> generate_matrix_tree_trunk(const DTO::tree<T>* tree) {
	glm::mat4 model_to_world;
		
	glm::vec3 x = tree->host_planet.get_tangent_alpha(tree->ground.coords);
	glm::vec3 y = tree->host_planet.get_normal(tree->ground.coords);
	glm::vec3 z = tree->host_planet.get_tangent_theta(tree->ground.coords);

	model_to_world = glm::translate(align_to_axis(x, y, z), tree->ground.bottom_mid);
		
	std::list<glm::mat4> ret = generate_matrix_tree_recursive(tree->nodes, tree->nodes.front());

	std::for_each(ret.begin(), ret.end(), [&model_to_world](glm::mat4& m){ m = model_to_world * m; });
		
	return ret;
}

template<class T>
std::list<glm::mat4> Rendering::generate_matrix_pallet_tree_trunk(const std::list<DTO::tree<T>*>& in) {
	std::list<glm::mat4> ret;

	for (DTO::tree<T>* tree : in) {
		std::list<glm::mat4> list = generate_matrix_tree_trunk(tree);
		std::for_each(list.begin(), list.end(), [&ret](glm::mat4& mat) {ret.push_back(mat); });
	}
	return ret;
}

template<class T>
std::list<glm::mat4> Rendering::generate_matrix_pallet_tree_soldier(const std::list<DTO::tree<T>*>& in) {

}
*/