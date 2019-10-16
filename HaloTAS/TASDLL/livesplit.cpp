#include "livesplit.h"
#include "tas_logger.h"
#include <Windows.h>
#include <memoryapi.h>
#include "halo_constants.h"

using namespace halo::addr;

livesplit::livesplit()
{
	LPVOID desiredAddr = reinterpret_cast<LPVOID>(0x44000000);
	SIZE_T poolSize = sizeof(livesplit_export);
	memPool = VirtualAlloc(desiredAddr, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (memPool == nullptr) {
		tas_logger::error("VirtualAlloc failed to allocate memory for livesplit at address %p", desiredAddr);
		return;
	}

	livesplit_export defaultExport = {};
	memcpy_s(memPool, sizeof(defaultExport), &defaultExport, sizeof(defaultExport));
}

livesplit::~livesplit()
{
	if (memPool != nullptr) {
		VirtualFree(memPool, 0, MEM_RELEASE);
	}
}

void livesplit::update_export(const livesplit_export& newExport)
{
	(*HUD_PLAYER_HEALTH < 1.0f) ? *HUD_FLAGS |= 1 << 1 : *HUD_FLAGS &= ~(1 << 1);
	(*HUD_PLAYER_SHIELD < 1.0f && *HUD_PLAYER_SHIELD != -1) ? *HUD_FLAGS |= 1 << 3 : *HUD_FLAGS &= ~(1 << 3);

	// Update the pool
	if (memPool != nullptr) {
		memcpy_s(memPool, sizeof(newExport), &newExport, sizeof(newExport));
	}
}
