# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2a / 0.2a fix1`
- **Текущая рабочая версия:** `0.2b — Pattern Recognizer`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Активная ветка:** `stage-2-pattern-recognizer`
- **Активный PR:** #16 — `0.2b — Pattern Recognizer`

PR #15 (`0.2a fix1`) принят по live-test и слит в `main`. `0.2b` начат отдельной веткой от принятого checkpoint.

## Архитектурная граница

Stage 1 остаётся закрытым и не изменяется ради Pattern Recognizer.

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
Pattern Recognizer / HarmonicSituation
```

`0.2b` использует только host-neutral `previous / current / next` и не знает ничего об ARA, JUCE, shared memory или Fender Studio Pro.

Важное правило confidence: движок не выдумывает отсутствующие события. Полный трёхаккордовый pattern может быть `confirmed`; boundary-position, где доступна только структурно сильная пара, получает `high` или `medium` в зависимости от объёма evidence.

## Принятый checkpoint 0.2a / 0.2a fix1

Подтверждено live-test в Fender Studio Pro:

- Stage 1 ARA/context regression не обнаружен;
- basic functions работают;
- `Dm7 → G7 → Cmaj7` на G7 → `Major ii-V-I`, `Dominant | 2 / 3`, confirmed resolution;
- `D7 → G7` в C major → `Secondary dominant`, local center `G major | temporary`;
- диагностический UI показывает Stage 2 analysis.

## Рабочая 0.2b — Pattern Recognizer

Уже реализовано в активной ветке:

- major `ii–V–I` для позиций `ii / V / I`;
- minor `iiø–V–i` для позиций `iiø / V / i`;
- сохранён `confirmed` для полного `ii–V–I` на позиции V;
- boundary positions используют реальный pair evidence и не получают ложный `confirmed`;
- `I–VI–ii–V` для внутренних и boundary positions;
- VI допускается как `vi7` или `VI7`;
- dominant chain, например `A7 → D7 → G7`;
- dominant chain имеет приоритет над одиночной трактовкой secondary dominant;
- pattern evidence flags: `previousChord`, `nextChord`, `confirmedResolution`;
- pattern confidence переносится в общий `HarmonicSituation` только если она сильнее уже имеющейся confidence;
- false-positive guard для неправильного качества V chord;
- расширенные `SmartImproviserHarmonicEngineTests`.

### Confidence policy 0.2b

```text
Dm7 → G7 → Cmaj7, current G7
= Major ii-V-I | Dominant 2/3 | confirmed

Dm7 → G7, current Dm7
= Major ii-V-I | Predominant 1/3 | high

G7 → Cmaj7, current Cmaj7
= Major ii-V-I | Resolution 3/3 | high

Cmaj7 → A7 → Dm7, current A7
= I-VI-ii-V | Preparation 2/4 | high

Cmaj7 → Am7, current Cmaj7
= I-VI-ii-V | Tonic 1/4 | medium

A7 → D7 → G7, current D7
= Dominant chain | Dominant 2/3 | confirmed
```

## Версия 0.2b

```text
Build label: Smart Improviser 0.2b
CMake:      0.2.3
Artifact:   Smart-Improviser-0.2b-Windows
Package:    Smart Improviser.vst3
```

## Что ещё нужно закрыть перед принятием 0.2b

1. Windows CI должен пройти все regression tests;
2. установить artifact `Smart-Improviser-0.2b-Windows`;
3. live-test major `ii–V–I` на позициях ii/V/I;
4. live-test minor `iiø–V–i` на позициях iiø/V/i;
5. live-test `I–VI–ii–V`;
6. live-test dominant chain;
7. подтвердить отсутствие Stage 1 regression;
8. после принятия перейти к `0.2c — Tritone Substitution`.

## Линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACTIVE]
0.2c — Tritone Substitution
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #16 — `0.2b — Pattern Recognizer`.
