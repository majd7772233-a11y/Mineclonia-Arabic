# رسالة الاستئناف الرسمية — Mineclonia-Arabic

## مرجع GitHub عند آخر تدقيق — 2026-11-12

- Repository: majd7772233-a11y/Mineclonia-Arabic
- Branch الوحيد: work/arabic-engine-support
- آخر commit تنفيذ: 69c62784baf96e3456239d5b954055be5e855501 — Repair Arabic engine CI workflow and Lua validator.
- main: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d — بقي دون تعديل.
- هذا التحديث يوثق Run #109؛ اقرأ HEAD الحالي من GitHub قبل أي عمل جديد ولا تفترض أن SHA لم يتغير.
- لا تعديل على main، لا فرع جديد، لا merge، لا Force Push.

## Run #109 — SUCCESS

- Run ID: 43212062926; run number 109; HEAD SHA 69c62784baf96e3456239d5b954055be5e855501.
- بدأ 2026-11-12T11:46:47Z، اكتمل 2026-11-12T11:57:15Z.
- Workflow sanity — Job 131161916735: success.
- Catalog/source validation — Job 131161957086: success.
- Linux x64 build/tests/package — Job 131162246258: success.
- Windows x64 MSVC build/tests/package — Job 131162246670: success.

### Validation evidence
- builtin: 245/245 keys، 0 empty، source/bundle byte-identical.
- gettext: 1,575/1,575 effective، 0 untranslated، 0 fuzzy، LANG_CODE=ar.
- msgfmt check-format: success.
- Lua syntax: 210 builtin Lua files parsed across both trees.
- C++ source validator: 17 declarations / 17 registrations / 17 definitions.
- vcpkg cache restore succeeded on both platforms; cache save skipped on both because cache hit.
- CMake configure and client builds succeeded on both.
- TestArabicTextLayout: exact zero-failure 17-test gate passed on both.
- Linux package: executable, matching builtin, Noto font/license, ldd runtime resolution checked.
- Windows package: executable, source/package builtin SHA match, Arabic comma check, Noto font/license, Raqm/HarfBuzz/FriBidi/OpenAL/Ogg/Vorbis DLLs checked.

### Artifacts
Linux:
- ID 13941099252; size 22,718,197 bytes.
- SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941099252

Windows:
- ID 13941743588; size 20,672,848 bytes.
- SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.
- https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941743588

Run: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926

## قواعد العمل الإلزامية

1. اقرأ branch HEAD وmain من GitHub مباشرة قبل أي كتابة.
2. اعمل حصراً على work/arabic-engine-support؛ استخدم expected_sha عند تحريك المرجع.
3. كل تعديل كود/Workflow/CMake/Lua/Python/Asset يتطلب تحديث Markdown المناسب في commit نفسه، على الأقل PROJECT_STATE وWORKLOG append-only وHANDOFF وSUPPORT وPROGRESS وNEXT_SESSION_PROMPT؛ README وBUILD_TESTING/ROADMAP حسب الحاجة.
4. لا تخفف assertions ولا تحذف اختبارات.
5. لا تدفع أثناء CI؛ cancel-in-progress=true قد يلغي التشغيل.
6. احفظ cache restore/save على المنصتين.
7. عند فشل جديد، اقرأ log الأصلي والكود المحيط قبل تغيير السبب.

## الخطوة التالية

1. المستخدم يختبر Windows artifact أعلاه كحزمة كاملة، لا exe منفرداً فوق تثبيت قديم.
2. تحقق من فتح القائمة الرئيسية ثم Settings > Language واختيار العربية.
3. اطلب/افحص log حديثاً للتحقق من Lua parser errors وSHA warnings؛ لم يُثبت اختفاؤها على جهاز المستخدم بعد.
4. تابع clipboard copy/cut/paste round-trip، mouse drag، Shift selection، Ctrl+Left/Right وCtrl+Backspace/Delete للنص المختلط.
5. تابع clipping/shadows/centering/alignment/scrolling/fallback/emoji/supplementary Unicode/combining marks.
6. Android يبقى مؤجلاً بعد Desktop.

لا تعلن اكتمال دعم العربية قبل إغلاق تحقق GUI والتفاعل الفعلي.


## Independent artifact audit after Run #109 — 2026-11-12

The two Run #109 GitHub Artifacts were downloaded and checked independently:
- Linux ID 13941099252; downloaded outer ZIP SHA-256 matches `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows ID 13941743588; downloaded outer ZIP SHA-256 matches `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.
- Both outer ZIP archives passed integrity checks and contain the expected package archive.

Linux extracted package:
- `bin/luanti --version` exited 0 and reported Luanti 5.17.0, Release, RUN_IN_PLACE=1.
- `ldd` had no missing dependencies.
- Arabic language Lua row, Arabic builtin translation, Noto Sans Arabic and OFL license are present.
- Running the Arabic module from the extracted package passed 17/17, exact zero-failure summary.

Windows extracted package:
- `bin/luanti.exe`, Raqm/HarfBuzz/FriBidi/FreeType DLLs, OpenAL/Ogg/Vorbis DLLs, Arabic language Lua row, Noto font and license are present.
- Windows binary was not executed here; do not claim Windows GUI/runtime success.

## Updated immediate next step

Run Windows artifact 13941743588 on the user's Windows machine as a complete extracted portable package. Open the main menu, then Settings > Language, select Arabic, and retain the new log to check for Lua parser errors and `No SHA256 known for builtin file` warnings. Do not overwrite an older install with only the EXE. Then proceed to clipboard, drag/Shift selection, mixed-direction keyboard tests and rendering. Android remains deferred.

## Windows portable profile — active implementation batch

Current work starts from parent commit 01a3dbbb196da678448211f818f9f5ab0a99f8d0 on work/arabic-engine-support. Re-read the live branch and main refs before any writes. main was a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d at the pre-edit check and must remain untouched.

The intended Windows ZIP layout is:
- Read-only application files: package root.
- Writable profile: package/Minetest.
- Worlds: package/Minetest/worlds.
- Downloaded games: package/Minetest/games.
- Config, logs, cache, content/profile data derived from path_user: inside package/Minetest.

The current source batch adds:
- getPortableUserPath() in porting.h/porting.cpp.
- Windows RUN_IN_PLACE path_user separation and a copy-first migration from %APPDATA%/Minetest using a staging sibling; legacy data is retained, and an existing portable directory is never merged automatically.
- Windows config lookup isolation so it does not read minetest.conf above the portable profile.
- TestFileSys portable-path assertion; expected module total 14 tests.
- Linux/Windows TestFileSys workflow gates and a Windows package smoke test for synthetic AppData migration.
- Four phase plan in docs/ROADMAP.md.

This batch is not yet validated: no C++ build, TestFileSys execution, YAML/PowerShell parser, or CI result is recorded. Before commit, preflight all changed files, especially the PowerShell block in build-luanti-arabic-engine.yml. Then verify the branch and main heads again, commit all code and Markdown in one commit, and trigger one full workflow. Do not push an extra change while CI runs because cancel-in-progress=true.

After CI, inspect its complete Linux and Windows jobs. Only if the package smoke test succeeds should the Windows artifact be given to the user. User acceptance still must verify the actual directory, settings, downloaded games/worlds, logs, cache and absence of new AppData writes. Do not claim fully portable before the actual Windows test.

## Continuation after Run #110 — Windows package smoke-test refinement

Repository: majd7772233-a11y/Mineclonia-Arabic. Work only on work/arabic-engine-support. Never modify main, create branches, or force-push. Before writes, fetch the live refs and verify protected main remains unchanged.

Previous code HEAD: 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427.

Run #110 (ID 43231076763, same SHA) completed with overall failure:
- Workflow sanity and catalog validation passed.
- Linux Configure/Build passed; Arabic text-layout 17/17; TestFileSys 14/14; package checks and artifact upload passed. Artifact 13950892008, 22,717,628 bytes, SHA-256 ac9c301921e1443b0bd89383a1b2a0d58bbeb0def9e5a404a293ebdb12eaee1b.
- Windows Configure/Build passed; Arabic text-layout 17/17; TestFileSys 14/14; install and DLL checks passed.
- Windows package smoke failed during a duplicate TestFileSys invocation from the copied install directory. The job log contained no captured native output, so its underlying process reason is unknown. The preceding package `--worldlist=path` invocation returned success.
- Windows artifact was not uploaded.
- vcpkg cache restore passed on both hosts; save was skipped because cache restore was a hit.

The current workflow correction keeps the mandatory build-tree TestFileSys checks unchanged and removes only the duplicate TestFileSys invocation inside the package smoke test. It now runs `--worldlist=path` twice against the installed package and validates:
- world discovery beneath package/Minetest/worlds on both starts;
- migrated minetest.conf and cache marker contents;
- world.mt, game.conf, cache marker, and CACHEDIR.TAG presence under Minetest;
- the legacy AppData profile's file inventory remains unchanged.

This refinement and its Markdown updates are committed together in the follow-up code commit. Inspect branch HEAD directly rather than relying on a copied SHA in this prompt. After the push, inspect the new full workflow by exact HEAD SHA, all jobs/steps/logs and artifacts. Do not claim portability until Windows package smoke and upload both pass. A real Windows GUI acceptance test is still required.
