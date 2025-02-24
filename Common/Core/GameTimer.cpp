/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "pch.h"
#include "GameTimer.h"

using namespace NCL;

GameTimer::GameTimer(void) {
	firstPoint = std::chrono::high_resolution_clock::now();
	nowPoint = firstPoint;
	Tick();
}

double	GameTimer::GetTotalTimeSeconds()	const {
	Timepoint time = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> diff = time - firstPoint;

	return diff.count();
};

double	GameTimer::GetTotalTimeMSec()		const {
	Timepoint time = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> diff = time - firstPoint;

	return diff.count();
}

void	GameTimer::Tick() {
	Timepoint latestTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<float> diff = latestTime - nowPoint;

	nowPoint = latestTime;

	timeDelta = diff.count();
}

NCL::ScopedTimer::~ScopedTimer() {
	const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start);
	const std::chrono::duration<double, std::milli> fp_ms = Clock::now() - start;
	const auto count = duration.count();
	float c = 2000;
	LOG_INFO("{} took {}ms", label, fp_ms.count());
}
