# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2b — Pattern Recognizer`
- **Текущая рабочая версия:** `0.2c — Tritone Substitution`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Активная ветка:** `stage-2-tritone-substitution`
- **Активный PR:** #17 — `0.2c — Tritone Substitution`

`0.2b` принят после CI и live-test в Fender Studio Pro и слит в `main` через PR #16.

## Архитектурная граница

Stage 1 остаётся закрытым. `0.2c` не меняет ARA/context contract:

```text
Fender Studio Pro / ARA 2
        ↓
TimelineHarmonicSnapshot
(previous / current / next)
        ↓
buildHarmonicSituation()
        ↓
analyzeHarmonicSituation()
        ↓
HarmonicSituation
```

Core/Harmonic Engine остаётся host-neutral.

## Принятый checkpoint 0.2b

Подтверждено live-test:

- major `ii–V–I` на позициях `ii / V / I`;
- minor `iiø–V–i` на позициях `iiø / V / i`;
- `I–VI–ii–V`;
- dominant chain;
- pattern role / position / confidence;
- Stage 1 regression не обнаружен.

## Рабочая 0.2c — Tritone Substitution

Реализовано в активной ветке:

- `HarmonicFunction::substituteDominant`;
- substitute-dominant candidate / confirmed state;
- distinction ordinary `V7` vs `SubV7` по реальному next chord;
- major `ii–SubV–I`;
- minor `iiø–SubV–i`;
- boundary positions `ii / I` для tritone-substitution pattern;
- applied SubV к нетонической ступени, например `Ab7 → G` в C major;
- priority SubV над generic chromatic/secondary-dominant interpretation при подтверждённом semitone resolution;
- корректная guide-tone resolution logic: `Db7 → Cmaj7` даёт `F → E` и `Cb/B → C`;
- local-key inference для SubV сознательно отложен до `0.2d`;
- расширены CoreHarmonyTests и HarmonicEngineTests.

## Версия 0.2c

```text
Build label: Smart Improviser 0.2c
CMake:      0.2.4
Artifact:   Smart-Improviser-0.2c-Windows
Package:    Smart Improviser.vst3
```

## Минимальный live-test 0.2c

```text
Global key: C major
Dm7 → Db7 → Cmaj7
Current: Db7

Function         Substitute dominant
Relation         Chromatic
Pattern          Tritone substitution
Pattern position Substitute dominant | 2 / 3
Resolution       Cmaj7 | CONFIRMED
Confidence       confirmed | unique
```

Дополнительные проверки:

```text
A minor: Bm7b5 → Bb7 → Am
```

и applied SubV:

```text
C major: Ab7 → G
```

где Ab7 должен трактоваться как substitute dominant к G, а не как ordinary secondary dominant.

## Линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACTIVE]
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Следующий checkpoint после 0.2c

`0.2d — Local Key Center` должен развить уже существующую temporary-center идею в полноценное автоматическое определение локальных/субтональных центров без необходимости постоянно менять project key в DAW.

План 0.2d:

- candidate local center;
- temporary tonicization;
- confirmed local center;
- отличие local center от настоящей modulation;
- `ii–V`, `iiø–V`, applied dominant и SubV evidence;
- возврат к global key;
- кейсы джазовых стандартов с частыми временными отклонениями.

## Что делать следующим

1. дождаться CI PR #17;
2. при зелёном CI установить `Smart-Improviser-0.2c-Windows`;
3. провести отдельный live-test 0.2c;
4. при успешном тесте принять checkpoint и перейти к `0.2d`.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #17 — `0.2c — Tritone Substitution`.
