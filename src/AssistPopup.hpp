#pragma once
#include <Geode/Geode.hpp>
#include <array>
#include <cmath>
#include <string>
#include "Assist.hpp"

using namespace geode::prelude;

// Glassy light-blue popup: Off / 1 / 2 / 3 / 4 / Max pills, a speed meter
// that glides to the chosen stage, and open/close animations.
class AssistPopup : public FLAlertLayer {
protected:
	static constexpr int PILLS = assist::STAGE_COUNT + 1; // Off, 1, 2, 3, 4, Max

	// layout (all relative to the centre of the screen)
	static constexpr float BAR_W = 300.f;
	static constexpr float BAR_H = 8.f;
	static constexpr float BAR_Y = -36.f;

	CCNode* m_root = nullptr;
	std::array<CCMenuItemSpriteExtra*, PILLS> m_pillBtn{};
	std::array<CCScale9Sprite*, PILLS> m_pillBG{};
	std::array<CCLabelBMFont*, PILLS> m_pillLabel{};
	CCLabelBMFont* m_stageTitle = nullptr;
	CCLabelBMFont* m_stageInfo = nullptr;
	CCScale9Sprite* m_barFill = nullptr;
	float m_barCur = 0.f;
	float m_barTarget = 0.f;
	bool m_closing = false;

	// ---- palette ---------------------------------------------------
	static ccColor3B accent()     { return ccc3(66, 140, 238); }
	static ccColor3B panelColor() { return ccc3(172, 222, 248); }
	static ccColor3B mutedText()  { return ccc3(84, 118, 156); }

	// ---- helpers ---------------------------------------------------
	static CCScale9Sprite* box(CCSize size, ccColor3B col, GLubyte opacity) {
		auto s = CCScale9Sprite::create("square02_001.png", { 0.f, 0.f, 80.f, 80.f });
		s->setContentSize(size);
		s->setColor(col);
		s->setOpacity(opacity);
		return s;
	}

	static std::string pillText(int i) {
		return i == 0 ? "Off" : assist::stageLabel(i - 1);
	}

	static int selectedIndex() {
		return assist::enabled ? assist::stage + 1 : 0;
	}

	static std::string speedText(int s) {
		if (s >= assist::MAX_STAGE) return "Spams every frame while you hold";
		return "~" + std::to_string(static_cast<int>(assist::CPS[s])) + " clicks/sec while you hold";
	}

	// ---- build -----------------------------------------------------
	bool init() {
		if (!FLAlertLayer::init(150)) return false;

		m_noElasticity = true;
		this->setTouchEnabled(true);
		this->setKeypadEnabled(true);

		auto win = CCDirector::sharedDirector()->getWinSize();

		// everything lives under m_root so the whole panel can scale around its centre
		m_root = CCNode::create();
		m_root->setPosition({ win.width / 2.f, win.height / 2.f });
		m_mainLayer->addChild(m_root);

		m_buttonMenu = CCMenu::create();
		m_buttonMenu->setPosition({ 0.f, 0.f });
		m_buttonMenu->setTouchPriority(-504);
		m_root->addChild(m_buttonMenu, 10);

		// shadow + panel + glass sheen
		auto shadow = box({ 368.f, 232.f }, ccc3(0, 0, 0), 70);
		shadow->setPosition({ 0.f, -4.f });
		m_root->addChild(shadow, 0);

		auto frame = box({ 364.f, 228.f }, ccc3(255, 255, 255), 235);
		m_root->addChild(frame, 1);

		auto panel = box({ 354.f, 218.f }, panelColor(), 255);
		m_root->addChild(panel, 2);

		auto sheen = box({ 340.f, 64.f }, ccc3(255, 255, 255), 55);
		sheen->setPosition({ 0.f, 76.f });
		m_root->addChild(sheen, 3);

		// header
		auto title = CCLabelBMFont::create("Alternating Assist", "bigFont.fnt");
		title->setScale(0.55f);
		title->setColor(accent());
		title->setAnchorPoint({ 0.f, 0.5f });
		title->setPosition({ -160.f, 92.f });
		m_root->addChild(title, 5);

		auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
		closeSpr->setScale(0.65f);
		auto closeBtn = CCMenuItemSpriteExtra::create(closeSpr, this, menu_selector(AssistPopup::onCloseBtn));
		closeBtn->setPosition({ 156.f, 92.f });
		m_buttonMenu->addChild(closeBtn);

		auto divider = box({ 330.f, 2.f }, ccc3(255, 255, 255), 130);
		divider->setPosition({ 0.f, 75.f });
		m_root->addChild(divider, 4);

		// stage card
		auto stageCard = box({ 330.f, 112.f }, ccc3(255, 255, 255), 105);
		stageCard->setPosition({ 0.f, 6.f });
		m_root->addChild(stageCard, 4);

		auto stageHeader = CCLabelBMFont::create("STAGE", "bigFont.fnt");
		stageHeader->setScale(0.32f);
		stageHeader->setColor(mutedText());
		stageHeader->setAnchorPoint({ 0.f, 0.5f });
		stageHeader->setPosition({ -152.f, 50.f });
		m_root->addChild(stageHeader, 5);

		// pills: Off 1 2 3 4 Max
		for (int i = 0; i < PILLS; i++) {
			auto cont = CCNode::create();
			cont->setContentSize({ 42.f, 26.f });

			auto bg = box({ 42.f, 26.f }, ccc3(255, 255, 255), 150);
			bg->setPosition({ 21.f, 13.f });
			cont->addChild(bg, 0);

			auto lbl = CCLabelBMFont::create(pillText(i).c_str(), "bigFont.fnt");
			lbl->setScale(0.45f);
			lbl->setPosition({ 21.f, 13.5f });
			cont->addChild(lbl, 1);

			auto btn = CCMenuItemSpriteExtra::create(cont, this, menu_selector(AssistPopup::onPill));
			btn->setTag(i);
			btn->setPosition({ (i - 2.5f) * 50.f, 24.f });
			m_buttonMenu->addChild(btn);

			m_pillBtn[i] = btn;
			m_pillBG[i] = bg;
			m_pillLabel[i] = lbl;
		}

		// big stage name + description
		m_stageTitle = CCLabelBMFont::create("", "bigFont.fnt");
		m_stageTitle->setScale(0.6f);
		m_stageTitle->setPosition({ 0.f, -4.f });
		m_root->addChild(m_stageTitle, 5);

		m_stageInfo = CCLabelBMFont::create("", "chatFont.fnt");
		m_stageInfo->setScale(0.8f);
		m_stageInfo->setColor(mutedText());
		m_stageInfo->setPosition({ 0.f, -19.f });
		m_root->addChild(m_stageInfo, 5);

		// speed meter
		auto track = box({ BAR_W, BAR_H }, ccc3(255, 255, 255), 150);
		track->setPosition({ 0.f, BAR_Y });
		m_root->addChild(track, 5);

		m_barFill = box({ BAR_H, BAR_H }, accent(), 255);
		m_barFill->setPosition({ -BAR_W / 2.f, BAR_Y });
		m_root->addChild(m_barFill, 6);

		// footer hint
		auto hintCard = box({ 330.f, 36.f }, ccc3(255, 255, 255), 105);
		hintCard->setPosition({ 0.f, -76.f });
		m_root->addChild(hintCard, 4);

		auto hint = CCLabelBMFont::create("Hold jump in a level - the assist spams for you", "chatFont.fnt");
		hint->setScale(0.75f);
		hint->setColor(mutedText());
		hint->setPosition({ 0.f, -76.f });
		m_root->addChild(hint, 5);

		this->refresh(false);

		// bar starts empty and glides to the current stage
		m_barCur = 0.f;
		this->schedule(schedule_selector(AssistPopup::tick));

		this->playOpenAnimation();
		return true;
	}

	// ---- state -----------------------------------------------------
	void refresh(bool animate) {
		int sel = selectedIndex();

		for (int i = 0; i < PILLS; i++) {
			bool on = (i == sel);
			ccColor3B bgCol = on ? accent() : ccc3(255, 255, 255);
			GLubyte bgOp = on ? 255 : 150;
			ccColor3B txtCol = on ? ccc3(255, 255, 255) : mutedText();

			auto bg = m_pillBG[i];
			auto lbl = m_pillLabel[i];

			if (animate) {
				bg->stopAllActions();
				lbl->stopAllActions();
				bg->runAction(CCTintTo::create(0.18f, bgCol.r, bgCol.g, bgCol.b));
				bg->runAction(CCFadeTo::create(0.18f, bgOp));
				lbl->runAction(CCTintTo::create(0.18f, txtCol.r, txtCol.g, txtCol.b));

				if (on) {
					auto img = m_pillBtn[i]->getNormalImage();
					img->stopActionByTag(77);
					auto pulse = CCSequence::create(
						CCScaleTo::create(0.07f, 1.14f),
						CCEaseBackOut::create(CCScaleTo::create(0.22f, 1.f)),
						nullptr
					);
					pulse->setTag(77);
					img->runAction(pulse);
				}
			} else {
				bg->setColor(bgCol);
				bg->setOpacity(bgOp);
				lbl->setColor(txtCol);
			}
		}

		// big title + description
		if (sel == 0) {
			m_stageTitle->setString("OFF");
			m_stageTitle->setColor(mutedText());
			m_stageInfo->setString("Assist is disabled");
			m_barTarget = 0.f;
		} else {
			int s = sel - 1;
			m_stageTitle->setString(s >= assist::MAX_STAGE ? "MAX" : ("STAGE " + assist::stageLabel(s)).c_str());
			m_stageTitle->setColor(accent());
			m_stageInfo->setString(speedText(s).c_str());
			m_barTarget = s >= assist::MAX_STAGE ? 1.f : assist::CPS[s] / 24.f;
		}

		if (animate) {
			m_stageTitle->stopAllActions();
			m_stageInfo->stopAllActions();
			m_stageTitle->setOpacity(0);
			m_stageInfo->setOpacity(0);
			m_stageTitle->runAction(CCFadeTo::create(0.18f, 255));
			m_stageInfo->runAction(CCFadeTo::create(0.22f, 255));
		}
	}

	// glide the speed meter toward its target (frame-rate independent)
	void tick(float dt) {
		float k = 1.f - std::exp(-14.f * dt);
		m_barCur += (m_barTarget - m_barCur) * k;

		float w = m_barCur * BAR_W;
		bool show = w > BAR_H;
		m_barFill->setVisible(show);
		if (show) {
			m_barFill->setContentSize({ w, BAR_H });
			m_barFill->setPosition({ -BAR_W / 2.f + w / 2.f, BAR_Y });
		}
	}

	// ---- animations ------------------------------------------------
	void playOpenAnimation() {
		this->setOpacity(0);
		this->runAction(CCFadeTo::create(0.2f, 150));

		m_root->setScale(0.82f);
		m_root->runAction(CCEaseBackOut::create(CCScaleTo::create(0.28f, 1.f)));

		// pills pop in one after another
		for (int i = 0; i < PILLS; i++) {
			auto img = m_pillBtn[i]->getNormalImage();
			img->setScale(0.f);
			img->runAction(CCSequence::create(
				CCDelayTime::create(0.08f + 0.04f * i),
				CCEaseBackOut::create(CCScaleTo::create(0.25f, 1.f)),
				nullptr
			));
		}
	}

	void finishClose() {
		this->setKeypadEnabled(false);
		this->removeFromParentAndCleanup(true);
	}

	void close() {
		if (m_closing) return;
		m_closing = true;
		m_buttonMenu->setEnabled(false);

		this->runAction(CCFadeTo::create(0.14f, 0));
		m_root->runAction(CCSequence::create(
			CCEaseIn::create(CCScaleTo::create(0.14f, 0.88f), 2.f),
			CCCallFunc::create(this, callfunc_selector(AssistPopup::finishClose)),
			nullptr
		));
	}

	// ---- callbacks -------------------------------------------------
	void onPill(CCObject* sender) {
		int i = static_cast<CCNode*>(sender)->getTag();
		if (i == 0) {
			assist::setEnabled(false);
		} else {
			assist::setStage(i - 1);
			assist::setEnabled(true);
		}
		assist::resetHolds();
		this->refresh(true);
	}

	void onCloseBtn(CCObject*) {
		this->close();
	}

	void keyBackClicked() override {
		this->close();
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
