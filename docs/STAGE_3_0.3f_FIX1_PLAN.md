# Stage 3 / 0.3f fix1 — Pattern continuity / hierarchical cadence context [ACCEPTED]

База: принятый `0.3f` (`PR #30`, Windows Build #284). Fix1 принят Владом 2026-09-25 после Windows Build #310 и live-test в Studio Pro. Stable остаётся `0.3`; следующий checkpoint — `0.3g — Explanation / usable output`.

## Причина fix1

Live-проверка после принятия `0.3f` показала, что на resolution chord окно Stage 1 `previous/current/next` теряло более ранний predominant. Например, в `F major`:

```text
Gm7 → C7 → Fmaj7
 ii     V      I
```

на `C7` полный `ii–V–I` уже подтверждён, но на `Fmaj7` без отдельного pattern context движок видел только `C7 → Fmaj7` и деградировал до `V–I`.

Второй обязательный кейс:

```text
Am7 → D7 → Gm7 → C7 → Fmaj7
 iii    VI7    ii     V      I
```

Музыкально это единый расширенный каданс, а не набор независимых коротких фрагментов.

## Принятое решение

Stage 1 contract остаётся неизменным:

```text
previous / current / next / global key
```

Для Stage 3 добавлен отдельный host-neutral `PatternContext / RecognizedPatternInstance`, который реконструируется детерминированно из ограниченного `PatternTimelineWindow` (до 9 chord events). Runtime-history не хранится, поэтому seek/chord edit/reopen не оставляют stale state.

### Continuity подтверждённых кадансов

Подтверждённый pattern сохраняется до resolution member:

```text
Gm7    → Major ii–V–I • 1/3
C7     → Major ii–V–I • 2/3
Fmaj7  → Major ii–V–I • 3/3
```

То же правило действует для:

```text
Dm7b5 → G7 → Cm7   = iiø–V–i • 1/3 … 3/3
Fm7   → G7 → Cm7   = iv–V–i  • 1/3 … 3/3
```

Если полного evidence нет, сохраняется safe fallback `V→I/i`; конкретный predominant не выдумывается.

### Иерархический расширенный каданс

Для:

```text
F major: Am7 → D7 → Gm7 → C7 → Fmaj7
          iii    VI7    ii     V      I
```

введён единый top-level pattern:

```text
Расширенный каданс iii–VI7–ii–V–I • 1/5 … 5/5
```

Вложенные связи сохраняются как evidence:

```text
D7 → Gm7         = V/ii → ii
Gm7 → C7 → Fmaj7 = ii–V–I
```

Они не дробят основной пользовательский каданс.

## Boundary resolver для stopped cursor

Live-test fix1 выявил отдельную техническую регрессию: на некоторых визуальных границах такта Studio Pro отдаёт transport PPQ на очень малую величину левее ARA chord-event. Поэтому в STOP плагин местами ещё показывал предыдущий аккорд, хотя playhead визуально уже стоял на следующем.

Исправление принято внутри fix1:

- один и тот же chord-index resolver используется для `HarmonicContext`, `TimelineHarmonicSnapshot` и `PatternTimelineWindow`;
- в `STOP` разрешён небольшой snap к следующему chord event на визуальной границе;
- в `PLAY` остаётся строгая реальная ARA-boundary, чтобы аккорд не переключался раньше времени;
- regression покрывает STOP-near-boundary, STOP-clearly-before и PLAY-at-same-position.

После финальной live-проверки пользователь подтвердил: технически границы снова работают корректно.

## Acceptance checklist

- [x] Host-neutral `PatternContext / RecognizedPatternInstance` реализован без arbitrary history.
- [x] `ii–V–I` сохраняет `I • 3/3`.
- [x] `iiø–V–i` сохраняет `i • 3/3`.
- [x] `iv–V–i` сохраняет `i • 3/3`.
- [x] Без полного evidence остаётся generic `V→I/i`.
- [x] `Am7–D7–Gm7–C7–Fmaj7` распознаётся как единый `iii–VI7–ii–V–I` top-level pattern.
- [x] `V/ii→ii` и `ii–V–I` сохраняются как nested evidence.
- [x] Seek/chord edits/reopen реконструируют context без stale runtime memory.
- [x] Stage 1 contract остаётся `previous/current/next`.
- [x] STOP-boundary resolver согласован между всеми timeline consumers; PLAY остаётся strict.
- [x] Regression suite зелёный в Windows Build #310.
- [x] Live-test в Studio Pro принят Владом 2026-09-25.

## Итог

`0.3f fix1` принят. `0.3f` вместе с refinement считается закрытым. Следующий checkpoint:

**`0.3g — Explanation / usable output`**.
