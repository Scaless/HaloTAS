#include "livesplit.h"
#include <Windows.h>
#include <memoryapi.h>


livesplit::livesplit()
{
	LPVOID desiredAddr = reinterpret_cast<LPVOID>(0x44000000);
	SIZE_T poolSize = sizeof(livesplit_export);
	memPool = VirtualAlloc(desiredAddr, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

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
	memcpy_s(memPool, sizeof(newExport), &newExport, sizeof(newExport));
}
