#pragma once

#include "IDamageFormula.h"

// Implements vanilla Skyrim damage formula.
// Some parts may be missing. If they are, there should be a TODO regarding it.
// If there's no corresponding TODO, consider adding it and/or filing an issue.

class TES5DamageFormula : public IDamageFormula
{
public:
  // SkyMP does not simulate armor skill or perks (see CalcArmorRatingComponent
  // and CalcArmorDamagePenalty in TES5DamageFormula.cpp), so summing raw base
  // ratings only gives ~17% reduction for the best set, while vanilla clients
  // reach the fMaxArmorRating (80%) cap with a full set at skill 100.
  // This multiplier approximates a maxed-out character. Override it with
  // armorFormulaSettings.ratingMultiplier in server-settings.json.
  static constexpr float kDefaultArmorRatingMultiplier = 4.5f;

  // Vanilla always adds a hidden armor rating of 25 per worn piece (see
  // fArmorBaseFactor), i.e. +3% damage reduction per piece at the default
  // fArmorScalingFactor of 0.12. It is not part of any item's rating, so it
  // must be applied on top of the summed ratings. SkyMP has no displayed
  // rating, so this bonus is expressed directly as percent per worn piece.
  // Override it with armorFormulaSettings.hiddenPieceBonus in
  // server-settings.json, set it to 0 to disable.
  static constexpr float kDefaultHiddenPieceBonus = 3.0f;

  explicit TES5DamageFormula(
    float armorRatingMultiplier_ = kDefaultArmorRatingMultiplier,
    float hiddenPieceBonus_ = kDefaultHiddenPieceBonus);

  [[nodiscard]] float CalculateDamage(const MpActor& aggressor,
                                      const MpActor& target,
                                      const HitData& hitData) const override;

  [[nodiscard]] float CalculateDamage(
    const MpActor& aggressor, const MpActor& target,
    const SpellCastData& spellCastData) const override;

private:
  float armorRatingMultiplier;
  float hiddenPieceBonus;
};
