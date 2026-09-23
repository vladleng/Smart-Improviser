# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2b — Pattern Recognizer`
- **Следующая рабочая версия:** `0.2c — Tritone Substitution`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Принятый PR 0.2b:** #16 — `0.2b — Pattern Recognizer`

## Принятие checkpoint 0.2b

`0.2b — Pattern Recognizer` **принят по результатам CI и live-test в Fender Studio Pro**.

Подтверждено:

- Windows CI для финального HEAD PR #16 — success;
- artifact `Smart-Improviser-0.2b-Windows` опубликован и установлен;
- major `ii–V–I` корректно распознаётся на позициях `ii / V / I`;
- minor `iiø–V–i` корректно распознаётся на позициях `iiø / V / i`;
- `I–VI–ii–V` корректно распознаётся в доступном previous/current/next окне;
- dominant chain корректно распознаётся;
- pattern role / position / confidence соответствуют ожидаемому evidence;
- Stage 1 ARA/context regression не обнаружен;
- Stage 1 contract не расширялся: Harmonic Engine по-прежнему получает host-neutral `previous / current / next`.

## Архитектурная цепочка

```text
Fender Studio Pro / ARA 2
        ↓
TimelineHarmonicSnapshot
        ↓
buildHarmonicSituation()
        ↓
analyzeHarmonicSituation() / Harmonic Engine
        ↓
HarmonicSituation
        ↓
Diagnostic UI / future product UI
```

Core и Harmonic Engine остаются host-neutral. UI только отображает уже рассчитанный `HarmonicSituation`.

## Что закрыто к 0.2b

- basic harmonic functions;
- major `ii–V–I` на всех доступных позициях;
- minor `iiø–V–i` на всех доступных позициях;
- `I–VI–ii–V`;
- secondary dominant;
- dominant chains;
- pattern role / position;
- evidence-aware confidence;
- temporary local center для подтверждённого applied dominant;
- regression tests;
- Stage 2 diagnostics в plugin UI.

## Следующий подэтап — 0.2c Tritone Substitution

Цель: научить Harmonic Engine отличать обычный dominant от substitute dominant и распознавать tritone-substitution контекст.

План:

- добавить `SubV` / substitute-dominant functional role;
- распознавать `ii–SubV–I`;
- различать `V7` и `SubV7` относительно target center;
- добавить substitute-dominant resolution logic;
- корректно определять pattern role / position для SubV;
- не спутывать chromatic dominant, secondary dominant и tritone substitute;
- добавить regression tests для major/minor target cases и boundary cases;
- провести отдельный live-test checkpoint в Fender Studio Pro.

## Дальнейшая линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [NEXT]
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Что делать следующим

1. начать реализацию `0.2c — Tritone Substitution` в отдельной ветке от принятого `main`;
2. не расширять закрытый Stage 1 context contract;
3. сохранить host-neutral Harmonic Engine;
4. после зелёного CI передать `0.2c` на отдельный live-test;
5. не смешивать полноценный Local Key Center inference с `0.2c` — это отдельный checkpoint `0.2d`.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine.
