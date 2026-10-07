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
	using Clock = std::chrono::steady_clock;

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

	struct TimerSite {
		const char*   name;
		const char*   function;
		std::uint32_t line;
	};
 
	// Constant-initialisable and trivially destructible, so a thread_local instance
	// needs no TLS init wrapper or atexit registration. Partial windows are not
	// flushed at thread exit.
	struct TimerStats {
		Clock::duration total    = Clock::duration::zero();
		Clock::duration shortest = (Clock::duration::max)();   // parens: windows.h max macro
		Clock::duration longest  = Clock::duration::zero();
		std::uint32_t   count    = 0;
	};
 
	namespace Detail {
		// Defined out of line: keeps formatting/IO code off the inlined hot path.
		// noexcept: called from destructors; must never propagate.
		void LogTimer(const TimerSite& site, Clock::duration elapsed) noexcept;
		void LogTimerStats(const TimerSite& site, const TimerStats& stats) noexcept;
 
		// Per-thread countdown: no atomic RMW, no shared cache line, no division.
		// Fires on the N-th, 2N-th, ... call on the calling thread.
		template <std::uint32_t N>
		[[nodiscard]] inline bool ShouldSample(std::uint32_t& remaining) noexcept {
			static_assert(N > 0, "Sample interval must be > 0");
			if (--remaining != 0) [[likely]] {
				return false;
			}
			remaining = N;
			return true;
		}
	}
 
	class ScopedTimer {
	public:
		explicit ScopedTimer(const TimerSite& inSite, bool inEnabled = true) noexcept
			: site(&inSite)
			, start(inEnabled ? Clock::now() : Clock::time_point{})
			, enabled(inEnabled) {}
 
		~ScopedTimer() {
			if (enabled) {
				Detail::LogTimer(*site, Clock::now() - start);
			}
		}
 
		ScopedTimer(const ScopedTimer&)            = delete;
		ScopedTimer& operator=(const ScopedTimer&) = delete;
 
	private:
		const TimerSite*  site;
		Clock::time_point start;
		bool              enabled;
	};
 
	template <std::uint32_t N>
	class AverageTimer {
		static_assert(N > 0, "Averaging window must be > 0");
	public:
		AverageTimer(const TimerSite& inSite, TimerStats& inStats) noexcept
			: site(&inSite), stats(&inStats), start(Clock::now()) {}
 
		~AverageTimer() {
			const Clock::duration elapsed = Clock::now() - start;
			TimerStats& s = *stats;
			s.total += elapsed;
			if (elapsed < s.shortest) { s.shortest = elapsed; }
			if (elapsed > s.longest)  { s.longest  = elapsed; }
			if (++s.count == N) [[unlikely]] {
				Detail::LogTimerStats(*site, s);
				s = TimerStats{};
			}
		}
 
		AverageTimer(const AverageTimer&)            = delete;
		AverageTimer& operator=(const AverageTimer&) = delete;
 
	private:
		const TimerSite*  site;
		TimerStats*       stats;
		Clock::time_point start;
	};
}

// ---- Implementation macros (id is a pre-expanded __LINE__ value) --------------------
// __LINE__ rather than __COUNTER__: function-local statics are mangled by name, and
// __COUNTER__ differs between TUs, which would split statics in inline functions.
 
#define NCL_TIMER_SITE_IMPL(name, id) \
	static constinit const ::NCL::TimerSite XCONCAT(nclTimerSite_, id){ XSTR(name), __FUNCTION__, __LINE__ }
 
#define NCL_SCOPED_TIMER_IF_IMPL(name, cond, id) \
	NCL_TIMER_SITE_IMPL(name, id); \
	const ::NCL::ScopedTimer XCONCAT(nclTimer_, id){ XCONCAT(nclTimerSite_, id), static_cast<bool>(cond) }
 
#define NCL_SCOPED_TIMER_EVERY_N_IMPL(name, n, id) \
	static thread_local constinit ::std::uint32_t XCONCAT(nclTimerLeft_, id){ (n) }; \
	NCL_SCOPED_TIMER_IF_IMPL(name, ::NCL::Detail::ShouldSample<(n)>(XCONCAT(nclTimerLeft_, id)), id)
 
#define NCL_SCOPED_TIMER_AVG_IMPL(name, n, id) \
	NCL_TIMER_SITE_IMPL(name, id); \
	static thread_local constinit ::NCL::TimerStats XCONCAT(nclTimerStats_, id); \
	const ::NCL::AverageTimer<(n)> XCONCAT(nclTimer_, id){ XCONCAT(nclTimerSite_, id), XCONCAT(nclTimerStats_, id) }
 
// ---- Public macros (function scope only; terminate with ';' at the callsite) --------
// At most one timer macro per source line.
 
// Always logs on scope exit.
#define NCL_SCOPED_TIMER(name)            NCL_SCOPED_TIMER_IF_IMPL(name, true, __LINE__)
// Logs on scope exit only if cond (evaluated once, on entry) is true. No clock reads otherwise.
#define NCL_SCOPED_TIMER_IF(name, cond)   NCL_SCOPED_TIMER_IF_IMPL(name, cond, __LINE__)
// Times and logs every n-th execution of this callsite per thread. n: compile-time constant.
#define NCL_SCOPED_TIMER_EVERY_N(name, n) NCL_SCOPED_TIMER_EVERY_N_IMPL(name, n, __LINE__)
// Times every execution; logs avg/min/max every n executions, per thread. n: compile-time constant.
#define NCL_SCOPED_TIMER_AVG(name, n)     NCL_SCOPED_TIMER_AVG_IMPL(name, n, __LINE__)
