# Stage 3 / 0.3a — live test

Статус: код подготовлен; принятие Владом ещё не выполнено. Последняя принятая stable — 0.3.

## Что изменилось

Host-neutral Improvisation Engine получает HarmonicSituation Stage 2. Первая стратегия использует реальные звуки текущего аккорда, сохраняет контекст/неоднозначность и ResolutionTarget. Source, thinking structure, target, ID/version правила и priority разделены. Tension ещё не классифицируется: Tension Engine — Stage 4. Гаммы и модели Бойко добавляются в следующих checkpoints.

Справа в диагностическом UI появилась текстовая карточка. Анализ и её форматирование выполняются на UI timer, вне paint и audio callback. Список материала пока показывает pitch classes; полное функциональное enharmonic spelling — позднейший checkpoint.

## Проверка в Fender Studio Pro

Установить artifact `Smart-Improviser-0.3a-Windows`, заменив `Smart Improviser.vst3`. Заголовок: 0.3a.

- [ ] C major: Dm7 → G7 → Cmaj7. На G7: `dominant to major`, материал G B D F, target Cmaj7.
- [ ] В том же C major заменить цель на Cm: `dominant to minor`, target Cm. Глобальная тональность остаётся C major.
- [ ] A7 → Dm7: цель minor, secondary относительно глобального C major; локальная трактовка Stage 2 сохраняется.
- [ ] Db7 → Cmaj7: SubV, major target; материал содержит реальные pitch classes Db F Ab B (B здесь enharmonic Cb).
- [ ] G7 → C7: dominant chain, не major tonic.
- [ ] G7 → Abmaj7 или G7 без следующего аккорда: resolution unconfirmed; ложной Cmaj7 цели нет.
- [ ] G7sus4: нет добавленной B; C triad: нет добавленной B/b7; G7b9: Ab не исчезает.
- [ ] PLAY / STOP / seek и редактирование Chord Track обновляют карточку вместе с Stage 2.
- [ ] При отсутствии валидного key/chord/position карточка показывает ожидание контекста; старые рекомендации не остаются.
- [ ] Reopen проекта восстанавливает анализ; поля Stage 1–2 работают как прежде.
- [ ] Правая карточка читаема и не перекрывает диагностику.

Автоматически: 8 C++ test targets, включая major/minor/chain/secondary/SubV, unresolved primary, missing data, sus/triad/alteration, детерминированность и 12 тональностей. Локально проверены через g++; Windows VST3 проверяется CI. Live acceptance не заменяется автоматическими тестами. После принятия — 0.3b.
