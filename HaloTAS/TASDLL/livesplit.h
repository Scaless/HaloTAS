#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>

struct livesplit_export
{
private:
	int32_t version = 1; // Bump this when this struct changes!
public:
	char currentMap[32];
};

class livesplit
{
private:
	void* memPool = nullptr;
public:
	livesplit();
	~livesplit();

	void update_export(const livesplit_export& newExport);
};

