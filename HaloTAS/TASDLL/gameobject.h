#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

struct GameObject {
	uint32_t header_head; // "daeh"
	uint32_t tag_id;
	uint32_t ptr_a;
	uint32_t ptr_next_object;
	uint32_t ptr_previous_object;
	uint32_t header_tail; // "liat"
	uint32_t padding1[6];
	uint32_t counter;
	uint32_t padding2[16];
	float unit_x;
	float unit_y;
	float unit_z;
	uint32_t padding3[3];
	glm::quat rotationQuaternion;
	uint32_t padding3_2[7];
	float unit_x_ineffective;
	float unit_y_ineffective;
	float unit_z_ineffective;
	uint32_t padding4[13];
	float health;
	float shield;
};

GameObject* GetPlayerObject(std::vector<GameObject*> objects) {
	for (auto& v : objects) {
		if (v->tag_id == 3588) {
			return v;
		}
	}
	return nullptr;
}