# Stage 3 / 0.3g — Explanation / usable output

## Цель

После принятого `0.3f fix4` Harmony/Improvisation Core уже знает текущий аккорд, global/local/modal interpretations, patterns, targets/resolution, rootless/implied reading и incomplete cadence. `0.3g` не добавляет новую гармоническую теорию: он превращает уже рассчитанные данные в короткую практическую цепочку для музыканта.

```text
идея → source → важные ноты → target/resolution → почему
```

## Архитектурное правило

Explanation строится **только из готового `ImprovisationResult` / `HarmonicSituation`**. Никакого повторного анализа harmony/history в UI.

Core предоставляет host-neutral presentation model. UI отвечает только за локализацию, layout и цвет.

## Core contract

Первый слой 0.3g — `ExplanationResult` / `ExplanationItem` / `ExplanationEvidenceItem`.

Для каждого визуально уникального музыкального материала хранятся:

- исходные strategy indices;
- provenance по interpretation indices;
- idea;
- source material;
- actual chord / thinking structure;
- важные ноты (guide + characteristic, fallback — source notes);
- next target / target notes;
- confirmed resolution;
- Why?-evidence.

Визуально одинаковый material из нескольких interpretations схлопывается в один item, но `interpretationIndices` сохраняются.

## Why?-evidence

Поддерживаемые причины:

- top-level `PatternContext`;
- nested pattern;
- confirmed `ResolutionTarget`;
- `ImpliedDominantReading`;
- `IncompleteCadence` / missing tonic;
- фактическое продолжение, противоречащее ожидаемой tonic;
- unresolved multiple interpretations.

Presentation-state:

- `confirmed` — подтверждённый pattern/resolution;
- `missing` — отсутствующий member полного шаблона;
- `implied` — подразумеваемая функция/root или provisional evidence;
- `contradicted` — известный факт противоречит гипотезе;
- `ambiguous` — несколько допустимых interpretations без hidden winner.

Цветовой контракт UI остаётся принятым:

- серый = missing / implicit;
- янтарный = implied / provisional;
- красный = contradicted.

## Presentation

В карточке «Текущий контекст» уровни должны быть названы явно:

- глобальная функция;
- локальный центр;
- локальная функция — только когда она реально рассчитана;
- modal interpretation — как отдельная трактовка.

Это закрывает известный кейс `Gm7`: global function в C и local F-major center не должны выглядеть одной и той же функцией.

Раздел «Материал» должен показывать компактный пользовательский ответ, а подробные исходники/ноты и diagnostics остаются в существующих разделах.

## Scope 0.3g

- [x] Создать host-neutral Explanation API поверх готового `ImprovisationResult`.
- [x] Схлопывать одинаковый material без потери interpretation provenance.
- [x] Включить PatternContext / nested patterns / resolution / implied dominant / incomplete cadence / contradiction / ambiguity в Why?-evidence model.
- [ ] Покрыть Explanation API regression-тестами.
- [ ] Подключить готовый ExplanationResult к UI без повторного анализа.
- [ ] Сделать краткий output `идея → source → важные ноты → target/resolution → почему`.
- [ ] Явно разделить global/local/modal уровень функции в summary.
- [ ] Реализовать цвета missing/implied/contradicted в presentation.
- [ ] Обновить build label и Windows artifact на `0.3g`.
- [ ] Regression suite green.
- [ ] Live-test Studio Pro.
- [ ] Acceptance пользователя.

## Regression matrix

Минимально проверить:

1. ordinary major `ii–V–I` — pattern + confirmed resolution;
2. minor `iiø–V–i`;
3. secondary dominant / SubV;
4. unresolved ambiguity — material не дублируется, provenance сохраняется;
5. `D7/A → Abdim` — rootless/implied dominant evidence янтарного типа;
6. `Fm7 → Bb7 → Em7` — missing I отдельно от contradicted actual continuation;
7. самостоятельный `iii–vi–ii–V` не становится incomplete `ii–V–I`;
8. determinism и сохранение Stage 1/2/0.3f regressions.

## Gate

```text
Core explanation tests
        ↓
all regression targets green
        ↓
Smart-Improviser-0.3g-Windows
        ↓
Studio Pro live-test
        ↓
user acceptance
```

Tension T1/T2/T3 остаётся Stage 4 и в 0.3g автоматически не назначается.
