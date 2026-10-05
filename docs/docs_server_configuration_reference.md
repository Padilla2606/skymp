# Server Configuration Reference

The recommended way to configure the server is setting up all required values in `server-settings.json`. It's standard JSON without C-style comments support. See also [Server Command Line API](docs_server_command_line_api.md).

## name

Server's name that will be published on a master server.

```json5
{
  // ...
  "name": "My Server"
  // ...
}
```

## masterKey

Specify the server key you wish to use for Master API. Client must have the same key specified to log in successfully.

```json5
{
  // ...
  "masterKey": "my-awesome-server",
  // ...
}
```

## listenHost

Specifies the IP address to bind to. Applies to the main UDP traffic (RakNet). Binds to `0.0.0.0` if unspecified.

```json5
{
  // ...
  "listenHost": "127.0.0.1",
  // ...
}
```

## uiListenHost

Specifies the IP address to bind to. Applies to the `uiPort` (http). Binds to `0.0.0.0` if unspecified.

```json5
{
  // ...
  "uiListenHost": "127.0.0.1",
  // ...
}
```

## metricsAuth

HTTP Basic authentication for the `/metrics` endpoint.

If omitted, `/metrics` is not available.

```json5
{
  // ...
  "metricsAuth": {
    "user": "prometheus",
    "password": "secret"
  },
  // ...
}
```

## port

This port would be used by player clients to connect to your server. At the current version of Skyrim Multiplayer servers use multiple ports and different protocols to manage different sorts of packets. See [Server Ports Usage](docs_server_ports_usage.md) page to learn more.

```json5
{
  // ...
  "port": 7777
  // ...
}
```

## maxPlayers

Sets player limit of the server. Visible in launcher and on skymp.io.

```json5
{
  // ...
  "maxPlayers": 108
  // ...
}
```

## dataDir

Contains relative or absolute path to a "data" directory which contains:
* vanilla Skyrim master files (Skyrim.esm, Update.esm, etc)
* plugin files (mods in .esp format)
* compiled Papyrus scripts in .pex format

This directory is exposed to `uiPort` and available via http.

At this moment, the server uses this directory for non-vanilla needs too:
* storing web-based GUI in `${dataDir}/ui`
* storing auto-generated manifest describing .esm/.esp files used and CRC32 of them

```json5
{
  // ...
  "dataDir": "data"
  // ...
}
```

## loadOrder

A list of relative or absolute paths to .esp/.esm files which would be loaded by the server during startup in the same order as Skyrim SE loads them.

Relative paths are searched in `${dataDir}` directory.

Absolute paths work but aren't accessible via `uiPort`. External tooling wouldn't be able to download them from the server.

```json5
{
  // ...
  "loadOrder": [
    "Skyrim.esm",
    "Update.esm",
    "Dawnguard.esm",
    "HearthFires.esm",
    "Dragonborn.esm"
  ]
  // ...
}
```

## archives

Specify BSA archives that will be loaded by the server.

At this moment, used only for compiled Papyrus scripts.

Relative/absolute paths work similar to esp/esm.

```json5
{
  // ...
  "archives": [
    "Skyrim - Misc.bsa"
  ]
  // ...
}
```

## lang

The language, the translation of which will be obtained from the string files located in Data/strings

```json5
{
  // ...
  "lang": "english"
  // ...
}
```

## offlineMode

The boolean variable shows is server in "offline mode" or not (the server allows clients to connect with any profile id they choose).
Users need to specify `"profileId"` in their `skymp5-settings.txt`.

```json5
{
  // ...
  "offlineMode": true
  // ...
}
```

## databaseDriver

Name of a database driver which would be used to store server data. `file` by default. There are also related options like `"databaseName"`. See [Database Drivers](docs_database_drivers.md) page to learn more.

```json5
{
  // ...
  "databaseDriver": "file"
  // ...
}
```

## reloot

A time before a game object restores its original state in milliseconds. Unlike Skyrim SE, Skyrim Multiplayer doesn't have a built-in Cell Reset mechanism. The server resets every object in the world every hour instead. With this option, you can change this time interval for every kind of game object. `"CONT"`, for example, means "Container" - chests, barrels, etc. See "record types" on [UESP](https://en.uesp.net/wiki/Skyrim_Mod:Mod_File_Format).

```json5
{
  // ...
  "reloot": {
    "FLOR": 86400000,
    "TREE": 86400000,
    "AMMO": 86400000,
    "ARMO": 86400000,
    "BOOK": 86400000,
    "INGR": 86400000,
    "ALCH": 86400000,
    "SCRL": 86400000,
    "CONT": 86400000,
    "SLGM": 86400000,
    "WEAP": 86400000,
    "MISC": 86400000
  }
  // ...
}
```

## forbiddenReloot
The option that allows you to forbid reloot for a specific item or a group of items based on its/their espm record type. Take a look at [UESP](https://en.uesp.net/wiki/Skyrim_Mod:Mod_File_Format).


```json5
{
  // ...
  // here your record types go
  "forbiddenReloot": ["FLOR", "TREE", "BOOK", ... ]
  // ...
}
```

## gamemodePath

Contains a relative or an absolute path to a file or directory with a gamemode.
Searches for `index.js` if a directory specified.

```json5
{
  // ...
  "gamemodePath": "gamemode.js"
  // ...
}
```

## startPoints

Contains a list of spawn points, one of which will be chosen at random.

```json5
{
  // ...
  "startPoints": [
    {
      "pos": [22659, -8697, -3594],
      "worldOrCell": "0x1a26f",
      "angleZ": 268
    }
  ]
  // ...
}
```

## isPapyrusHotReloadEnabled

A boolean setting that enables to turn on or turn off hot reload for compiled Papyrus scripts (.pex)

```json5
{
  // ...
  "isPapyrusHotReloadEnabled": false
  // ...
}
```

## locale

The name of a localizaiton file in `data/localization` that would be used by `M.GetText` Papyrus function (without extension).

```json5
{
  // ...
  "locale": "ru-RU"
  // ...
}
```

## enableConsoleCommandsForAll

Enable console commands for all, useful for testing.

```json5
{
  // ...
  "enableConsoleCommandsForAll": true
  // ...
}
```

## sweetPieMinimumPlayersToStart

The minimal amount of players to begin deathmatch. This setting is sweetpie only and does not affect vanilla server. Default is 5.

```json5
{
  // ...
  "sweetPieMinimumPlayersToStart": 5
  // ...
}
```

## sweetPieAllowCheats

Prevents the gamemode from disabling cheats. This setting is sweetpie only and does not affect vanilla server. Default is false.

```json5
{
  // ...
  "sweetPieAllowCheats": true
  // ...
}
```

## sweetPieChatSettings

Allows tuning settings related to in-game chat, such as message visibility radius.

```json5
{
  // ...
  "sweetPieChatSettings": {
    // Hearing distance in units. If player A says something and player B is farther away, they won't see that message.
    "hearingRadiusNormal": 123,
  },
  // ...
}
```

## sweetPieCommandEnabled

Enables or disables `/new2024` command that teleports player to SweetPie hall.

```json5
  // ...
  "sweetPieCommandEnabled": true
  // ...
```

## npcEnabled

Enables npc loading. Default is false.

```json5
{
  // ...
  "npcEnabled": false,
  // ...
}
```

## npcSettings

Optional npcs configuration. May not be present or can be an empty object which means all npcs are allowed to be loaded, provided `"npcEnabled"` is set to `true`.
`"NpcSettings"` consists of fields, each of which describes from what game file it is permitted to load an npc and additional restrictions of
how they should be spawned: in interior or exterior. By default all the npcs are allowed (`"npcSettings": {}`).
`"default":{}` field specifies `"spawnInInterior"` and `"spawnInExterior"` for all non-mentioned game files.

```json5
{
  // ...
  "npcSettings": {
    "default": {
      "spawnInInterior": true,
      "spawnInExterior": false
    },
    "Skyrim.esm": {
      "spawnInInterior": true,
      "spawnInExterior": false
    },
    "Dawnguard.esm": {
      "spawnInInterior": false,
      "spawnInExterior": true
    },
    "DragonBorn.esm": {
      "spawnInInterior": true,
      "spawnInExterior": true
    },
  },
  // ...
}
```

## npcAllowedBases

Optional list of NPC base fragments. When it is a non-empty array, only NPCs whose base
editorId contains (case-insensitively) one of the fragments are loaded from the game files.
Everything else is skipped at load time, so no whitelist checks are needed in the gamemode.

An empty array (or a missing key) disables the filter and allows every NPC.
The match is a substring match, so `"wolf"` matches `EncWolf`, `LvlWolf`, `Werewolf`, etc.

Many placed references do not use a species-named base directly, but a habitat-named
template such as `LvlAnimalForestPredator`, whose levelled list contains only wolves,
skeevers, spiders, bears and trolls. When the raw base editorId does not match, the
server resolves the NPC's template chain (`TPLT` and the levelled creature lists in
between) and loads the NPC only when *every* species reachable from it matches the
whitelist. This keeps spawning deterministic (the pick from a levelled list is random)
and lets all `LvlAnimal*Predator` bases spawn, while bases like `LvlAmbientCreatures`
(foxes, hares) or `LvlAnimalForestPrey` (deer) stay excluded unless you add matching
fragments such as `"fox"` or `"hare"`.

```json5
{
  // ...
  "npcAllowedBases": [
    "wolf",
    "bear",
    "draugr",
    "falmer",
    "chaurus",
    "giant",
    "troll"
  ],
  // ...
}
```

Note: this only decides which NPCs may exist on the server. Where they stand and what
they respawn is configured in `npc-spots.json` (see `NPC-SPOTS.txt`).

## weaponStaminaModifiers

This setting is only available with game mod file "SweetPie.esp".
This option allows you to flexibly adjust stamina forfeits of players' attacks using keywords set in the Creation Kit.
In case this field is not provided, some default, yet hardcoded, values are in use.

```json5
{
  // ...
  "weaponStaminaModifiers": {
    "WeapTypeDagger": 4.0,
    "WeapTypeShortSword": 5.0,
    "WeapTypeSword": 6.0,
    // ...
  }
  // ...
}
```

## additionalServerSettings

To automate the fetching of the latest server settings from GitHub, configure the additionalServerSettings in your server's startup script or configuration file as follows:

```json5
{
  // ...
  "additionalServerSettings": [
    {
      "type": "github",
      "repo": "your-org/server-settings-repo",
      "ref": "main", // Specify the branch, tag, or commit hash here
      "pathRegex": "^(common|indev)/.*", // No need to check for .json extension
      "token": "YOUR_GITHUB_PERSONAL_ACCESS_TOKEN"
    }
  ]
  // ...
}
```

## damageMultFormulaSettings
This setting allows you to control server damage mult formula through its variables.
If "damageMultFormulaSettings" is not present, the server will use some default values.

```json5
{
  // ...
  "damageMultFormulaSettings": {
    "multiplier": 1.0
  }
  // ...
}
```

Note: when this setting is absent, the default `multiplier` is `2.0`, i.e. NPCs
deal double damage to players.

## armorFormulaSettings

Scales the armor rating the server sums up from a player's worn armor, and adds
vanilla's hidden per-piece bonus. The server does not simulate armor skill,
perks or smithing, so without the multiplier the best armor set in the game
would only reduce incoming physical damage by ~17%.

```json5
{
  // ...
  "armorFormulaSettings": {
    "ratingMultiplier": 4.5,
    "hiddenPieceBonus": 3.0
  }
  // ...
}
```

* `ratingMultiplier` (default `4.5`): approximates armor skill 100 + perks +
  matching set. A full daedric set (rating ~94) goes from ~11% to ~51% damage
  reduction. Set it to `1.0` to use the raw vanilla base ratings.
* `hiddenPieceBonus` (default `3.0`): percent of damage reduction added per
  worn armor piece, equivalent to vanilla's hidden armor rating of 25 per piece.
  A full set therefore gets +12% (+15% with a shield) on top of the rating
  above, and daedric with a shield reaches the `fMaxArmorRating` cap (80%).
  Set it to `0` to disable.

Both values are read at server start, so they can be tuned later in
`server-settings.json` without rebuilding. If "armorFormulaSettings" is not
present, the defaults above are used.

## enableGamemodeDataUpdatesBroadcast

A boolean setting that controls hot-reloading behavior for connected clients.

* `false` (Default): Updates to gamemode scripts are applied to the server state but **not** broadcast to currently connected players. Existing players must re-login to receive the update. This ensures client stability if scripts do not support hot-reloading.
* `true`: Updates are immediately broadcast to all connected clients. Useful for local development, but may cause desync or client errors if the scripts are not designed to be re-applied at runtime.

```json5
{
  // ...
  "enableGamemodeDataUpdatesBroadcast": false
  // ...
}
