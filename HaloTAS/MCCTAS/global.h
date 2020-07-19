#pragma once
class global
{
private:
	bool mKillFlag;

	static global& get() {
		static global instance;
		return instance;
	}

	global() = default;
	~global() = default;
public:

	static void kill_mcctas();
	static bool is_kill_set();
};
