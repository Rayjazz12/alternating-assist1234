#pragma once
#include <Geode/Geode.hpp>
#include <array>
#include "Assist.hpp"

using namespace geode::prelude;

class AssistPopup : public FLAlertLayer {
protected:
	CCMenuItemToggler* m_toggler = nullptr;
	std::array<ButtonSprite*, assist::STAGE_COUNT> m_stageSprites{};
	CCLabelBMFont* m_info = nullptr;

	bool init() {
		if (!FLAlertLayer::init(150)) return false;

		m_noElasticity = true;
		this->setTouchEnabled(true);
		this->setKeypadEnabled(true);

		auto win = CCDirector::sharedDirector()->getWinSize();
		float cx = win.width / 2.f;
		float cy = win.height / 2.f;

		// background
		auto bg = CCScale9Sprite::create("GJ_square01.png");
		bg->setContentSize({ 320.f, 240.f });
		bg->setPosition({ cx, cy });
		m_mainLayer->addChild(bg);

		// title
		auto title = CCLabelBMFont::create("Alternating Assist", "goldFont.fnt");
		title->setScale(0.8f);
		title->setPosition({ cx, cy + 97.f });
		m_mainLayer->addChild(title, 5);

		m_buttonMenu = CCMenu::create();
		m_buttonMenu->setPosition({ 0.f, 0.f });
		m_buttonMenu->setTouchPriority(-504);
		m_mainLayer->addChild(m_buttonMenu, 10);

		// on / off toggle
		m_toggler = CCMenuItemToggler::create(
			CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
			CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
			this, menu_selector(AssistPopup::onToggle)
		);
		m_toggler->setPosition({ cx - 70.f, cy + 52.f });
		m_toggler->toggle(assist::enabled);
		m_buttonMenu->addChild(m_toggler);

		auto toggleLabel = CCLabelBMFont::create("Assist On / Off", "bigFont.fnt");
		toggleLabel->setScale(0.5f);
		toggleLabel->setAnchorPoint({ 0.f, 0.5f });
		toggleLabel->setPosition({ cx - 48.f, cy + 52.f });
		m_mainLayer->addChild(toggleLabel, 5);

		// stage label
		auto stageLabel = CCLabelBMFont::create("Stage", "bigFont.fnt");
		stageLabel->setScale(0.5f);
		stageLabel->setPosition({ cx, cy + 14.f });
		m_mainLayer->addChild(stageLabel, 5);

		// stage buttons: 1 2 3 4 Max
		for (int i = 0; i < assist::STAGE_COUNT; i++) {
			auto spr = ButtonSprite::create(
				assist::stageLabel(i).c_str(), 46, true, "bigFont.fnt",
				"GJ_button_04.png", 30.f, 0.8f
			);
			m_stageSprites[i] = spr;
			auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(AssistPopup::onStage));
			btn->setTag(i);
			btn->setPosition({ cx + (i - 2) * 56.f, cy - 18.f });
			m_buttonMenu->addChild(btn);
		}

		// info text
		m_info = CCLabelBMFont::create("", "chatFont.fnt");
		m_info->setScale(0.8f);
		m_info->setPosition({ cx, cy - 58.f });
		m_mainLayer->addChild(m_info, 5);

		// OK button
		auto okBtn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("OK"), this, menu_selector(AssistPopup::onClose)
		);
		okBtn->setPosition({ cx, cy - 92.f });
		m_buttonMenu->addChild(okBtn);

		this->refresh();
		return true;
	}

	void refresh() {
		for (int i = 0; i < assist::STAGE_COUNT; i++) {
			m_stageSprites[i]->updateBGImage(i == assist::stage ? "GJ_button_01.png" : "GJ_button_04.png");
		}
		m_info->setString(assist::describeStage(assist::stage).c_str());
	}

	void onToggle(CCObject* sender) {
		// the toggler flips after the callback, so isToggled() is the OLD state
		bool nowOn = !static_cast<CCMenuItemToggler*>(sender)->isToggled();
		assist::setEnabled(nowOn);
		assist::resetHolds();
	}

	void onStage(CCObject* sender) {
		assist::setStage(static_cast<CCNode*>(sender)->getTag());
		this->refresh();
	}

	void onClose(CCObject*) {
		this->setKeypadEnabled(false);
		this->removeFromParentAndCleanup(true);
	}

	void keyBackClicked() override {
		this->onClose(nullptr);
	}

public:
	static AssistPopup* create() {
		auto ret = new AssistPopup();
		if (ret && ret->init()) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
};
