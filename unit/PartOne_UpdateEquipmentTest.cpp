#include "TestUtils.hpp"

extern espm::Loader& GetEspmLoader();

TEST_CASE("UpdateEquipment", "[PartOne]")
{

  PartOne partOne;

  DoConnect(partOne, 0);
  partOne.CreateActor(0xff000ABC, { 1.f, 2.f, 3.f }, 180.f, 0x3c);
  partOne.SetUserActor(0, 0xff000ABC);
  DoMessage(partOne, 0, jEquipment);
  partOne.Messages().clear();

  DoConnect(partOne, 1);
  partOne.CreateActor(0xffABCABC, { 11.f, 22.f, 33.f }, 180.f, 0x3c);
  partOne.SetUserActor(1, 0xffABCABC);

  // createActor should contain equipment
  auto res = FindRefrMessageIdx<CreateActorMessage>(partOne, 0);
  REQUIRE(res.filteredMessages.size() == 1);
  REQUIRE(res.filteredMessages[0].equipment.has_value());
  REQUIRE(res.filteredMessages[0].equipment->ToJson().dump() ==
          jEquipment["data"].dump());

  partOne.Messages().clear();

  DoMessage(partOne, 0, jEquipment);

  REQUIRE(partOne.Messages().size() == 2);
  REQUIRE(std::find_if(partOne.Messages().begin(), partOne.Messages().end(),
                       [&](auto m) {
                         return m.j["t"] == MsgType::UpdateEquipment &&
                           m.j["idx"] == 0 && m.reliable && m.userId == 1 &&
                           m.j["data"] == jEquipment["data"];
                       }) != partOne.Messages().end());
  REQUIRE(std::find_if(partOne.Messages().begin(), partOne.Messages().end(),
                       [&](auto m) {
                         return m.j["t"] == MsgType::UpdateEquipment &&
                           m.j["idx"] == 0 && m.reliable && m.userId == 0 &&
                           m.j["data"] == jEquipment["data"];
                       }) != partOne.Messages().end());
}

TEST_CASE("UpdateEquipment keeps owned items when an unknown one is worn",
          "[PartOne][espm]")
{
  // Regression: a single item the server does not own (inventory desync)
  // used to reject the whole equipment update, so the server unequipped the
  // player's weapon and melee damage silently stopped working.
  PartOne partOne;

  partOne.AttachEspm(&GetEspmLoader());

  DoConnect(partOne, 0);
  partOne.CreateActor(0xff000ABC, { 1.f, 2.f, 3.f }, 180.f, 0x3c);
  partOne.SetUserActor(0, 0xff000ABC);

  auto& actor = partOne.worldState.GetFormAt<MpActor>(0xff000ABC);
  actor.RemoveAllItems();
  actor.AddItem(0x12eb7, 1); // Iron Sword: the server owns it

  partOne.Messages().clear();

  nlohmann::json msg = jEquipment;
  msg["data"]["inv"]["entries"] = nlohmann::json::array(
    { { { "baseId", 0x12eb7 }, { "count", 1 }, { "worn", true } },
      { { "baseId", 0x3eadd }, { "count", 1 }, { "worn", true } } });
  DoMessage(partOne, 0, msg);

  const auto& equipment = actor.GetEquipment().inv;
  bool hasSword = false, hasUnknown = false, swordIsWorn = false;
  for (const auto& entry : equipment.entries) {
    if (entry.baseId == 0x12eb7) {
      hasSword = true;
      swordIsWorn = entry.GetWorn() != Inventory::Worn::None;
    }
    if (entry.baseId == 0x3eadd) {
      hasUnknown = true;
    }
  }

  REQUIRE(hasSword);
  REQUIRE_FALSE(hasUnknown);

  // Without the worn flag the damage formula sees an unarmed target, so
  // armor never reduces incoming damage (CalcArmorRatingComponent /
  // CalcWornArmorPieceCount only look at worn entries).
  REQUIRE(swordIsWorn);
}
