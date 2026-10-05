#include "PutItemEvent.h"

#include "MpActor.h"

PutItemEvent::PutItemEvent(MpActor* actor_, MpObjectReference* sourceRefr_,
                           const Inventory::Entry& entry_)
  : actor(actor_)
  , sourceRefr(sourceRefr_)
  , entry(entry_)
{
}

const char* PutItemEvent::GetName() const
{
  return "onPutItem";
}

std::string PutItemEvent::GetArgumentsJsonArray() const
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

void PutItemEvent::OnFireSuccess(WorldState*)
{
  // Same desync tolerance as TakeItemEvent: move only what the player really
  // has, instead of throwing and dropping the transfer.
  std::vector<Inventory::Entry> removed;
  actor->RemoveItemsClamped({ entry }, &removed);

  for (const auto& e : removed) {
    sourceRefr->AddItems({ e });
  }
}
