# Stage 3 / 0.3e — Harmonic concepts

База: принятый 0.3d fix2, Windows Build #246 success, PR #26 merged.
0.3e реализован; живой тест ожидается. Stable остаётся 0.3.

## Объём и граница с Phrase

Checkpoint добавляет текстовые гармонические идеи поверх уже проверенных Strategy. В этом checkpoint chromatic approach/enclosure — символические схемы интервалов относительно цели, а не короткие сгенерированные фразы. Нет ритма, длительностей, октав, аппликатуры, MIDI, воспроизведения или записи пользовательских нот. Выбор формы нотных примеров и генерации Phrase остаётся отдельным будущим решением.

| Идея | Материал / поведение |
|---|---|
| Chord-tone playing | Реальные звуки текущего аккорда с написанием и ступенями; не добавляет отсутствующие 3/7 |
| Guide-tone targeting | Только имеющиеся guides; использует confirmed Stage 2 moves либо отдельно помеченные optional melodic connections |
| Diatonic colors | 9/11/13 из уже доступного диатонического источника; natural 11 против major 3 и b13 против natural 5 помечены passing |
| Think m6 over chord | Четыре звука уже доступной m6 thinking structure; интервалы показываются относительно реального аккорда, полный scale source сохраняется |
| Chromatic approach | Ниже цели на полутон → цель; задана реальная целевая нота |
| Chromatic enclosure | Выше на полутон → ниже на полутон → цель; это одна начальная схема, не полный каталог enclosure |

Approach/enclosure — продуктовые шаблоны, не транскрипции фраз Бойко. m6 — продуктовая инструкция, привязанная к принятому каталогу источников раздела 2. Новые заменяющие гаммы, Fm6 над G7 и outside-системы не добавляются.

## Контекст и роли

- Concepts хранят rule ID/version, sourceRuleId, interpretation/evidence и actualChord. Списки Strategy и их приоритеты не меняются.
- MaterialNote в concept.material относится к actualChord. Target относится к отдельному targetChord; targetScope явно currentChord/nextChord.
- Цель approach: сначала присутствующая терция следующего аккорда, затем другая guide, затем присутствующий корень. Если следующего аккорда нет, тот же выбор делается внутри текущего. Отсутствующий target не выдумывается.
- Подготовительные ступени оцениваются относительно текущего реально звучащего аккорда. Membership — не оценка устойчивости и не Tension Level.
- При next target инструкция предлагает подготовку на текущей гармонии и попадание в цель при смене аккорда. Точного момента начала/длины фразы в данных нет.
- Primary unresolved сохраняется. Идеи по explicit chord/next notes доступны без выбора тонального центра; диатонические/m6 идеи появляются только вместе с соответствующими уже разрешёнными источниками.
- Fix2 сохраняется: host C#7 перед C major/minor при confirmed SubV даёт Db7/Abm6 в новых идеях; raw Stage 1/2 может оставаться C#7.
- Функция, глобальный/локальный центр, confirmed resolution и пользовательские ноты не меняются.
- По-прежнему нет автоматической классификации T1–T3. Сравнение global/local alternatives и ranking — 0.3f.

## Проверка в Fender Studio Pro

Artifact `Smart-Improviser-0.3e-Windows`, установленная папка `Smart Improviser.vst3`, заголовок 0.3e. Новый блок **HARMONIC IDEAS** стоит перед **SCALE SOURCES** в прокручиваемой правой панели.

| Сценарий | Ожидается |
|---|---|
| C major / Dm7 → G7 → Cmaj7, на G7 | Chord-tone playing: G B D F; guides B/F, confirmed B→C и F→E |
| Тот же G7 | Diatonic colors: A (9), C (11) [passing], E (13) |
| Тот же G7 | Think Dm6 over G7: D (5), F (b7), A (9), B (3); Think Abm6 over G7: Ab (b9), B (3), Eb (b13), F (b7) |
| Тот же G7 | Approach/enclosure target E (3) in Cmaj7 [next chord]; схемы -1→0 и +1→-1→0; верхний подготовительный звук принадлежит G7, нижний — нет |
| G7 → Cm | Цель становится Eb (b3) in Cm; -1 подготовка теперь принадлежит G7 (высота D), +1 — нет (высота E). Подтверждённое F→Eb сохранено |
| Cmaj7 → Dm7 → G7, на Dm7 | Guide connections к G7 помечены optional, а не confirmed |
| Db7 → Cmaj7 / Cm | Новая идея Think Abm6 over Db7; корректное функциональное написание, major/minor targets различаются |
| G7sus4 → Cmaj7 | В guide-идее только F (b7), отсутствующая B не добавляется |
| C major / C трезвучие, без next | Guide только E; approach target E [current chord]; септима не добавляется |
| C5 без next | Нет guide-идеи, подход направлен к C [current chord] |
| C major / Dm7b5 → G7 при UNRESOLVED | Explicit-note идеи допустимы; неподтверждённые scale/m6 идеи не появляются |
| C# major / C#maj7 | Новые аккордовые опоры пишутся C# E# G# B# |

Последние два столбца диагностики ниже источников могут пока сохранять pitch-class запись старых checkpoints; новые harmonic ideas используют ступенное написание.

- [ ] PLAY/STOP/seek, изменения аккорда и цели major/minor обновляют идеи вместе с источниками.
- [ ] После удаления валидного контекста старые идеи исчезают; reopen восстанавливает результат.
- [ ] Прокрутка позволяет прочитать все идеи, источники и targets; неизменный контекст не сбрасывает позицию.
- [ ] Регрессии принятого 0.3d fix2 не обнаружены.

## Автоматическая проверка

11 C++ test targets проходят локально через g++. Новый набор проверяет реальные major/minor targets, guide/optional semantics, m6 skeleton и source связь, passing colors, sus/triad/power, unresolved primary, написание SubV fix2/C#/Gb, 12 тональностей, оценку подготовки на текущем аккорде, детерминированность и очистку invalid context.

Windows VST3/UI — CI и живой тест. После принятия 0.3e — 0.3f.
