# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2c — Tritone Substitution`
- **Текущая рабочая версия:** `0.2d — Local Key Center`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Активная ветка:** `stage-2-local-key-center`
- **Активный PR:** #18 — `0.2d — Local Key Center`

`0.2c` принят после CI и live-test в Fender Studio Pro и слит в `main` через PR #17.

## Архитектурная граница

Stage 1 остаётся закрытым. `0.2d` не меняет ARA/context contract и не меняет project key в DAW:

```text
Fender Studio Pro / ARA 2
        ↓
TimelineHarmonicSnapshot
(previous / current / next)
        ↓
Harmonic Engine
        ├── global interpretation
        └── Local Key Center Analyzer
                ↓
          local interpretation
        ↓
HarmonicSituation
```

Core остаётся host-neutral.

## Что принято к 0.2c

Подтверждено live-test:

- major `ii–V–I` на позициях `ii / V / I`;
- minor `iiø–V–i` на позициях `iiø / V / i`;
- `I–VI–ii–V`;
- dominant chain;
- secondary dominants;
- tritone substitution;
- major/minor `ii–SubV–I`;
- applied SubV;
- guide-tone resolution;
- Stage 1 regression не обнаружен.

## Рабочая 0.2d — Local Key Center

Главная цель: global key проекта остаётся стабильным, а движок отдельно определяет активный локальный/временный тональный центр.

Пример:

```text
Global key: F major
Em7b5 → A7 → Dm

Current: A7
Global interpretation: chromatic / applied dominant
Local center: D minor
Local function: V / Dominant
Local pattern: minor iiø–V–i
```

### Модель local center

```text
candidate
    ↓
tonicized / temporary
    ↓
established local center
    ↓
modulationCandidate
```

`modulationCandidate` никогда автоматически не меняет global project key. Это только evidence-backed гипотеза для будущего ambiguity layer.

### Реализовано

- отдельный host-neutral `LocalKeyCenterAnalyzer`;
- `KeyCenterStatus`: `candidate`, `tonicized`, `established`, `modulationCandidate`;
- unresolved `ii–V` / `iiø–V` → candidate local center;
- `V→target` → confirmed temporary tonicization;
- `SubV→target` → confirmed temporary tonicization;
- полный local `ii–V–I` / `iiø–V–i` → established local center;
- полный local `ii–SubV–I` / `iiø–SubV–i` → established local center;
- remote centers могут находиться вне набора ступеней global key;
- `localHarmonic` хранит функцию current chord относительно local center;
- `localPattern` хранит pattern относительно local center;
- global-key cadence не создаёт redundant local center;
- `V–I–X`, где X поддерживает local center и конфликтует с global key, создаёт только `modulationCandidate`;
- отдельные evidence flags для candidate / tonicization / local cadence / modulation evidence;
- отдельный набор `SmartImproviserLocalKeyCenterTests`.

## Версия 0.2d

```text
Build label: Smart Improviser 0.2d
CMake:      0.2.5
Artifact:   Smart-Improviser-0.2d-Windows
Package:    Smart Improviser.vst3
```

## Live-test после зелёного CI

Основной кейс:

```text
Global key: F major
Em7b5 → A7 → Dm
```

На `A7` ожидается:

```text
Key          F major
Local center D minor | local
```

На `Em7b5` в паре `Em7b5 → A7` ожидается candidate D minor; в текущем diagnostic UI он отображается как `D minor | temporary`.

Дополнительные проверки:

```text
C major: D7 → G        → G major | temporary
C major: Ab7 → G       → G major | temporary
C major: Dm7 → G7 → C  → Local center = -
C major: G#m7 → C#7 → F#maj7 → F# major | local
```

## Линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACTIVE]
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Следующий checkpoint после 0.2d

`0.2e — Ambiguity / Confidence` должен научить движок хранить несколько допустимых трактовок и разрешать конфликты между global и local harmonic interpretation без преждевременного выбора одного варианта.

## Что делать следующим

1. дождаться CI PR #18;
2. при зелёном CI установить `Smart-Improviser-0.2d-Windows`;
3. провести отдельный live-test local centers;
4. при успешном тесте принять checkpoint и перейти к `0.2e`.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #18 — `0.2d — Local Key Center`.
