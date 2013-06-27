#include "rune.hpp"

void Rune::Pickup(int client)
{
	++ref;
	if(f_pPickup == NULL)
		return;

	f_pPickup->PushCell(client);
	f_pPickup->PushCell(Id);
	f_pPickup->PushCell(ref);
  f_pPickup->Execute(NULL);
}


void Rune::Drop(int client)
{
	--ref;
	if(f_pDrop == NULL)
		return;

	f_pDrop->PushCell(client);
	f_pDrop->PushCell(Id);
	f_pDrop->PushCell(ref);
	f_pDrop->Execute(NULL);
}
