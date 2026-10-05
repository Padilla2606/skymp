#include "TestUtils.hpp"
#include <catch2/catch_all.hpp>
#include <chrono>

#include "GetBaseActorValues.h"
#include "HitData.h"
#include "PacketParser.h"
#include "formulas/TES5DamageFormula.h"
#include "libespm/Loader.h"

namespace {
const auto kExtraWornTrue = [] {
  Inventory::ExtraData extra;
  extra.worn_ = true;
  return extra;
}();
const auto kExtraWornFalse = [] {
  Inventory::ExtraData extra;
  extra.worn_ = false;
  return extra;
}();
}

PartOne& GetPartOne();
extern espm::Loader l;
using namespace std::chrono_literals;

TEST_CASE("Formula takes weapon damage into account", "[TES5DamageFormula]")
{
  PartOne& p = GetPartOne();
  DoConnect(p, 0);
  p.CreateActor(0xff000000, { 0, 0, 0 }, 0, 0x3c);
  p.SetUserActor(0, 0xff000000);
  auto& ac = p.worldState.GetFormAt<MpActor>(0xff000000);

  ac.SetEquipment(Equipment());

  RawMessageData rawMsgData;
  rawMsgData.userId = 0;
  HitData hitData;
  hitData.target = 0x14;
  hitData.aggressor = 0x14;
  hitData.source = 0x0001397E; // iron dagger 4 damage

  TES5DamageFormula formula{};
  REQUIRE(formula.CalculateDamage(ac, ac, hitData) == 4.0f);

  p.DestroyActor(0xff000000);
  DoDisconnect(p, 0);
}

TEST_CASE("Damage is reduced based on target's armor", "[TES5DamageFormula]")
{
  PartOne& p = GetPartOne();
  DoConnect(p, 0);
  p.CreateActor(0xff000000, { 0, 0, 0 }, 0, 0x3c);
  p.SetUserActor(0, 0xff000000);
  auto& ac = p.worldState.GetFormAt<MpActor>(0xff000000);

  RawMessageData rawMsgData;
  rawMsgData.userId = 0;
  HitData hitData;
  hitData.target = 0x14;
  hitData.aggressor = 0x14;
  hitData.source = 0x0001397E; // iron dagger 4 damage

  // 77382 = 0x12e46: Iron Gauntlets, rating = 10
  // 77387 = 0x12e4b: Iron Boots, rating = 10
  // 77389 = 0x12e4d: Iron Helmet, rating = 15
  // Total rating for worn armor: 10 + 10 = 20

  Equipment eq;
  eq.inv.entries.push_back(Inventory::Entry(77382, 1, kExtraWornTrue));
  eq.inv.entries.push_back(Inventory::Entry(77387, 1, kExtraWornTrue));
  eq.inv.entries.push_back(Inventory::Entry(77389, 1, kExtraWornFalse));
  ac.SetEquipment(eq);

  TES5DamageFormula formula{};
  // 4 * 0.01 * (100 - (20 * 4.5 * .12 + 3 * 2)) = 3,328
  // (20 * 4.5 = armor skill/perk approximation, see kDefaultArmorRatingMultiplier;
  //  3 * 2 = hidden armor bonus, see kDefaultHiddenPieceBonus)
  REQUIRE(formula.CalculateDamage(ac, ac, hitData) ==
          4.0f * (0.01f * (100.0f - (20.0f * 4.5f * 0.12f + 3.0f * 2.0f))));

  auto repeatativeEntry = Inventory::Entry(77382, 1, kExtraWornTrue);
  Equipment eq2;

  for (int i = 0; i < 70; i++) {
    eq2.inv.entries.push_back(repeatativeEntry);
  }

  // Total rating for worn armor: 10 * 70 = 700
  ac.SetEquipment(eq2);

  // Armor rating is 700 * 4.5 * 0.12% = 378%, plus 3% * 70 pieces = 210%
  // But fMaxArmorRating = 80%
  // 4 * 0.01 * (100 - 80) = 4 * 0.2 = 0.8
  REQUIRE(formula.CalculateDamage(ac, ac, hitData) == 0.7999999523f);

  p.DestroyActor(0xff000000);
  DoDisconnect(p, 0);
}

TEST_CASE("Formula is race-dependent for unarmed attack",
          "[TES5DamageFormula]")
{
  PartOne& p = GetPartOne();
  DoConnect(p, 0);
  p.CreateActor(0xff000000, { 0, 0, 0 }, 0, 0x3c);
  p.SetUserActor(0, 0xff000000);
  // Nord bu default
  auto& ac = p.worldState.GetFormAt<MpActor>(0xff000000);
  ac.SetEquipment(Equipment());

  RawMessageData rawMsgData;
  rawMsgData.userId = 0;
  HitData hitData;
  hitData.target = 0x14;
  hitData.aggressor = 0x14;
  hitData.source = 0x1f4; // unarmed attack

  {
    TES5DamageFormula formula{};
    REQUIRE(formula.CalculateDamage(ac, ac, hitData) == 4.0f);
  }

  Appearance appearance;
  appearance.raceId = 0x13745; // KhajiitRace
  ac.SetAppearance(&appearance);
  ac.SetPercentages({ 1, 1, 1 });

  {
    TES5DamageFormula formula{};
    REQUIRE(formula.CalculateDamage(ac, ac, hitData) == 10.0f);
  }

  p.DestroyActor(0xff000000);
  DoDisconnect(p, 0);
}
