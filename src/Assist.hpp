#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>

namespace assist {
	constexpr int STAGE_COUNT = 5;   // stages 1, 2, 3, 4, Max
	constexpr int MAX_STAGE = 4;     // index of "Max"

	// Clicks per second for each stage. 0 = toggle every frame (Max).
	// Edit these numbers to tune how fast each stage is.
	constexpr float CPS[STAGE_COUNT] = { 8.f, 12.f, 16.f, 20.f, 0.f };

	inline bool enabled = false;
	inline int stage = 0; // 0..4

	inline void load() {
		auto mod = geode::Mod::get();
		enabled = mod->getSavedValue<bool>("enabled", false);
		stage = std::clamp(mod->getSavedValue<int>("stage", 0), 0, MAX_STAGE);
	}

	inline void setEnabled(bool value) {
		enabled = value;
		geode::Mod::get()->setSavedValue("enabled", value);
	}

	inline void setStage(int value) {
		stage = std::clamp(value, 0, MAX_STAGE);
		geode::Mod::get()->setSavedValue("stage", stage);
	}

	inline std::string stageLabel(int s) {
		return s >= MAX_STAGE ? "Max" : std::to_string(s + 1);
	}

	inline std::string describeStage(int s) {
		if (s >= MAX_STAGE) return "Max: spams every frame while held";
		return "Stage " + std::to_string(s + 1) + ": ~" +
			std::to_string(static_cast<int>(CPS[s])) + " clicks/sec while held";
	}

	// Per-player hold state (index 0 = player 1, 1 = player 2)
	struct Hold {
		int pressed = 0;     // how many REAL presses are currently held (alternating = 2 fingers)
		bool down = false;   // the button state we last sent to the game
		float timer = 0.f;
		bool held() const { return pressed > 0; }
	};
	inline Hold holds[2];

	// true while WE are sending a click, so our hook ignores our own clicks
	inline bool injecting = false;

	inline void resetHolds() {
		holds[0] = Hold{};
		holds[1] = Hold{};
	}
}
