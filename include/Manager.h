#pragma once

class Manager : public REX::Singleton<Manager>
{
public:
	void loadINI();
	void update(const RE::Actor* player);

private:
	void end(const RE::Actor* player);
	bool changeCollisionFlag(const RE::TESObjectREFRPtr& ref, bool set);
	bool isValidActor(const RE::TESObjectREFRPtr& actor) const;
	RE::TESObjectREFRPtr getBumpedActor(const RE::bhkCharacterController* a_charCtrl);

	RE::RefHandle m_currentTarget{ 0 };
	float m_bumpCounter{ 0.0f };
	std::vector<RE::RefHandle> m_resetHandles;
	float m_resetCounter{ 0.0f };
	float m_bumpTimeLimit{ 2.0f };
};