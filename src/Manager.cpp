#include "Manager.h"

void Manager::loadINI()
{
	const auto path = std::format("Data/SKSE/Plugins/{}.ini", Plugin::NAME);

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(path.c_str());

	constexpr const char* sectionGeneral = "General";
	m_bumpTimeLimit = static_cast<float>(ini.GetDoubleValue(sectionGeneral, "fBumpTimeLimit"));
}

void Manager::update(const RE::Actor* player)
{
	constexpr float RESET_LIMIT = 2.0f;

	const auto timer = RE::BSTimer::GetSingleton();
	const auto realTime = timer ? timer->realTimeDelta : 0.0f;
	if (realTime <= 0.0f)
		return;

	m_resetCounter += realTime;
	if (m_resetCounter >= RESET_LIMIT)
	{
		end(player);
		m_resetCounter = 0.0f;
	}

	const auto actorState = player->AsActorState();
	const auto vats = RE::VATS::GetSingleton();
	if (!player->Is3DLoaded() || player->IsInCombat() || !actorState || actorState->IsFlying() || !vats || vats->mode == RE::VATS::VATS_MODE::kKillCam)
		return;

	const auto controller = player->GetCharController();
	if (!controller)
		return;

	const auto target = getBumpedActor(controller);
	if (!target)
		return;

	const auto handle = target->CreateRefHandle().native_handle();
	if (m_currentTarget != handle)
	{
		m_currentTarget = handle;
		m_bumpCounter = 0.0f;
	}

	m_bumpCounter += realTime;
	if (m_bumpCounter >= m_bumpTimeLimit)
	{
		const auto changed = changeCollisionFlag(target, true);
		if (changed && m_currentTarget != 0)
		{
			m_resetHandles.push_back(m_currentTarget);
		}

		m_currentTarget = 0;
		m_bumpCounter = 0.0f;
	}
}

void Manager::end(const RE::Actor* player)
{
	constexpr float DISTANCE_MIN = 50.0f;

	for (auto it = m_resetHandles.begin(); it != m_resetHandles.end();)
	{
		const auto entry = *it;
		RE::TESObjectREFRPtr actor;
		if (!RE::LookupReferenceByHandle(entry, actor))
		{
			// non persistent npcs... Should not exist normally
			SKSE::log::warn("Unable to reset: {:08X}", entry);
			it = m_resetHandles.erase(it);
			continue;
		}

		if (player->GetDistance(actor.get()) >= DISTANCE_MIN)
		{
			changeCollisionFlag(actor, false);
			it = m_resetHandles.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool Manager::changeCollisionFlag(const RE::TESObjectREFRPtr& ref, bool set)
{
	constexpr auto FILTER_FLAG = 0x20;

	const auto actor = ref->As<RE::Actor>();
	if (!actor)
		return false;

	const auto controller = actor->GetCharController();
	if (!controller)
		return false;

	const auto rigidBody = controller ? controller->GetRigidBody() : nullptr;
	if (!rigidBody)
		return false;

	auto& filter = rigidBody->collidable.broadPhaseHandle.collisionFilterInfo.filter;
	if (static_cast<bool>(filter & FILTER_FLAG) == set)
		return false;

	if (set)
	{
		filter |= FILTER_FLAG;
	}
	else
	{
		filter &= ~FILTER_FLAG;
	}

	return true;
}

bool Manager::isValidActor(const RE::TESObjectREFRPtr& actor) const
{
	return actor && actor->IsActor() && actor->IsInitialized() && actor->Is3DLoaded();
}

RE::TESObjectREFRPtr Manager::getBumpedActor(const RE::bhkCharacterController* charCtrl)
{
	auto rigidBody = charCtrl ? charCtrl->bumpedCharCollisionObject.get() : nullptr;
	if (!rigidBody)
		return nullptr;

	const auto charCollisionFilterInfo = rigidBody->collidable.GetCollisionLayer();
	if (charCollisionFilterInfo != RE::COL_LAYER::kCharController)
		return nullptr;

	RE::TESObjectREFRPtr objRef(rigidBody ? rigidBody->GetUserData() : nullptr);
	if (!isValidActor(objRef))
		return nullptr;

	return objRef;
}