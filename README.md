runetf - gamemode for Team Fortress 2
=================

RuneTF is a server mod for Team Fortress 2. Runes are power-ups that grant special abilities. You can have only one rune active at a time, and remain active until death.


Design Rationale
======
The original Quake Runes mod was a free-for-all player deathmatch. Many of the runes had abilities increasing the player's survivability or fragging efficiency. When porting the concept to Team Fortress, it became apparent that some of the runes were too strong in a team-based objective setting. Similar to the design philosophy of the weapon alternatives, runes that increase a player's damage output come with negative effects.

When designing new runes, the focus was to complement team-play more than buffing individual players. Some runes can become more effective when used in coordination with the team. Because of the random nature of the rune drops, runes that only effect certain classes were avoided. However some runes are certainly more effective in the hands of certain classes. As such, runes can be shared with teammates by dropping them on the ground.

Although most runes have passive abilities, some runes require activating with the command +USE.


List of Runes
=======

Aware - Periodically scans your nearby surroundings. Shows class and health of nearby players.

Recall - Teleport to a previous location. +USE sets or clears teleport destination. When far enough away, teleports player to destination.

Blink - Teleports between two locations. +USE to toggle destinations.

IronDome - Launches interceptor rockets upon death. Interceptor rockets cause knockback only.

DeadMansTrigger - Triggers explosion upon death. +USE to activate with greater effect. When fully detonated the affected players are temporarily stunned. Contact with interceptor rockets negate this rune's effects.

Sharing - Healthkits and ammokits picked up are shared with nearby teammates.

Rage - Increases rate of fire upon damaging an enemy. Prolonged usage can cause adverse effects.

Powerplay - Instantly respawns fallen teammates when you kill three enemies in under 10 seconds.

Shared Sacrifice - Incoming damage is divided among nearby teammates.

Diamond Skin - Damage reduced by 10. The bleed, jarate, milk, and stun conditions have no effect.

Engineering - Grants speed boost when carrying objects. Steals enemy sentries upon death.

Ammo - Passively replenishes ammo. Active with +USE to restore your weapon clip. Additionally, engineers gain metal, and spies gain cloak time.

Pounce - Passively grants more control over movement when in the air.

Vault - Passively increases height of jumps.

Assault - Grants a speed boost when ubered.

Vanguard - Redirects damage from nearby weaker teammates to you.

AirBud - Increased air movement. Decreased gravity.

Sabotage - Deal extra damage to buildings and take extra damage from engineers. Stronger spy sappers.

Melee - Melee attacks only. Damage increased.

Haste - Slight buff to player movement speed. Penality when under the effects of jarate, bleed, milk, or daze.

Repulsion - Enemy rockets, flares, and arrows are pushed away from the player.



Installation
=====
Requires sdkhooks

Copy materials, models, sound, plugins, extensions, gamedata, data into the respective sourcemod folder.

Add exec happs.cfg to your server.cfg - namely `tf_allow_player_use 1` is required.


Currently, only stock maps have rune spawn points.  But an in-game menu is provided to add spawn points to custom maps.


cvars
===

rune_enable
rune_spawn_interval
rune_spawn_lifetime
rune_spawn_droptime
rune_round_start_clear
rune_round_end_clear

rune_vote_threshold
rune_vote_allow_enable
rune_vote_allow_disable


You can change these cvars on a per map basis by adding them to a cfg/mapname.cfg file.


User Commands
===
+use _players must have bound a key to +use_

``!drop`` _drop your currently held rune_

``!inspect_rune`` _shows rune held by player or targeting_

``!runes`` _vote to enable runetf_

``!norunes`` _vote to disable runetf_

``!info_rune`` _display a rune's abilities_


Dev Commands
===

``!spawn_rune`` _spawn a specific rune_

``/toggle_spawn_rune`` _enable !spawn__rune for everyone_

``!menu_rune`` _add rune spawn points to the map using the in-game menu_

``sm_gen``  _modify rune spawn points in the map_

@args create, display, set, merge, load, drop, cluster, event

Create a new cluster with `sm_gen` cluster create <clustername>

Add a range of cluster ids with `sm_gen` cluster add|remove <clustername> [lower-upper]

``sm_cluster`` _create clusters of rune spawn points_

``sm_it`` _low-level spawn point manipulations_

``sm_gen_save`` _saves current spawn points and events to disk_

``spawn_test`` _reloads from disk spawn points of map_

Spawn points have unique numbers assiociated with them.  You can group spawn points with the `sm_gen cluster create` command.  Clusters can be attached to events that will either add/remove or toggle those spawn points to the list of spawn points.  Additionally clusters of runes can be instantly spawned when events are triggered.

When adding new spawn points, make sure to merge the working-set into the master list and save.  Test your changes with `test_spawn`.

