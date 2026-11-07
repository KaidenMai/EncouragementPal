#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <array>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
//  Dialogue. Cold, clinical, technically supportive.
// ---------------------------------------------------------------------------

namespace lines {
    static std::vector<std::string> const p25 = {
        "25%!",
        "Go!",
        "Nice!",
        "Woah..",
    };
    static std::vector<std::string> const p50 = {
        "50%!",
        "ts tuff",
        ":O",
        "WHAT.",
    };
    static std::vector<std::string> const p75 = {
        "75%!",
        "yo hes almost there!",
        "holy shit..",
        "ur so portuguese.",
    };
    static std::vector<std::string> const p90 = {
        "90%!",
        "i think i might cream myself..",
        "👀",
        "uh-",
    };
    static std::vector<std::string> const newBest = {
        ":O",
        "ouh shiii",
        "poo",
        "6-7",
        "ohh mah gawd",
    };
    static std::vector<std::string> const complete = {
        "*feels the aura*",
        "Level complete. I'll prepare the cake. Hypothetically. In theory. Do not ask about the cake, Its all a lie..",
        "*mogs*",
        "GG",
        "GET REKT SURV!",
        "Mpphh~ ah~ ngh~ *PLAP PLAP PLAP*",
        "THERE IS A STAIN THE SIZE OF TEXAS IN MY PANTS - itzvapestinct.",
        "AMBATUKAAAAAAAAAAAAAA",
        "IM CREAMING AHHH~"
    };

    std::string const& pick(std::vector<std::string> const& list) {
        static std::mt19937 rng{std::random_device{}()};
        static std::string const* last = nullptr;
        std::uniform_int_distribution<size_t> dist(0, list.size() - 1);
        auto const* choice = &list[dist(rng)];
        if (choice == last && list.size() > 1) {
            choice = &list[(choice - list.data() + 1) % list.size()];
        }
        last = choice;
        return *choice;
    }
}

// ---------------------------------------------------------------------------
//  The little pal + speech bubble that slides in from the corner.
// ---------------------------------------------------------------------------

class PalBubble : public CCNode {
public:
    static PalBubble* create(std::string const& text, float duration) {
        auto ret = new PalBubble();
        if (ret->init(text, duration)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

protected:
    bool init(std::string const& text, float duration) {
        if (!CCNode::init()) return false;
        this->setID("pal-bubble"_spr);

        constexpr float palSize = 64.f;
        constexpr float pad = 8.f;
        constexpr float textScale = 0.55f;

        // the pal
             CCSprite* pal = nullptr;
        auto custom = Mod::get()->getSettingValue<std::filesystem::path>("custom-picture");
        std::error_code ec;
        if (!custom.empty() && std::filesystem::exists(custom, ec)) {
            pal = CCSprite::create(custom.string().c_str());
        }
        if (!pal) pal = CCSprite::create("pal.png"_spr);
        if (!pal) pal = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        if (pal) {
            pal->setScale(palSize / std::max(pal->getContentWidth(), 1.f));
            pal->setPosition({palSize / 2.f, palSize / 2.f});
            this->addChild(pal, 2);

            // gentle idle bob + a little wiggle when talking
            pal->runAction(CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(CCMoveBy::create(0.6f, {0.f, 4.f})),
                CCEaseSineInOut::create(CCMoveBy::create(0.6f, {0.f, -4.f})),
                nullptr
            )));
            pal->runAction(CCSequence::create(
                CCRotateTo::create(0.08f, -8.f),
                CCRotateTo::create(0.08f, 8.f),
                CCRotateTo::create(0.08f, -4.f),
                CCRotateTo::create(0.08f, 0.f),
                nullptr
            ));
        }

        // the text
        auto label = CCLabelBMFont::create(
            text.c_str(), "chatFont.fnt", 300.f, kCCTextAlignmentLeft
        );
        label->setScale(textScale);
        label->setAnchorPoint({0.f, 0.5f});

        auto textW = label->getContentWidth() * textScale;
        auto textH = label->getContentHeight() * textScale;
        auto bubbleSize = CCSize{textW + pad * 2.f, std::max(textH + pad * 2.f, palSize * 0.8f)};

        // the bubble
        auto bubble = CCScale9Sprite::create("square02b_001.png", {0, 0, 80, 80});
        bubble->setColor({0, 0, 0});
        bubble->setOpacity(170);
        bubble->setContentSize(bubbleSize);
        bubble->setAnchorPoint({0.f, 0.5f});
        bubble->setPosition({palSize + 6.f, palSize / 2.f + 6.f});
        this->addChild(bubble, 1);

        label->setPosition({bubble->getPositionX() + pad, bubble->getPositionY()});
        this->addChild(label, 3);

        this->setContentSize({palSize + 6.f + bubbleSize.width, std::max(palSize, bubbleSize.height)});

        // slide in, wait, slide out, vanish
        CCPoint shown = {10.f, 40.f};
        CCPoint hidden = {-this->getContentWidth() - 20.f, 40.f};
        this->setPosition(hidden);
        this->runAction(CCSequence::create(
            CCEaseBackOut::create(CCMoveTo::create(0.35f, shown)),
            CCDelayTime::create(duration),
            CCEaseBackIn::create(CCMoveTo::create(0.3f, hidden)),
            CCRemoveSelf::create(),
            nullptr
        ));
        return true;
    }
};

// ---------------------------------------------------------------------------
//  Hooks
// ---------------------------------------------------------------------------

static constexpr std::array<int, 4> MILESTONES = {25, 50, 75, 90};

class $modify(PalPlayLayer, PlayLayer) {
    struct Fields {
        std::array<bool, MILESTONES.size()> hit{};
        bool spokeOnDeath = false;
    };

    bool palAllowed() {
        if (m_isPlatformer) return false; // percentages don't mean much there
        if (m_isPracticeMode && !Mod::get()->getSettingValue<bool>("practice-mode")) return false;
        return true;
    }

    void palSay(std::string const& text) {
        if (auto old = this->getChildByID("pal-bubble"_spr)) {
            old->removeFromParent();
        }
        auto duration = static_cast<float>(Mod::get()->getSettingValue<double>("duration"));
        if (auto bubble = PalBubble::create(text, duration)) {
            this->addChild(bubble, 1000);
        }
    }

    int palBest() {
        return m_level ? m_level->m_normalPercent.value() : 0;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        auto fields = m_fields.self();
        fields->spokeOnDeath = false;
        // starting from a checkpoint / start pos? don't cheer for milestones already behind you
        int percent = this->getCurrentPercentInt();
        for (size_t i = 0; i < MILESTONES.size(); i++) {
            fields->hit[i] = MILESTONES[i] <= percent;
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!Mod::get()->getSettingValue<bool>("milestone-cheers")) return;
        if (!this->palAllowed()) return;

        auto fields = m_fields.self();
        int percent = this->getCurrentPercentInt();
        bool pastBestOnly = Mod::get()->getSettingValue<bool>("milestones-past-best-only");

        for (size_t i = 0; i < MILESTONES.size(); i++) {
            if (fields->hit[i] || percent < MILESTONES[i]) continue;
            fields->hit[i] = true;

            if (pastBestOnly && MILESTONES[i] <= this->palBest()) continue;

            switch (MILESTONES[i]) {
                case 25: this->palSay(lines::pick(lines::p25)); break;
                case 50: this->palSay(lines::pick(lines::p50)); break;
                case 75: this->palSay(lines::pick(lines::p75)); break;
                case 90: this->palSay(lines::pick(lines::p90)); break;
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // GD "kills" you with a hidden spike at level start as an anti-cheat check
        if (object == m_anticheatSpike) {
            return PlayLayer::destroyPlayer(player, object);
        }

        int bestBefore = this->palBest();
        int percent = this->getCurrentPercentInt();

        PlayLayer::destroyPlayer(player, object);

        auto fields = m_fields.self();
        if (fields->spokeOnDeath) return;
        if (!Mod::get()->getSettingValue<bool>("new-best-cheers")) return;
        if (m_isPracticeMode || !this->palAllowed()) return;
        if (!m_player1 || !m_player1->m_isDead) return; // noclip etc.

        int minBest = static_cast<int>(Mod::get()->getSettingValue<int64_t>("min-new-best"));
        if (percent > bestBefore && percent >= minBest && percent < 100) {
            fields->spokeOnDeath = true;
            this->palSay(fmt::format("{}% -> {}%. {}", bestBefore, percent, lines::pick(lines::newBest)));
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        if (!Mod::get()->getSettingValue<bool>("complete-cheers")) return;
        if (!this->palAllowed()) return;
        this->palSay(lines::pick(lines::complete));
    }
};
