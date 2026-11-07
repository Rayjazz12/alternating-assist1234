#include <Geode/Geode.hpp>
#include "Assist.hpp"
#include "AssistPopup.hpp"

using namespace geode::prelude;

// Load saved on/off + stage when the mod loads.
$execute {
	assist::load();
}

// ---------------------------------------------------------------
// Pause menu button that opens the Alternating Assist GUI
// ---------------------------------------------------------------
#include <Geode/modify/PauseLayer.hpp>
class $modify(AssistPauseLayer, PauseLayer) {
	void customSetup() {
		PauseLayer::customSetup();

		auto spr = CircleButtonSprite::create(
			CCLabelBMFont::create("AA", "bigFont.fnt"),
			CircleBaseColor::Green,
			CircleBaseSize::Medium
		);
		auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(AssistPauseLayer::onAssist));
		btn->setID("assist-button"_spr);

		if (auto menu = this->getChildByID("right-button-menu")) {
			menu->addChild(btn);
			menu->updateLayout();
		} else {
			// fallback if the node ID isn't found
			auto win = CCDirector::sharedDirector()->getWinSize();
			auto menu2 = CCMenu::create();
			menu2->setPosition({ win.width - 35.f, 60.f });
			btn->setPosition({ 0.f, 0.f });
			menu2->addChild(btn);
			this->addChild(menu2, 10);
		}
	}

	void onAssist(CCObject*) {
		AssistPopup::create()->show();
	}
};

// ---------------------------------------------------------------
// The assist itself: while you HOLD jump, we auto-click for you.
// ---------------------------------------------------------------
#include <Geode/modify/GJBaseGameLayer.hpp>
class $modify(AssistGameLayer, GJBaseGameLayer) {
	// Watch the real button presses so we know when you're holding.
	void handleButton(bool down, int button, bool isPlayer1) {
		GJBaseGameLayer::handleButton(down, button, isPlayer1);

		if (button != 1) return;
		if (!typeinfo_cast<PlayLayer*>(this)) return; // not the editor

		auto& h = assist::holds[isPlayer1 ? 0 : 1];
		h.held = down;
		h.down = down;
		h.timer = 0.f;
	}

	void update(float dt) {
		if (assist::enabled && typeinfo_cast<PlayLayer*>(this)) {
			for (int i = 0; i < 2; i++) {
				auto& h = assist::holds[i];
				if (!h.held) continue;

				bool toggle = false;
				float cps = assist::CPS[assist::stage];
				if (cps <= 0.f) {
					toggle = true; // Max: flip every frame
				} else {
					h.timer += dt;
					float half = 0.5f / cps; // press + release = 1 click
					if (h.timer >= half) {
						h.timer = std::fmod(h.timer, half);
						toggle = true;
					}
				}

				if (toggle) {
					h.down = !h.down;
					// call the ORIGINAL so we don't re-trigger our own hook
					GJBaseGameLayer::handleButton(h.down, 1, i == 0);
				}
			}
		}
		GJBaseGameLayer::update(dt);
	}
};

// Clear the hold state on death/restart and when pausing.
#include <Geode/modify/PlayLayer.hpp>
class $modify(AssistPlayLayer, PlayLayer) {
	void resetLevel() {
		assist::resetHolds();
		PlayLayer::resetLevel();
	}

	void pauseGame(bool p0) {
		assist::resetHolds();
		PlayLayer::pauseGame(p0);
	}
};
