/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

#include <chrono>

namespace NCL {
	typedef  std::chrono::time_point<std::chrono::high_resolution_clock>  Timepoint;
	using Clock = std::chrono::high_resolution_clock;

	class GameTimer {
	public:
		GameTimer(void);
		~GameTimer(void) = default;

		double	GetTotalTimeSeconds()	const;
		double	GetTotalTimeMSec()		const;

		float	GetTimeDeltaSeconds()	const { return timeDelta; };
		float	GetTimeDeltaMSec()		const { return timeDelta * 1000.0f; };

		void Tick();
	protected:
		float		timeDelta;
		Timepoint	firstPoint;
		Timepoint	nowPoint;
	};

#define NCL_SCOPED_TIMER(name) const ScopedTimer _nclScopedTimer(XSTR(name) ":" __FUNCTION__ ":" XSTR(__LINE__));

	class ScopedTimer {
	public:
		ScopedTimer(std::string_view inLabel = "Timer") : label(inLabel), start(Clock::now()) {}

		~ScopedTimer();

	private:
		std::string label;
		Timepoint start;
	};
}

