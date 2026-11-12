# تسليم دعم العربية في محرك Luanti — نقطة الاستئناف الرسمية

التاريخ: 2026-11-12
Repository: majd7772233-a11y/Mineclonia-Arabic
Branch الوحيد: work/arabic-engine-support
رأس تنفيذ Workflow/Lua: 69c62784baf96e3456239d5b954055be5e855501
main المفحوص: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (لم يُعدّل).
Run الأخير: #109 / 43212062926 على SHA 69c62784baf96e3456239d5b954055be5e855501 — completed/success.

## ما اكتمل

- أُصلح Workflow في commit 69c62784baf96e3456239d5b954055be5e855501: أربع Jobs فريدة sanity/catalog/Linux/Windows؛ أزيلت المقاطع اليتيمة والتعريفات المكررة؛ اكتملت شروط Bash وPowerShell.
- صُحح double escaping في tools/validate_lua_syntax.py.
- حُفظ cancel-in-progress=true وvcpkg cache restore/save في Linux وWindows.
- Run #109 نجح بكل وظائفه الأربع.
- catalog: 245/245 builtin keys و1,575/1,575 gettext translations؛ 0 empty و0 fuzzy.
- msgfmt: success.
- Lua parser: 210 ملفات builtin عبر الشجرتين.
- Arabic source validator: 17 declarations / 17 registrations / 17 definitions.
- Linux وWindows: CMake configure، client build، اختبار TestArabicTextLayout (17/17 و0 failures)، install/package/upload — كلها success.
- Linux artifact ID 13941099252، SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows artifact ID 13941743588، SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.

## Run #109 وروابط الحزم

Run: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926

- Linux ZIP/tar.gz artifact: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941099252
  - ID 13941099252; 22,718,197 bytes; SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows ZIP artifact: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941743588
  - ID 13941743588; 20,672,848 bytes; SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.

## ما لم يختبر بعد

- لم يُشغّل المستخدم Windows artifact على جهازه بعد.
- لا يوجد إثبات فعلي حتى الآن لاختفاء خطأ قائمة Lua أو تحذيرات SHA-256 على جهاز المستخدم.
- clipboard copy/cut/paste، mouse drag، Shift selection، Ctrl+Left/Right، Ctrl+Backspace/Delete بالنص المختلط، scrolling، clipping، shadows، centering/alignment، emoji، supplementary Unicode وcombining marks تحتاج اختبار GUI.
- Android مؤجل إلى ما بعد استقرار Desktop.

## نقطة التوقف التالية بدقة

1. استخدم Windows artifact 13941743588 كحزمة كاملة مستقلة؛ لا تنقل exe وحده إلى تثبيت قديم.
2. شغّل التطبيق وافتح القائمة الرئيسية.
3. افتح Settings > Language وتحقق من ظهور العربية واختيارها.
4. التقط log جديداً وابحث عن `Failed to load and run script` و`No SHA256 known for builtin file`. لا تفترض اختفاء التحذير قبل وجود log المستخدم.
5. ابدأ اختبارات التحرير التفاعلية المذكورة أعلاه، ثم Rendering GUI.
6. إذا ظهر فشل جديد، اقرأ السجل والكود المحيط أولاً، وجمّع الإصلاحات مع توثيق Markdown في commit واحد. لا تدفع أثناء CI بسبب cancel-in-progress=true.

هذا التحديث بعد Run #109 توثيقي فقط؛ لا يعدل كود المحرك أو Workflow.


## فحص الـArtifacts بعد التنزيل

بعد Run #109 تم تنزيل الحزمتين وفحصهما بشكل مستقل:
- Linux Artifact 13941099252: SHA-256 المنزّل يطابق `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`. الأرشيف سليم، وELF يعمل من جذر الحزمة، و`--version` يعرض Luanti 5.17.0. `ldd` بلا مكتبات مفقودة.
- من نفس الحزمة المستخرجة، نجح `TestArabicTextLayout` بنتيجة 17/17 وصفر إخفاقات.
- ملف builtin الخاص باختيار اللغة وخط Noto والترخيص موجودة.
- Windows Artifact 13941743588: SHA-256 المنزّل يطابق `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`، وZIP الداخلي سليم، ووجدت exe وDLLs وbuiltin وخط Noto والترخيص. لم يتم تشغيل exe على Windows في هذه البيئة.

## نقطة الاستئناف الآن

1. Windows artifact 13941743588 هو الخطوة التالية على جهاز المستخدم: استخرج الحزمة كاملة وشغّل `bin/luanti.exe` منها.
2. افتح القائمة الرئيسية ثم Settings > Language، واختر العربية.
3. احتفظ بسجل تشغيل جديد؛ لا تعتبر Lua menu error أو SHA warnings منتهية قبل التحقق من السجل.
4. استكمل اختبارات clipboard round-trip وmouse drag وShift selection والتنقل والحذف المختلط، ثم Rendering GUI.
5. لا تبدأ Android قبل استقرار Desktop.

## 2026-11-12 — Windows portable profile work

Starting point: 01a3dbbb196da678448211f818f9f5ab0a99f8d0 on work/arabic-engine-support. The protected main head checked before edits was a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d.

The Windows RUN_IN_PLACE profile is being separated from the app/share root:
- share stays at the Luanti package root.
- user data moves to <package>/Minetest.
- path_cache remains under path_user.
- Windows config search is restricted to Minetest/minetest.conf.
- Existing %APPDATA%/Minetest is copied on first launch using a staging directory; the source remains untouched. If the portable directory already exists, no automatic merge occurs.

The batch also adds a portable path unit test and Windows package integration test in the build workflow. TestFileSys is expected to report 14 tests on both platforms.

**Validation status:** not built or run yet. Workflow YAML, PowerShell and C++ still need preflight and the full CI run. Do not claim the application is portable until the Windows smoke test and the user's Windows GUI acceptance both pass.

**Next step:** inspect the final diff, run available YAML/Bash/PowerShell/C++ preflight, re-check branch refs, commit all code and Markdown together, then wait for one complete CI. After CI, test the Windows artifact on the user's computer.

## Handoff update after Run #110

Run #110 (43231076763) for commit 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427:
- Linux build, Arabic tests 17/17, TestFileSys 14/14, install verification and artifact upload passed.
- Windows configure/build, Arabic tests 17/17, TestFileSys 14/14 and install/runtime DLL checks passed.
- Windows packaging failed only in the package smoke step, during a duplicate TestFileSys invocation from the copied install tree. Its process output was empty in the job log; do not claim a specific C++ or migration root cause from this.
- The first packaged `--worldlist=path` process returned exit code 0.
- Linux artifact exists (ID 13950892008); Windows artifact was not uploaded.

The follow-up workflow correction removes only the duplicate package-level TestFileSys invocation. It keeps the mandatory build-tree TestFileSys checks on both platforms and strengthens the actual package probe with first- and second-launch world discovery, migrated-file content checks, and AppData immutability.

Next: inspect the exact patch, verify branch/main refs, commit the correction with all documentation in the same commit, then inspect the new CI run. If Windows smoke passes, inspect Artifact ID, size and digest before giving the Windows ZIP to the user.


## Handoff after Run #111 — fix the Windows smoke harness and advance the portable audit

Run #111 (43236622672, HEAD bb3df102f129b2ef11b50f16911f292e20881f74) compiled and tested both native targets. Linux passed TestArabicTextLayout 17/17, TestFileSys 14/14, package checks, and upload (Artifact 13953262107; 22,719,344 bytes; SHA-256 86dd6ecab7085bcde58b03fbd2225e347bed4326e86c46753fbf28070bc6889d). Windows passed configure/build, 17/17 Arabic tests, 14/14 FileSys tests, install and DLL checks, but failed the package smoke test because it expected stdout from a Windows GUI executable linked with `/SUBSYSTEM:WINDOWS`. The process returned 0 and output was empty; the script stopped before checking the copied profile. Do not describe this as a proven migration bug. No Windows artifact was uploaded.

This follow-up changes the package test to rely on exit codes and on-disk content, adds a 15th TestFileSys case to verify that the actual world/game discovery functions use a path_user containing spaces, and tests both first-run migration and the existing-profile/no-merge case from package directories containing spaces.

Next steps after this commit:
1. Re-fetch live refs; only move `work/arabic-engine-support` from the expected previous SHA, and leave `main` untouched.
2. Run one full CI workflow. Check catalog validation, configure/build, TestArabicTextLayout 17/17, TestFileSys 15/15, Windows smoke checks, and both artifact uploads.
3. If Windows fails, diagnose the exact failed assertion and do not weaken assertions just to make CI green.
4. After CI success, provide the newly uploaded Windows artifact and request real-device Phase 4 acceptance. Test Content tab downloads, creating worlds, config/log/screenshot/cache paths, no new AppData writes, and a removable/non-system drive.
5. Do not move to font/UI or RTL rendering work until portable-profile acceptance is verified. Android stays deferred.
