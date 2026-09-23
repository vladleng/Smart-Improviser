# Smart Improviser Core Data Model — 0.0b

## Назначение

`0.0b` фиксирует host-neutral data model Smart Improviser. Этот слой описывает **результат музыкального анализа**, но не должен сам выполнять преждевременное распознавание сложных harmonic patterns или local key centers.

Главный поток данных:

```text
Timeline / ARA Adapter
        ↓
TimelineHarmonicSnapshot
        ↓
buildHarmonicSituation()
        ↓
HarmonicSituation
        ↓
будущие analyzers / Tension Engine / Improvisation Engine
```

## HarmonicSituation

Центральная сущность Core.

Хранит:

- previous/current/next normalized chord;
- global `KeyCenter`;
- local `KeyCenter` — поле существует, но Stage 0 его не угадывает;
- `HarmonicAnalysis`;
- `HarmonicPattern`;
- `ResolutionTarget`;
- `AnalysisEvidence` / confidence;
- позицию PPQ.

`HarmonicSituation` не является копией ARA context. Это нормализованная музыкальная модель, не знающая о конкретной DAW.

## TimelineHarmonicSnapshot

Минимальный входной контракт между timeline/context adapter и Core:

- PPQ position;
- previous chord;
- current chord;
- next chord;
- global key.

ARA, VST3 и Fender Studio Pro types не должны пересекать эту границу.

## AnalysisEvidence и ConfidenceLevel

Все будущие анализаторы используют единый механизм evidence.

Уровни confidence:

```text
unknown
low
medium
high
confirmed
```

Evidence flags позволяют отличить:

- explicit chord/key information;
- previous/next chord evidence;
- diatonic/chromatic relation;
- confirmed resolution;
- pattern match;
- modal interchange;
- inferred local center.

Принцип: **несколько музыкально допустимых интерпретаций не должны маскироваться под абсолютную уверенность**.

## KeyCenter

`KeyCenter` отделён от простого `NormalizedKey`.

Scope:

```text
global
local
temporary
modal
```

На `0.0b` только global center создаётся напрямую из explicit Key Track. Local/temporary/modal center будут выводиться отдельным анализатором Stage 2.

## HarmonicPattern

Data model поддерживает тип паттерна, роль текущего аккорда, индекс позиции и длину паттерна.

Первичный vocabulary включает:

- Major ii–V–I;
- Minor iiø–V–i;
- V–I;
- I–VI–ii–V;
- Secondary dominant;
- Tritone substitution;
- Backdoor dominant;
- Minor iv→I;
- Passing/Common-tone diminished;
- Dominant chain;
- Modal vamp.

`0.0b` **не реализует Pattern Recognizer**. До запуска Stage 2 builder возвращает `none`, а не делает догадку.

## ResolutionTarget

Resolution — отдельная сущность, потому что tension и improvisation strategy должны знать не только текущую harmony, но и **куда она движется**.

Хранит:

- target chord/root/quality;
- confirmed/unconfirmed status;
- evidence;
- набор `ResolutionMove`.

Для подтверждённого обычного dominant resolution builder уже умеет материализовать две структурные tendency-линии:

```text
3rd of V → root of target
b7 of V → 3rd of target
```

Например `G7 → Cmaj7`:

```text
B → C
F → E
```

Это не полноценный voice-leading engine, а минимальная формализация resolution logic.

## TensionLevel

Core contract фиксирует ровно три пользовательских уровня:

```text
1 Stable
2 Color
3 Outside / Maximum
```

Подробные правила выбора материала относятся к Stage 4 `Tension Engine`.

## ImprovisationStrategy

Стратегия является уровнем **выше конкретной гаммы**.

Data model уже допускает стратегии вроде:

- chord-tone based;
- guide-tone resolution;
- diatonic color;
- lydian dominant;
- melodic-minor application;
- altered dominant;
- diminished dominant;
- tritone superimposition;
- chromatic enclosure;
- side-slip outside.

Stage 3 будет определять, какие из них уместны в конкретном `HarmonicSituation`.

## Phrase

`Phrase` с самого начала хранится семантически, а не только как абсолютный MIDI.

Минимальный контракт `0.0b`:

- id/name;
- harmonic pattern;
- tension level;
- phrase role;
- start/target degree;
- relative phrase notes;
- harmonic slot каждой ноты;
- chromatic offset;
- rhythm position/duration;
- target marker.

Source/license, расширенные tags, fingerings и library persistence будут добавляться в Stage 5 без изменения основного принципа relative representation.

## Что сознательно НЕ делает 0.0b

- не распознаёт ii–V–I;
- не выводит local key center;
- не выбирает tension notes;
- не выбирает improvisation strategies;
- не ищет phrases;
- не содержит JUCE/ARA/DAW types;
- не содержит Voicing-specific logic.

## Эталонный test case

```text
Global key: C major
Previous: Dm7
Current: G7
Next: Cmaj7
```

Ожидается:

```text
current function = Dominant / V
resolution = confirmed Cmaj7
structural moves = B→C, F→E
pattern = none (до Pattern Recognizer)
local key = undefined (до Local Key Analyzer)
confidence = confirmed благодаря explicit context + real resolution
```

Этот кейс проверяет границу между **data model** и будущим **analysis engine**.
