#include "Hooks.h"
#include "Manager.h"

namespace Hooks
{
	/*struct HandleBumpedCharacter
	{
		static void thunk(RE::bhkCharacterController* pusher, RE::bhkCharacterController* target, const RE::hkContactPoint& contactPoint)
		{
			func(pusher, target, contactPoint);
		}
		static inline REL::Relocation<decltype(thunk)> func;

		static void Install()
		{
			REL::Relocation<std::uintptr_t> target1{ RELOCATION_ID(77244, 79130), REL::Relocate(0x3CD, 0x3B1) };
			stl::write_thunk_call<HandleBumpedCharacter>(target1.address());

			REL::Relocation<std::uintptr_t> target2{ RELOCATION_ID(77248, 79134), REL::Relocate(0x3F, 0x37) };
			stl::write_thunk_call<HandleBumpedCharacter>(target2.address());

			REL::Relocation<std::uintptr_t> target3{ RELOCATION_ID(77306, 79186), REL::Relocate(0x29F, 0x2A9) };
			stl::write_thunk_call<HandleBumpedCharacter>(target3.address());
		}
	};*/

	struct PlayerUpdate
	{
		static void thunk(RE::PlayerCharacter* player, float delta)
		{
			func(player, delta);
			if (!player)
				return;

			RE::ActorPtr controllingActor;
			player->GetControllingActor(controllingActor);

			Manager::GetSingleton()->update(controllingActor.get());
		};

		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx = 0xAD;
	};

	void Install()
	{
		//SKSE::AllocTrampoline(14);
		//HandleBumpedCharacter::Install();
		stl::write_vfunc<RE::PlayerCharacter, PlayerUpdate>();
	}
}