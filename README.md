# SeriousSpeedrun — таймер для спидрана Serious Sam Classic

Таймер RTA и IGT, LiveSplit и спидометр для Serious Sam Classic (The First Encounter 1.05, The Second Encounter
1.07). **Игру не меняет**: подходит для забегов обычной игры, работает и вместе с другими модами (например,
[SeriousHL](https://github.com/animeliodas/SeriousHL) — движение Half-Life). Плагин
[Serious Sam Classics Patch](https://github.com/SamClassicPatch/SuperProject) 1.9.4; порт
[SeriousBridge](https://github.com/animeliodas/SeriousBridge) (таймер — его модуль ядра `core/sam/Speedrun`).

> Сделано с помощью Claude Code (ИИ). Проверено скриптом в настоящих играх (TFE и TSE, `tools/test_speedrun.py`,
> ниже); с настоящим LiveSplit — проверено пользователем.

## Установка (один клик)

1. Скачай `SeriousSpeedrun-<версия>.zip` со страницы [Releases](https://github.com/animeliodas/SeriousSpeedrun/releases),
   распакуй куда угодно.
2. Запусти `Install.cmd`: сам найдёт TFE и TSE в Steam (или GOG; или перетащи папку игры на `Install.cmd`), поставит
   Classics Patch 1.9.4, если его нет (с его страницы релизов на GitHub, SHA-256 проверяется), положит
   `SeriousSpeedrun.dll` в `Bin\Plugins` и ярлык на рабочий стол.
3. Играй через Classics Patch (ярлык, `Bin\SeriousSam_Custom.exe` или в Steam параметр `+game ClassicsPatchMod`).

Удаление — `Uninstall.cmd`. Вручную: `TFE\SeriousSpeedrun.dll` / `TSE\SeriousSpeedrun.dll` в `<игра>\Bin\Plugins\`.

## Таймер

Встроенный таймер игры раннеры не используют; этот считает сам:
- **RTA** — настоящее время (часы ПК) от старта: загрузки, меню и пауза входят;
- **IGT** — время игры: её тики (по 1/20 с), пока идёт забег. Загрузки уровней, пауза, меню и Netricsa не идут
  (в одиночной игре на них нет тиков), загрузка сохранения не сбивает.

Старт — первое управление игроком после новой игры (после заставки и Netricsa; `sr_iStartOn 0` — сразу на новой
игре). Сплит — конец каждого уровня (когда игра пишет его статистику; или смена уровня без неё). Финиш — конец
игры. Новая игра сбрасывает забег. На экране справа сверху — `RTA` и `IGT` (`LS` — LiveSplit подключён, `#N` —
сплитов). Заставка меню и демо забегом не считаются.

## LiveSplit

Правой кнопкой по LiveSplit → Control → **Start TCP Server** (порт 16834; в старых версиях — компонент LiveSplit
Server). Игра подключится сама (пробует раз в 3 с) и будет запускать таймер, ставить сплиты, финишировать и
сбрасывать. **Game Time** в LiveSplit всегда равно IGT — для забега по IGT сравнивай по Game Time (правой кнопкой →
Compare Against → Game Time). Сплитов в LiveSplit — по одному на уровень. Без LiveSplit таймер работает сам.

## Спидометр

Под прицелом — скорость по земле: м/с (`sr_iSpeedUnits 1` — юниты Half-Life, для SeriousHL); зелёная — быстрее
бега Сэма (10 м/с).

## Консоль (`~`)

| Команда | По умолчанию | |
|---|---|---|
| `sr_bShowTimer` | 1 | таймер на экране |
| `sr_bShowSpeed` | 1 | спидометр |
| `sr_iSpeedUnits` | 0 | 0 — м/с, 1 — юниты HL |
| `sr_bLiveSplit` | 1 | связь с LiveSplit |
| `sr_strLiveSplit` | `127.0.0.1:16834` | его адрес |
| `sr_iStartOn` | 1 | 1 — старт на первом управлении, 0 — на новой игре |
| `sr_Status()` | | состояние |

Команд, которые меняли бы время забега, нет (тестовые — только в режиме разработки, `SERIOUSBRIDGE_DEV=1`).

## Сборка и проверка

VS 2022 Build Tools (v143, x86); рядом — ядро и SDK Classics Patch:
```
<папка>/seriousspeedrun      этот репозиторий
<папка>/seriousbridge        git clone https://github.com/animeliodas/SeriousBridge seriousbridge
<папка>/_deps/SuperProject   git clone --branch 1.9.4 --recurse-submodules https://github.com/SamClassicPatch/SuperProject.git
```
`plugin/build.sh` → `plugin/dist/Release_TFE105|Release_TSE107/SeriousSpeedrun.dll`; `python tools/make_release.py` →
`release/SeriousSpeedrun-<версия>.zip`.

Оракул — `python tools/test_speedrun.py --game "<папка игры>" --launch` (вместо LiveSplit — заглушка на порту 16834,
записывает команды; один забег по шагам, на обычной игре):

| Проверка | TFE | TSE |
|---|---|---|
| старт на управлении; LiveSplit: reset, starttimer, game time на паузе и задано | PASS | PASS |
| IGT идёт как реальное время (1.000); на паузе стоит, RTA идёт | PASS | PASS |
| Game Time LiveSplit = IGT (setgametime 10 раз в секунду) | PASS | PASS |
| выход с уровня → сплит (и в LiveSplit); загрузка в IGT не входит | PASS | PASS |
| конец игры → последний сплит, таймер стоит | PASS | PASS |
| новая игра → сброс | PASS | PASS |

## Лицензия

GPL-2.0, как Serious Engine 1 и Classics Patch.
