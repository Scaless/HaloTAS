#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

enum class UNIT_FLAGS : uint32_t {
	IS_FLOOD_REVIVER = 2 << 12
};

struct GameObject {
	uint32_t header_head; // "daeh"
	uint32_t tag_id;
	uint32_t ptr_a;
	uint32_t ptr_next_object;
	uint32_t ptr_previous_object;
	uint32_t header_tail; // "liat"
	uint32_t padding1[5];
	uint32_t counter;
	uint32_t padding2[17];
	float unit_x;
	float unit_y;
	float unit_z;
	float momentum[3];
	float rotationQuaternion[4];
	uint32_t padding3_2[7];
	float unit_x_ineffective;
	float unit_y_ineffective;
	float unit_z_ineffective;
	uint32_t padding4[13];
	float health;
	float shield;
	uint32_t padding5[71];
	uint32_t unit_flags;
};

GameObject* GetPlayerObject(std::vector<GameObject*> objects);