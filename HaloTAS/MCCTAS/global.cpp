#include "global.h"

void global::kill_mcctas()
{
	global::get().mKillFlag = true;
}

bool global::is_kill_set()
{
	return global::get().mKillFlag;
}
