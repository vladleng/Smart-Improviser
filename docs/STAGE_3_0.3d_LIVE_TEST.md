# Stage 3 / 0.3d — Melodic minor / diminished sources

0.3c принят Владом 2026-09-25; Windows Build #237 success; PR #25 merged.
0.3d реализован; живой тест выявил enharmonic spelling issue для SubV. Fix1 не решил live-case: Fender Studio продолжил отдавать C#-spelling в Stage 3 при визуальном Db7 на Chord Track. Текущая проверяемая версия — 0.3d fix2. Stable остаётся 0.3.

## Каталог и границы

Основной источник — Игорь Бойко, «Мой метод», раздел 2. Номера страниц печатные.

| Rule ID | Контекст | Источник | Основание / ограничение |
|---|---|---|---|
| boyko.melodic-minor.root | minor, включая m7 | melodic minor от I | с. 88–89, 121; на m7 большая 7 — проходящая к корню, не опора; b7 аккорда остаётся в foundation |
| boyko.melodic-minor.bIII | m7b5 | melodic minor от bIII | с. 92–93, 121; относительно аккорда Locrian natural 2, не скрывать заданную b9 |
| boyko.melodic-minor.V | ordinary V7 → major | melodic minor от V | с. 96–97, 121; Lydian dominant с #11; major-only — консервативная политика проекта |
| boyko.melodic-minor.bII | ordinary V7 → major/minor | melodic minor от bII | с. 101–102, 121; altered, натуральная 5 отсутствует в линии, но остаётся в реальном аккорде |
| project.subv.melodic-minor.V | confirmed SubV7 → major/minor | melodic minor от V SubV | применение Lydian dominant к SubV — правило проекта, не буквальная формулировка автора |
| boyko.diminished.whole-half | dim7 | тон–полутон от корня | с. 112, пример 176; восемь звуков, не путать с полууменьшённым аккордом |

Правила пока требуют выбранную primary interpretation с поддерживаемым major/minor center. Dominant chains, unresolved dominant, sus и unresolved primary сохраняют прежние опоры. Это ограниченный каталог, не универсальный выбор всех возможных гамм.

Полутон–тон для доминанты не добавлен автоматически: в проверенном разделе источник тон–полутон дан для dim7. Harmonic minor, дополнительные outside-подходы и выбор T1–T3 остаются последующими расширениями.

## Совместимость и данные

- Stage 2 и global/local key не меняются при выборе source root.
- Source notes хранят ступени/написание относительно источника; chordRelativeNotes — относительно реального аккорда, в том же порядке высот.
- Например Ab melodic minor над G7 содержит Cb в исходной гамме, но B (терция) относительно G7. Эти записи не являются разными высотами.
- thinkingStructure содержит m6 от корня melodic minor; это опорное арпеджио, не замена всех семи нот четырьмя.
- Явные extensions/degrees проверяются. Поддержаны осознанные эквиваленты b5/#11 и #5/b13 на доминанте; запись явной ступени сохраняется в chordRelativeNotes.
- Два объявленных исключения: b7 аккорда m7 отсутствует в melodic-minor источнике; натуральная 5 отсутствует в altered. omittedChordTones хранит эти высоты. Foundation не меняется.
- Явные natural 9/11/13 не удаляются ради altered. Slash bass также проверяется; отсутствующий в источнике бас блокирует правило.
- sourceTransitions — необязательные короткие движения новых красок к root/guide реального next chord. Они отделены от confirmed ResolutionTarget Stage 2 и не доказывают функцию.
- Приоритеты: foundation 100, diatonic 50, special 40; это порядок каталога, не оценка tension или окончательный musical ranking.
- У всех правил tensionClassified = false. Несколько источников отображаются для проверки каталога; это не переключатель T1–T3.

## 0.3d fix1 — результат live-test

Fix1 пытался восстановить enharmonic spelling из `ARA SheetChord name`. Автоматические тесты этого пути проходят, но live-test показал, что Studio Pro в данном сценарии не отдаёт пригодное исходное `Db`-написание через это поле: справа по-прежнему было `C#7 → G# melodic minor`.

Поэтому fix1 не принимается как решение пользовательского кейса.

## 0.3d fix2 — functional SubV spelling

Fix2 не зависит от текста, возвращаемого host. Когда Harmonic Engine уже подтвердил SubV и реальную цель, написание выводится из функции:

- подтверждённый SubV = `bII7` относительно реального target;
- для target C: SubV функционально пишется `Db7`, независимо от того, пришёл ли structured root как C#;
- melodic-minor source для этого SubV становится `Ab melodic minor / Abm6`;
- `G` пишется как `#11` относительно Db;
- sounding pitch classes, Stage 2 function и tonal center не меняются.

Это исправление находится в Improvisation Engine, а не в Stage 1 mapper: Stage 1 не должен сам угадывать гармоническую функцию. Левая Stage 1/2 диагностическая строка `Current chord` может по-прежнему показывать host-canonicalized `C#7`; проверяемый продуктовый вывод Stage 3 справа должен использовать функциональное `Db7`.

## Проверка в Fender Studio Pro

Artifact `Smart-Improviser-0.3d-fix2-Windows`; установленная папка `Smart Improviser.vst3`; заголовок `0.3d fix2`.

Проект C major, если не указано другое:

| Сценарий | Ожидаемый результат |
|---|---|
| Cmaj7 → Dm7 → G7, на Dm7 | D Dorian сохранён; дополнительно D melodic minor / Dm6; C# — проходящая большая 7 против C в m7 |
| Dm7 → G7 → Cmaj7, на G7 | G Mixolydian; D melodic minor / Dm6 (#11 C#); Ab melodic minor / Abm6 (altered) |
| Dm7b5 → G7 → Cm, на G7 | Ab melodic minor; цель Cm с Eb, B→C и F→Eb сохранены; нет automatic Mixolydian |
| Тот же оборот, на Dm7b5 | При выбранной C minor interpretation: F melodic minor / Fm6; natural 9 E относительно D; опоры D F Ab C сохранены |
| G7b9 → Cmaj7 | Ab melodic minor; explicit Ab в аккорде сохранён; несовместимого Mixolydian нет |
| G9 → Cm / G13 → Cm | Нет altered, теряющего явную natural 9/13; опоры и minor target остаются |
| Db7 → Cmaj7, затем Cm | Справа Stage 3: Db7; Ab melodic minor / Abm6; G = #11 относительно Db; target меняется major/minor |
| Gdim7 в C major при выбранной трактовке | G whole-half diminished: G A Bb C Db Eb Fb F#; восемь звуков, Fb = уменьшённая септима |
| G7 → C7 / G7 без следующего аккорда | Нет автоматического special source |
| G7sus4 → Cmaj7 | Нет добавленной B через melodic-minor правило |

- [ ] Исходная гамма и строка On chord показывают одинаковые высоты с разными функциями/написанием.
- [ ] Правую панель можно прокрутить до опор/targets/resolution; текст не обрезан, таймер не сбрасывает прокрутку при неизменном контексте.
- [ ] PLAY/STOP/seek, изменение текущего аккорда и major/minor цели обновляют весь список.
- [ ] При отсутствии валидного контекста старые источники исчезают; reopen восстанавливает результат.
- [ ] Предыдущие chord/guide/characteristic notes и confirmed/suggested движения работают.
- [ ] fix2: справа `Db7 → Cmaj7` показывает Db7 / Ab melodic minor / Abm6 / G как #11, без G#/E#/F##.
- [ ] fix2: справа `Db7 → Cm` сохраняет то же функциональное написание Db/Ab и корректную minor target.

Автоматически: 10 C++ test targets. SpecialSources regression теперь отдельно проверяет два входных spelling одного pitch class: явный Db (`rootFifths=-5`) и host-canonicalized C# (`rootFifths=7`) — оба обязаны дать функциональный `Db7 / Ab melodic minor` при подтверждённом разрешении в C. TimelineContextMapper regression fix1 сохранён как безопасная metadata-попытка, но больше не является единственным механизмом.

После принятия 0.3d fix2 — 0.3e. При необходимости следующий точечный fix текущей буквы.
