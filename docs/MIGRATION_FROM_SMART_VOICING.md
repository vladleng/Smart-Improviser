# Миграция фундамента из Smart Voicing

> Ветка: `stage-0-core-migration`  
> Источник: `vladleng/Smart-Voicing` stable 0.4 architecture  
> Цель: переиспользовать проверенный ARA/гармонический фундамент без переноса voicing-специфического технического долга.

## Принцип миграции

Smart Improviser **не является форком Smart Voicing**.

Компоненты переносятся по whitelist и сразу адаптируются под новую архитектуру:

```text
Studio Pro
    ↓
Smart Improviser ARA
    ↓
Shared Timeline Context
    ↓
ARAContextProvider
    ↓
Chord / Key normalization
    ↓
Harmonic Function
    ↓
будущий HarmonicSituation / Pattern / Tension / Strategy
```

## Перенесено

### Host-neutral Core

- `HarmonicContext` — только Chord/Key/TimeSignature/transport contract;
- `ChordModel` — нормализация ARA chord representation;
- `KeyModel` — нормализация тональности;
- `HarmonicFunction` — tonic/predominant/dominant, diatonic/chromatic, applied dominant, real-next-chord resolution evidence, modal-interchange candidate.

### Context / ARA infrastructure

- shared-memory bridge;
- отдельные harmonic/transport revisions;
- ARA Musical Context reader;
- Chord Track;
- Key Signatures;
- Tempo Entries;
- Bar Signatures;
- transport PPQ/time;
- `contextAt(ppq)` и `nextChordStartAfter(ppq)`.

### Tests

Перенесены только музыкально универсальные regression cases:

- C major normalization;
- `Dm7 | G7 | Cmaj7`;
- dominant resolution evidence;
- `D7 → G` как подтверждённый applied dominant;
- `Fm7` в C major как modal-interchange candidate.

## Изменения относительно Smart Voicing

### Новый namespace

```text
smartvoicing::harmony
→
smartimproviser::harmony
```

### Новый IPC ABI

Smart Improviser использует отдельное shared-memory пространство:

```text
MoonRiverStudio_SmartImproviser_HarmonicContext_v1
```

Оно намеренно несовместимо с Smart Voicing, чтобы два проекта не могли случайно читать состояние друг друга.

### ARA helper без UI

Первая версия ARA helper сделана headless. Старый debug/editor UI не переносится.

### Timeline helpers отделены от UI diagnostics

Старый `HarmonicContextDebugText.h` не переносится. Нужная логика выбора активного timeline event находится непосредственно в `ARAContextProvider`.

## Намеренно НЕ перенесено

- `CloseVoicingHarmonizer`;
- `LiveReharmonizer`;
- `VoiceOutput` / `VoiceSlot`;
- фиксированное `kVoiceCount = 4`;
- MIDI voice router;
- note ownership;
- sustain/retrigger logic;
- Distribution Mode;
- Harmony Mode;
- InstrumentPluginProcessor;
- InstrumentPluginEditor;
- Close / Drop / Spread / Quartal / Cluster voicing logic;
- Voice Leading;
- voicing-specific tests;
- старый Smart Voicing UI.

## TensionPolicy

`TensionPolicy` из Smart Voicing **не переносится кодом на этом этапе**.

Из него будут переиспользованы архитектурные идеи при создании собственного Smart Improviser `TensionEngine`:

- три уровня tension;
- explicit chord outranks inference;
- real next chord is resolution evidence;
- различие dominant → major target и dominant → minor target;
- functionally directed tensions.

Новый API не должен отвечать на voicing-специфический вопрос `isHarmonyCandidate()`. Он должен описывать роль звука, степень tension и ожидаемое разрешение для импровизации.

## Следующий шаг

После подтверждения сборки этого migration checkpoint:

1. добавить `HarmonicSituation`;
2. отделить coarse `HarmonicFunction` от более точной функциональной классификации;
3. добавить `HarmonicPattern`;
4. начать Pattern Recognizer с major/minor ii-V-I;
5. затем создать собственный `TensionEngine` Smart Improviser.
