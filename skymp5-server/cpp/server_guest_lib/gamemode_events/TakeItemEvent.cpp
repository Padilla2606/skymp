#include "TakeItemEvent.h"

#include "MpActor.h"
#include "MpObjectReference.h"

TakeItemEvent::TakeItemEvent(MpActor* actor_, MpObjectReference* sourceRefr_,
                             const Inventory::Entry& entry_)
  : actor(actor_)
  , sourceRefr(sourceRefr_)
  , entry(entry_)
{
}

const char* TakeItemEvent::GetName() const
{
  return "onTakeItem";
}

std::string TakeItemEvent::GetArgumentsJsonArray() const
{
  std::string result;
  result += "[";
  result += std::to_string(sourceRefr->GetFormId());
  result += ",";
  result += std::to_string(actor->GetFormId());
  result += ",";
  result += std::to_string(entry.baseId);
  result += ",";
  result += std::to_string(entry.count); // TODO: implement extra data
  result += "]";
  return result;
}

void TakeItemEvent::OnFireSuccess(WorldState*)
{
  // The client's inventory view can drift from ours (periodic applyInventory,
  // loot menus, respawns), so a take can legitimately ask for more than we
  // have. Take what is really there instead of throwing and losing the whole
  // transfer.
  std::vector<Inventory::Entry> removed;
  sourceRefr->RemoveItemsClamped({ entry }, &removed);

  for (const auto& e : removed) {
    actor->AddItems({ e });
  }
}
