#pragma once
#include "pch.h"

class speedometer
{
private:
	int32_t mDataTickRate = 0;
	boost::circular_buffer<glm::vec3> mDataPoints;
	double mSumTotalDistanceXYTravelled = 0.0f;
	double mSumTotalDistanceXYZTravelled = 0.0f;
	size_t mSumTotalDistanceCount = 0;
public:
	enum class axis {
		X,
		Y,
		Z,
		XY,
		XYZ
	};

	speedometer(size_t maxSecondsOfData, int ticksPerSecond) : mDataTickRate(ticksPerSecond)
	{
		auto maxDataPoints = ticksPerSecond * maxSecondsOfData;
		mDataPoints.resize(maxDataPoints);
		mDataPoints.clear();
	}
	~speedometer() = default;

	void clear();
	double average_speed_per_second(axis a);
	void push_back(const glm::vec3& position);
	std::vector<float> display_data(axis a);
};
