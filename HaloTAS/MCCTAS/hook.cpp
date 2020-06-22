#include "hook.h"
#include <detours/detours.h>

void detours_error(LONG detourResult) {
	if (detourResult != NO_ERROR) {
		throw;
	}
}

void hook::attach()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	detours_error(DetourAttach(original_function, replaced_function));
	detours_error(DetourTransactionCommit());
}

void hook::detach()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(original_function, replaced_function);
	DetourTransactionCommit();
}
