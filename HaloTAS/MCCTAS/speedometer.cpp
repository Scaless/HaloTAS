#include "pch.h"
#include "speedometer.h"

void speedometer::clear()
{
	mDataPoints.clear();
	mSumTotalDistanceXYTravelled = 0.0f;
	mSumTotalDistanceXYZTravelled = 0.0f;
	mSumTotalDistanceCount = 0;
}

float vec3_xy_distance(glm::vec3 a, glm::vec3 b) {
	return glm::distance(
		glm::vec2(a.x, a.y),
		glm::vec2(b.x, b.y)
	);
}

double speedometer::average_speed_per_second(axis a)
{
	// Doesn't make sense to have a speed if we don't have at least 2 data points
	if (mDataPoints.size() < 2)
		return 0.0f;

	// Get the sum of distances between all data points
	double totalDistance = 0.0f;
	glm::vec3 lastPosition = mDataPoints.front();
	for (auto& currentPosition : mDataPoints) {
		switch (a)
		{
		case speedometer::axis::X:
			totalDistance += glm::distance(lastPosition.x, currentPosition.x);
			break;
		case speedometer::axis::Y:
			totalDistance += glm::distance(lastPosition.y, currentPosition.y);
			break;
		case speedometer::axis::Z:
			totalDistance += glm::distance(lastPosition.z, currentPosition.z);
			break;
		case speedometer::axis::XY:
			totalDistance += vec3_xy_distance(lastPosition, currentPosition);
			break;
		case speedometer::axis::XYZ:
			totalDistance += glm::distance(lastPosition, currentPosition);
			break;
		}
		lastPosition = currentPosition;
	}

	auto speedCount = mDataPoints.size() - 1; // 2 data points = 1 speed value, etc
	return (totalDistance / speedCount) * mDataTickRate;
}

void speedometer::push_back(const glm::vec3& newPosition)
{
	if (mDataPoints.size() > 0) {
		auto back = mDataPoints.back();
		mSumTotalDistanceXYTravelled += vec3_xy_distance(back, newPosition);
		mSumTotalDistanceXYZTravelled += glm::distance(back, newPosition);
		mSumTotalDistanceCount++;
	}

	mDataPoints.push_back(newPosition);
}

std::vector<float> speedometer::display_data(axis axis)
{
	std::vector<float> data;

	glm::vec3 lastPosition = mDataPoints.front();
	bool skipFirst = false;
	for (auto& currentPosition : mDataPoints) {
		if (!skipFirst) {
			skipFirst = true;
			continue;
		}

		switch (axis)
		{
		case speedometer::axis::X:
			data.push_back(glm::distance(lastPosition.x, currentPosition.x));
			break;
		case speedometer::axis::Y:
			data.push_back(glm::distance(lastPosition.y, currentPosition.y));
			break;
		case speedometer::axis::Z:
			data.push_back(glm::distance(lastPosition.z, currentPosition.z));
			break;
		case speedometer::axis::XY:
			data.push_back(vec3_xy_distance(lastPosition, currentPosition));
			break;
		case speedometer::axis::XYZ:
			data.push_back(glm::distance(lastPosition, currentPosition));
			break;
		}
		
		lastPosition = currentPosition;
	}

	return data;
}
