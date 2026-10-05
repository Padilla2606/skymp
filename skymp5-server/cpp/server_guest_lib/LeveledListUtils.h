#pragma once
#include "libespm/Combiner.h"
#include "libespm/espm.h"
#include <cstdint>
#include <map>
#include <set>
#include <vector>

class LeveledListUtils
{
public:
  struct Entry
  {
    uint32_t formId = 0;
    uint32_t count = 0;
  };

  // It seems that pcLevel=0 makes it thinking that pcLevel=maximum possible pc
  // level
  static std::vector<Entry> EvaluateList(
    const espm::CombineBrowser& br, const espm::LookupResult& lookupRes,
    uint32_t pcLevel = 0, uint8_t* chanceNoneOverride = nullptr);

  static std::map<uint32_t, uint32_t> EvaluateListRecurse(
    const espm::CombineBrowser& br, const espm::LookupResult& lookupRes,
    uint32_t countMult = 1, uint32_t pcLevel = 0,
    uint8_t* chanceNoneOverride = nullptr);

  static std::vector<uint32_t> EvaluateTemplateChain(
    const espm::CombineBrowser& br, const espm::LookupResult& headNpc,
    uint32_t pcLevel);

  // Enumerates every terminal NPC reachable from headNpc through its template
  // chain (TPLT) and any leveled creature lists (LVLN/LVLI) in between,
  // without evaluating (i.e. without random/level-based selection). A terminal
  // is an NPC without a template, or one whose template target cannot be
  // resolved (then it counts as terminal itself). Duplicates and cycles are
  // removed. Returns an empty vector when nothing can be resolved.
  static std::vector<uint32_t> CollectAllTerminalNpcs(
    const espm::CombineBrowser& br, const espm::LookupResult& headNpc);

private:
  static void CollectAllTerminalNpcsRecursive(
    const espm::CombineBrowser& br, const espm::LookupResult& lookupRes,
    std::set<uint32_t>& visited, std::set<uint32_t>& terminals);

  static const espm::NPC_* ConvertToNpc(
    const espm::LookupResult& lookupResult);

  static uint32_t GetBaseTemplateId(const espm::NPC_* cursor, uint32_t fileIdx,
                                    const espm::CombineBrowser& browser);

  static void UpdateCursorAndResult(uint32_t templateId,
                                    espm::LookupResult& templateResult,
                                    const espm::NPC_*& cursor,
                                    uint32_t& cursorFileIdx,
                                    std::vector<uint32_t>& result,
                                    const espm::CombineBrowser& browser,
                                    uint32_t pcLevel);

  static uint32_t EvaluateAndSelectNpcId(
    const espm::CombineBrowser& browser,
    const espm::LookupResult& templateResult, uint32_t pcLevel);
};
