# سجل قوالب Mineclonia المصدرية

هذه القائمة تربط ملفات PO العربية بالقوالب الإنجليزية التي استُخرجت منها الرسائل.

## مرجع المصدر

- المستودع المرجعي المستخدم لجلب القوالب: https://github.com/mineclonia-mirror/mineclonia
- صفحة README للمصدر تذكر أن مستودع المشروع الأساسي مستضاف على Codeberg: https://codeberg.org/mineclonia/mineclonia
- لقطة المصدر المرجعية المسجلة عند بدء المشروع: `24521bf6c99ddfc82bf52a60420a23902282c2a0`
- كل قيمة أدناه هي Git blob SHA للقالب الذي جرى جلبه.

## القوالب المستخدمة

| ملف قالب المصدر | Blob SHA |
|---|---|
| `mods/CORE/mcl_init/locale/template.pot` | `2eb9b575a89a28a47c78be0e84f6cc56241afc40` |
| `mods/CORE/mcl_liquids/locale/template.pot` | `60f8e392de33e05203da766760ecafa8744d2b8a` |
| `mods/ENVIRONMENT/mcl_lightning/locale/template.pot` | `5ee35752a08e58f80845cedc91518705d15c8faa` |
| `mods/ENVIRONMENT/mcl_raids/locale/template.pot` | `dfa5875e62f18811cb601aafc40890b5e79574b3` |
| `mods/ENVIRONMENT/mcl_void_damage/locale/template.pot` | `6df8f24710572f990c63b5abe66ef5dce99a96e7` |
| `mods/ENVIRONMENT/mcl_weather/locale/template.pot` | `70b8a3b6eea48537c858866b488adce4ec99f5fb` |
| `mods/MISC/findbiome/locale/template.pot` | `187f4d05ba0f4088c609effdcf00b216c1025086` |
| `mods/MISC/mcl_commands/locale/template.pot` | `d7d7308d18207be0b2f2fda43eb9abf432a05c1e` |
| `mods/MISC/mcl_privs/locale/template.pot` | `30b5e49925243f82dc76e4a54922c0e4c2ab3a5b` |
| `mods/MISC/mcl_wip/locale/template.pot` | `74a62db559c9253a0a4bc1ddec882d55f52553af` |
| `mods/PLAYER/mcl_gamemode/locale/template.pot` | `146de67d1d759b2c52597acc54fba236442cd1b5` |
| `mods/PLAYER/mcl_hunger/locale/template.pot` | `e7722db8943cf3b99099bab6cb28d79698ef9a70` |
| `mods/PLAYER/mcl_player/locale/template.pot` | `a3cc7f69a68a157c7ab537e1308c61258c5e066e` |
| `mods/PLAYER/mcl_serverplayer/locale/template.pot` | `07a07dc861c530bd65994643081a8b5df81c53fa` |
| `mods/PLAYER/mcl_skins/locale/template.pot` | `a5fe48fd67df1049ffdd14cba58edb6e18731da4` |
| `mods/PLAYER/mcl_spawn/locale/template.pot` | `7750947409c95755256ee10472a4aee3d4cb169a` |
| `mods/PLAYER/playerphysics/locale/template.pot` | `24d94d09f848117347627bb4b5c92d239c9feac8` |
| `mods/HUD/hudbars/locale/template.pot` | `f7b6216532445a74f9528e75c2093aa1368d97cb` |
| `mods/HUD/mcl_bossbars/locale/template.pot` | `44a63e315df0864480634733f8f45e6e3a13efc4` |
| `mods/HUD/mcl_credits/locale/template.pot` | `29450c42f9a17fc1d022433730193f5a204f8a07` |
| `mods/HUD/mcl_death_messages/locale/template.pot` | `608374bee734d9a2fa5ed5b835c29fe71381e71e` |
| `mods/HUD/mcl_experience/locale/template.pot` | `dd6263035cdca3db1b4c5243c29f44de3980e43d` |
| `mods/HUD/mcl_game_meta/locale/template.pot` | `95d13bd25e458da517b8fcbfc0e9381b7b58e5f4` |
| `mods/HUD/mcl_hbarmor/locale/template.pot` | `f00cfe425cab3b68bf72999b92b10f3338ee031c` |
| `mods/HUD/mcl_info/locale/template.pot` | `9e077bbb1cdaf50990a15403c6ce318a6e4f964f` |
| `mods/HUD/mcl_inventory/locale/template.pot` | `360db8133ecaf0b73b8ff7cc085df14495f7d325` |

## مبدأ التتبع

- نص `msgid` في كل PO يُنسخ من القالب المقابل دون إعادة صياغة.
- الترجمة العربية وحدها توضع في `msgstr`.
- عند تحديث القوالب، لا نستبدل هذه الملفات آلياً؛ نراجع الفروقات أولاً ثم نحدّث الترجمات.
