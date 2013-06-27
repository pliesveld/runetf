/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_ammo.inc
 * Description: Functions to return information about TF2 ammo.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _tf2_ammo_included
 #endinput
#endif
#define _tf2_ammo_included

enum TFAmmoTypes
{
    Primary=1,
    Secondary=2,
    Metal=3
}

stock TF2_GetAmmoAmount(client,TFAmmoTypes:type=Primary)
{
    return GetEntData(client, FindDataMapOffs(client, "m_iAmmo") + (_:type * 4), 4);
}

stock TF2_SetAmmoAmount(client, ammo,TFAmmoTypes:type=Primary)
{
    SetEntData(client, FindDataMapOffs(client, "m_iAmmo") + (_:type * 4), ammo, 4);
}

stock TF2_GetMetalAmount(client)
{
    return GetEntData(client, FindDataMapOffs(client, "m_iAmmo") + (_:Metal * 4), 4);
}

stock TF2_SetMetalAmount(client, metal)
{
    SetEntData(client, FindDataMapOffs(client, "m_iAmmo") + (_:Metal * 4), metal, 4);  
}
