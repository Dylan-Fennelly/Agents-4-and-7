/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
enum class ReceiverCategories
{
	kNone = 0,
	kScene = 1 << 0,
	kPlayerAircraft = 1 << 1,
	kAlliedAircraft = 1 << 2,
	kEnemyAircraft = 1 << 3,
	kAlliedProjectile = 1 << 4,
	kEnemyProjectile = 1 << 5,
	kPickup = 1 << 6,
	kParticleSystem = 1 << 7,
	kSoundEffect = 1 << 8,
	kNetwork = 1 << 9,

	kAircraft = kPlayerAircraft | kAlliedAircraft | kEnemyAircraft,
	kPlayers = kPlayerAircraft | kAlliedAircraft, 
	kProjectile = kAlliedProjectile | kEnemyProjectile
};