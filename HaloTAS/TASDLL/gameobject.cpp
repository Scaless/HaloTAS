#include "gameobject.h"

GameObject* GetPlayerObject(std::vector<GameObject*> objects) {
	for (auto& v : objects) {
		if (v->tag_id == 3588) {
			return v;
		}
	}
	return nullptr;
}