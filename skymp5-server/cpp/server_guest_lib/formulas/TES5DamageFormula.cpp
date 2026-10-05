#include "TES5DamageFormula.h"

#include "HitData.h"
#include "MpActor.h"
#include "SpellCastData.h"
#include "WorldState.h"
#include "libespm/espm.h"

namespace internal {

bool IsUnarmedAttack(const uint32_t sourceFormId)
{
  return sourceFormId == 0x1f4;
}

class TES5DamageFormulaImpl
{
  using Effects = std::vector<espm::Effects::Effect>;

public:
  TES5DamageFormulaImpl(const MpActor& aggressor_, const MpActor& target_,
                        const HitData& hitData_, float armorRatingMultiplier_,
                        float hiddenPieceBonus_);

  [[nodiscard]] float CalculateDamage() const;

private:
  const MpActor& aggressor;
  const MpActor& target;
  const HitData& hitData;
  WorldState* espmProvider;
  float armorRatingMultiplier;
  float hiddenPieceBonus;

private:
  [[nodiscard]] float GetBaseWeaponDamage() const;
  [[nodiscard]] float CalcWeaponRating() const;
  [[nodiscard]] float CalcArmorRatingComponent(
    const Inventory::Entry& opponentEquipmentEntry) const;
  [[nodiscard]] float CalcOpponentArmorRating() const;
  [[nodiscard]] float CalcWornArmorPieceCount() const;
  [[nodiscard]] float CalcMagicEffects(const Effects& effects) const;
  [[nodiscard]] float DetermineDamageFromSource(uint32_t source) const;
  [[nodiscard]] float CalcUnarmedDamage() const;
  [[nodiscard]] float CalcArmorDamagePenalty() const;
};

TES5DamageFormulaImpl::TES5DamageFormulaImpl(const MpActor& aggressor_,
                                             const MpActor& target_,
                                             const HitData& hitData_,
                                             float armorRatingMultiplier_,
                                             float hiddenPieceBonus_)
  : aggressor(aggressor_)
  , target(target_)
  , hitData(hitData_)
  , espmProvider(aggressor.GetParent())
  , armorRatingMultiplier(armorRatingMultiplier_)
  , hiddenPieceBonus(hiddenPieceBonus_)
{
}

float TES5DamageFormulaImpl::GetBaseWeaponDamage() const
{
  const auto weapData =
    espm::GetData<espm::WEAP>(hitData.source, espmProvider);
  if (!weapData.weapData) {
    throw std::runtime_error(
      fmt::format("no weapData for {:#x}", hitData.source));
  }
  return weapData.weapData->damage;
}

float TES5DamageFormulaImpl::CalcWeaponRating() const
{
  // TODO(#457): take other components into account
  return GetBaseWeaponDamage();
}

float TES5DamageFormulaImpl::CalcMagicEffects(const Effects& effects) const
{
  float armorRating = 0.f;
  for (const auto& effect : effects) {
    const auto actorValueType =
      espm::GetData<espm::MGEF>(effect.effectId, espmProvider).data.primaryAV;
    if (actorValueType == espm::ActorValue::DamageResist) {
      armorRating += effect.magnitude;
    }
  }
  return armorRating;
}

float TES5DamageFormulaImpl::CalcArmorRatingComponent(
  const Inventory::Entry& opponentEquipmentEntry) const
{
  if (opponentEquipmentEntry.GetWorn() != Inventory::Worn::None &&
      espm::GetRecordType(opponentEquipmentEntry.baseId, espmProvider) ==
        espm::ARMO::kType) {
    const auto armorData =
      espm::GetData<espm::ARMO>(opponentEquipmentEntry.baseId, espmProvider);
    // TODO(#458): take other components into account
    auto ac = static_cast<float>(armorData.baseRatingX100) / 100;
    if (armorData.enchantmentFormId) {
      // TODO(#632) refactor this effect with actor effect system
      const auto enchantmentData =
        espm::GetData<espm::ENCH>(armorData.enchantmentFormId, espmProvider);
      ac += CalcMagicEffects(enchantmentData.effects);
    }

    return ac;
  }
  return 0;
}

float TES5DamageFormulaImpl::CalcOpponentArmorRating() const
{
  float combinedArmorRating = 0;
  auto eq = target.GetEquipment();
  for (auto& entry : eq.inv.entries) {
    combinedArmorRating += CalcArmorRatingComponent(entry);
  }
  return combinedArmorRating;
}

float TES5DamageFormulaImpl::CalcWornArmorPieceCount() const
{
  // Same set of items CalcOpponentArmorRating sums up: only worn apparel
  // counts. Worn weapons (a shield-less dagger in hand, arrows) must not:
  // vanilla grants the hidden bonus per worn armor slot only.
  float pieceCount = 0;
  auto eq = target.GetEquipment();
  for (auto& entry : eq.inv.entries) {
    if (entry.GetWorn() != Inventory::Worn::None &&
        espm::GetRecordType(entry.baseId, espmProvider) ==
          espm::ARMO::kType) {
      pieceCount += 1.f;
    }
  }
  return pieceCount;
}

float TES5DamageFormulaImpl::CalcUnarmedDamage() const
{
  const uint32_t raceId = aggressor.GetRaceId();
  return espm::GetData<espm::RACE>(raceId, espmProvider).unarmedDamage;
}

float TES5DamageFormulaImpl::DetermineDamageFromSource(uint32_t source) const
{
  return IsUnarmedAttack(source) ? CalcUnarmedDamage() : CalcWeaponRating();
}

float TES5DamageFormulaImpl::CalcArmorDamagePenalty() const
{
  // TODO(#457): weapon rating is probably not only component of incomingDamage
  // Replace this with another issue reference upon investigation
  const float maxArmorRating =
    espm::GetData<espm::GMST>(espm::GMST::kFMaxArmorRating, espmProvider)
      .value;
  const float armorScalingFactor =
    espm::GetData<espm::GMST>(espm::GMST::kFArmorScalingFactor, espmProvider)
      .value;

  // TODO(#458): armor skill, perks and per-piece bonuses are not simulated.
  // Summing the raw base ratings alone would give ~17% reduction for a full
  // daedric set, while vanilla reaches fMaxArmorRating at skill 100, so the
  // sum is scaled to approximate a maxed-out character.
  const float effectiveArmorRating =
    CalcOpponentArmorRating() * armorRatingMultiplier;

  // Vanilla's hidden armor: +25 rating per worn piece (+3% reduction by
  // default), applied after the scaled rating and capped together with it.
  float damageReduction =
    effectiveArmorRating * armorScalingFactor +
    hiddenPieceBonus * CalcWornArmorPieceCount();
  damageReduction = std::min<float>(damageReduction, maxArmorRating);
  damageReduction = std::max<float>(damageReduction, 0.f);

  return 0.01f * (100.f - damageReduction);
}

float TES5DamageFormulaImpl::CalculateDamage() const
{
  const float incomingDamage = DetermineDamageFromSource(hitData.source);

  // TODO(#461): add difficulty multiplier
  // TODO(#463): add sneak modifier
  float damage = incomingDamage * CalcArmorDamagePenalty();

  if (hitData.isPowerAttack) {
    damage *= 2.f;
  }

  if (hitData.isHitBlocked) {
    // TODO(#460): implement correct block formula
    damage *= 0.1f;
  }

  if (hitData.isSneakAttack) {
    // TODO(GM-613): get from GameSettings
    damage *= 1.3f;
  }

  return damage;
}

class TES5SpellDamageFormulaImpl
{
  using Effects = std::vector<espm::Effects::Effect>;

public:
  TES5SpellDamageFormulaImpl(const MpActor& aggressor_, const MpActor& target_,
                             const SpellCastData& spellCastData_);

  [[nodiscard]] float CalculateDamage() const;

private:
  const MpActor& aggressor;
  const MpActor& target;
  const SpellCastData& spellCastData;
  WorldState* espmProvider;

private:
  [[nodiscard]] float GetBaseSpellDamage() const;
};

TES5SpellDamageFormulaImpl::TES5SpellDamageFormulaImpl(
  const MpActor& aggressor_, const MpActor& target_,
  const SpellCastData& spellCastData_)
  : aggressor(aggressor_)
  , target(target_)
  , spellCastData(spellCastData_)
  , espmProvider(aggressor.GetParent())
{
}

float TES5SpellDamageFormulaImpl::GetBaseSpellDamage() const
{
  const auto spellData =
    espm::GetData<espm::SPEL>(spellCastData.spell, espmProvider);

  float damage = 0.f;

  for (const auto& effect : spellData.effects) {

    if (!effect.effectItem || effect.effectFormId == 0) {
      continue;
    }

    auto magicEffect =
      espm::GetData<espm::MGEF>(effect.effectFormId, espmProvider);

    const bool needAddDamage =
      magicEffect.data.IsFlagSet(espm::MGEF::Flags::Hostile) ||
      magicEffect.data.IsFlagSet(espm::MGEF::Flags::Detrimental);

    if (needAddDamage &&
        magicEffect.data.primaryAV == espm::ActorValue::Health) {

      damage += effect.effectItem->magnitude;
    }
  }
  return damage;
}

float TES5SpellDamageFormulaImpl::CalculateDamage() const
{
  return GetBaseSpellDamage();
}

}

TES5DamageFormula::TES5DamageFormula(float armorRatingMultiplier_,
                                     float hiddenPieceBonus_)
  : armorRatingMultiplier(armorRatingMultiplier_)
  , hiddenPieceBonus(hiddenPieceBonus_)
{
}

float TES5DamageFormula::CalculateDamage(const MpActor& aggressor,
                                         const MpActor& target,
                                         const HitData& hitData) const
{
  return internal::TES5DamageFormulaImpl(aggressor, target, hitData,
                                         armorRatingMultiplier,
                                         hiddenPieceBonus)
    .CalculateDamage();
}

float TES5DamageFormula::CalculateDamage(
  const MpActor& aggressor, const MpActor& target,
  const SpellCastData& spellCastData) const
{
  return internal::TES5SpellDamageFormulaImpl(aggressor, target, spellCastData)
    .CalculateDamage();
}
