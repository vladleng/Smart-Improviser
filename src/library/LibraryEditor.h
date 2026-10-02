#pragma once
#include "ara/ARAPluginProcessor.h"
#include "core/analysis/IdeaNoteInput.h"
#include <array>

class LibraryEditor final : public juce::Component, private juce::ListBoxModel
{
    using Record=smartimproviser::harmony::LibraryRecord;
    using Domain=smartimproviser::harmony::LibraryDomain;
    using Idea=smartimproviser::harmony::Idea;
    using Edit=SmartImproviserARAProcessor::LibraryEdit;
    static juce::String tr(const char* s) { return juce::String::fromUTF8(s); }
    static juce::String utf(const std::string& s) { return juce::String::fromUTF8(s.c_str()); }
    struct NoteRow {
        std::array<juce::TextEditor,5> fields;
        smartimproviser::harmony::IdeaNote original;
    };
public:
    explicit LibraryEditor(SmartImproviserARAProcessor& p) : processor(p)
    {
        for(auto* button : {&fresh,&save,&copy,&variant,&discard,&reload,&addNote,&removeNote}) addAndMakeVisible(*button);
        fresh.setButtonText(tr("Новая идея")); save.setButtonText(tr("Сохранить"));
        copy.setButtonText(tr("Личная копия")); variant.setButtonText(tr("Вариант"));
        discard.setButtonText(tr("Отменить правки")); reload.setButtonText(tr("Обновить"));
        addNote.setButtonText(tr("+ Нота")); removeNote.setButtonText(tr("− Последняя"));
        domain.addItem(tr("Личная библиотека"),1); domain.addItem(tr("Общая библиотека"),2);
        domain.setSelectedId(1,juce::dontSendNotification); addAndMakeVisible(domain);
        domain.onChange=[this] {
            if(dirty) { domain.setSelectedId(currentDomain==Domain::user?1:2,juce::dontSendNotification); blocked(); return; }
            currentDomain=domain.getSelectedId()==1?Domain::user:Domain::common; refresh();
        };
        list.setModel(this); list.setRowHeight(34); addAndMakeVisible(list);
        for(auto* editor : {&name,&text,&rhythm,&harmony,&explanation}) {
            addAndMakeVisible(*editor);
            editor->onTextChange=[this]{ if(!loading) {dirty=true; status.setText(tr("Есть несохранённые изменения."),juce::dontSendNotification);} };
        }
        name.setTextToShowWhenEmpty(tr("Название (необязательно)"),juce::Colours::grey);
        text.setMultiLine(true); text.setReturnKeyStartsNewLine(true);
        text.setTextToShowWhenEmpty(tr("Текст идеи / concept"),juce::Colours::grey);
        rhythm.setTextToShowWhenEmpty(tr("Заметки о ритме"),juce::Colours::grey);
        harmony.setTextToShowWhenEmpty(tr("Заметки о гармонии"),juce::Colours::grey);
        explanation.setTextToShowWhenEmpty(tr("Пояснение"),juce::Colours::grey);
        status.setColour(juce::Label::textColourId,juce::Colours::orange);
        status.setJustificationType(juce::Justification::topLeft); addAndMakeVisible(status);
        info.setJustificationType(juce::Justification::topLeft); addAndMakeVisible(info);
        headings.setText(tr("Ступень (b9/#11)     Октава ±     Начало, beats     Длина, beats     Slot (от 0)"),juce::dontSendNotification);
        addAndMakeVisible(headings);
        viewport.setViewedComponent(&notes,false); addAndMakeVisible(viewport);
        fresh.onClick=[this]{ if(dirty){blocked();return;} currentDomain=Domain::user; domain.setSelectedId(1,juce::dontSendNotification);
            draft=Record{}; list.deselectAllRows(); showDraft(); };
        discard.onClick=[this]{ draft=baseline; showDraft(); };
        reload.onClick=[this]{ if(dirty){blocked();return;} auto result=processor.reloadUserLibrary();
            if(result.succeeded()) refresh(); else report(result); };
        save.onClick=[this]{ saveDraft(); };
        copy.onClick=[this]{ derive(Edit::copy); };
        variant.onClick=[this]{ derive(Edit::variant); };
        addNote.onClick=[this]{ if(!editableIdea())return; addRow({}); dirty=true; layoutNotes(); };
        removeNote.onClick=[this]{ if(!editableIdea() || rows.empty())return; rows.pop_back(); dirty=true; layoutNotes(); };
        const auto result=processor.reloadUserLibrary();
        refresh();
        if(!result.succeeded()) report(result);
        setSize(960,720);
    }
    void resized() override
    {
        auto area=getLocalBounds().reduced(16);
        auto top=area.removeFromTop(34);
        domain.setBounds(top.removeFromLeft(250)); top.removeFromLeft(8);
        fresh.setBounds(top.removeFromLeft(135)); top.removeFromLeft(8);
        reload.setBounds(top.removeFromLeft(115)); top.removeFromLeft(8);
        discard.setBounds(top);
        area.removeFromTop(12);
        status.setBounds(area.removeFromBottom(64));
        auto actions=area.removeFromBottom(36);
        save.setBounds(actions.removeFromLeft(160)); actions.removeFromLeft(8);
        copy.setBounds(actions.removeFromLeft(160)); actions.removeFromLeft(8);
        variant.setBounds(actions.removeFromLeft(160));
        area.removeFromBottom(10);
        list.setBounds(area.removeFromLeft(250)); area.removeFromLeft(16);
        name.setBounds(area.removeFromTop(30)); area.removeFromTop(8);
        text.setBounds(area.removeFromTop(105)); area.removeFromTop(8);
        rhythm.setBounds(area.removeFromTop(30)); area.removeFromTop(8);
        harmony.setBounds(area.removeFromTop(30)); area.removeFromTop(8);
        explanation.setBounds(area.removeFromTop(30)); area.removeFromTop(8);
        info.setBounds(area.removeFromTop(70));
        auto buttons=area.removeFromTop(30); addNote.setBounds(buttons.removeFromLeft(120)); buttons.removeFromLeft(8);
        removeNote.setBounds(buttons.removeFromLeft(140));
        headings.setBounds(area.removeFromTop(28)); viewport.setBounds(area);
        layoutNotes();
    }
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour::fromRGB(35,39,46));}
private:
    bool editableIdea() const {return draft.domain==Domain::user && std::holds_alternative<Idea>(draft.content);}
    void blocked() {status.setText(tr("Сначала сохраните изменения или нажмите «Отменить правки»."),juce::dontSendNotification);}
    int getNumRows() override {return static_cast<int>(records.size());}
    void paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool selected) override {
        if(row<0 || row>=getNumRows())return;
        if(selected)g.fillAll(juce::Colour::fromRGB(65,79,96));
        g.setColour(juce::Colours::white);
        const auto& record=records[static_cast<std::size_t>(row)];
        auto label=record.name.empty()?tr("(Без названия)"):utf(record.name);
        label+=std::holds_alternative<Idea>(record.content)?tr(" · идея"):tr(" · фраза");
        g.drawText(label,8,0,width-16,height,juce::Justification::centredLeft,true);
    }
    void selectedRowsChanged(int row) override {
        if(loading || row<0 || row>=getNumRows())return;
        if(dirty) {loading=true; list.deselectAllRows();
            for(int i=0;i<getNumRows();++i)if(records[static_cast<std::size_t>(i)].id==draft.id)list.selectRow(i);
            loading=false;blocked();return;}
        draft=records[static_cast<std::size_t>(row)]; showDraft();
    }
    void refresh() {
        records=processor.libraryRecords(currentDomain); loading=true;
        list.updateContent();list.deselectAllRows();loading=false;
        draft=Record{};draft.domain=currentDomain;
        if(!records.empty()){draft=records.front();loading=true;list.selectRow(0);loading=false;}
        showDraft();
        if(records.empty())status.setText(currentDomain==Domain::common
            ?tr("Общий набор появится в 0.5f. Сейчас можно сохранять свои идеи.")
            :tr("Личная библиотека пуста. Нажмите «Новая идея»."),juce::dontSendNotification);
    }
    void addRow(smartimproviser::harmony::IdeaNote note) {
        auto row=std::make_unique<NoteRow>();row->original=note;
        juce::String degree;
        if(note.pitch){degree=juce::String::repeatedString(note.pitch->chromaticOffset<0?"b":"#",std::abs(note.pitch->chromaticOffset))
            +juce::String(note.pitch->degree);}
        std::array<juce::String,5> values{degree,note.octaveOffset?juce::String(*note.octaveOffset):juce::String{},
            note.beatOffset?juce::String(*note.beatOffset,8):juce::String{},
            note.durationBeats?juce::String(*note.durationBeats,8):juce::String{},
            note.pitch && note.pitch->chordIndex>=0?juce::String(note.pitch->chordIndex):juce::String{}};
        for(std::size_t i=0;i<5;++i) {
            auto& field=row->fields[i];field.setText(values[i],false);field.setReadOnly(!editableIdea());
            field.onTextChange=[this]{if(!loading)dirty=true;};notes.addAndMakeVisible(field);
        }
        rows.push_back(std::move(row));
    }
    void layoutNotes() {
        const int width=juce::jmax(400,viewport.getWidth()-20);
        notes.setSize(width,juce::jmax(viewport.getHeight(),static_cast<int>(rows.size())*34));
        for(std::size_t i=0;i<rows.size();++i)for(int col=0;col<5;++col)
            rows[i]->fields[static_cast<std::size_t>(col)].setBounds(col*width/5,static_cast<int>(i)*34,width/5-6,28);
    }
    void showDraft() {
        loading=true;dirty=false;baseline=draft; rows.clear();
        name.setText(utf(draft.name),false);explanation.setText(utf(draft.explanation),false);
        text.clear();rhythm.clear();harmony.clear();
        if(const auto* idea=std::get_if<Idea>(&draft.content)){
            text.setText(utf(idea->text),false);rhythm.setText(utf(idea->rhythmNotes),false);harmony.setText(utf(idea->harmonyNotes),false);
            for(const auto& note:idea->notes)addRow(note);
        } else {
            const auto& phrase=std::get<smartimproviser::harmony::Phrase>(draft.content);
            for(const auto& note:phrase.notes) {
                smartimproviser::harmony::IdeaNote view;
                view.pitch=note.pitch;view.beatOffset=note.beatOffset;view.durationBeats=note.durationBeats;view.octaveOffset=note.octaveOffset;
                addRow(view);
            }
        }
        const bool user=draft.domain==Domain::user;
        name.setReadOnly(!user);explanation.setReadOnly(!user);
        for(auto* field:{&text,&rhythm,&harmony})field->setReadOnly(!editableIdea());
        save.setEnabled(user);addNote.setEnabled(editableIdea());removeNote.setEnabled(editableIdea());
        copy.setEnabled(!draft.id.empty());variant.setEnabled(!draft.id.empty());
        const auto validation=smartimproviser::harmony::validateLibraryRecord(draft);
        auto description=validation.readyForSearch?tr("Готова для поиска (гармоническое совпадение ещё не проверено).")
            :tr("Набросок / недостаточно данных для поиска. Сохранять можно.");
        if(!editableIdea())description+=tr("\nНоты Phrase доступны для просмотра; полный редактор — Stage 8.");
        if(draft.lineage)description+=tr("\nИсточник: ")+utf(draft.lineage->parent.id)+" · rev "+juce::String(static_cast<juce::int64>(draft.lineage->parent.revision));
        info.setText(description,juce::dontSendNotification);
        status.setText(tr("Пустые поля остаются неизвестными. Октава — смещение от корня slot; MIDI-регистр не угадывается."),juce::dontSendNotification);
        loading=false;layoutNotes();
    }
    void report(const smartimproviser::harmony::LibraryStorageResult& result) {
        using S=smartimproviser::harmony::LibraryStorageStatus;
        auto message=result.status==S::conflict
            ?tr("Библиотека изменена другим экземпляром. Черновик сохранён в форме. Скопируйте текст, отмените правки и обновите библиотеку.")
            :tr("Не удалось сохранить / прочитать библиотеку. Данные в форме сохранены. ");
        status.setText(message+utf(result.explanation),juce::dontSendNotification);
    }
    void saveDraft() {
        auto candidate=draft;candidate.name=name.getText().toStdString();candidate.explanation=explanation.getText().toStdString();
        if(auto* idea=std::get_if<Idea>(&candidate.content)) {
            idea->text=text.getText().toStdString();idea->rhythmNotes=rhythm.getText().toStdString();idea->harmonyNotes=harmony.getText().toStdString();
            idea->notes.clear();
            for(std::size_t i=0;i<rows.size();++i) {
                auto& row=*rows[i]; smartimproviser::harmony::IdeaNoteInput input;
                input.degree=row.fields[0].getText().trim().toStdString();input.octave=row.fields[1].getText().trim().toStdString();
                input.beat=row.fields[2].getText().trim().toStdString();input.duration=row.fields[3].getText().trim().toStdString();
                input.slot=row.fields[4].getText().trim().toStdString();
                const auto parsed=smartimproviser::harmony::parseIdeaNoteInput(input,row.original);
                if(!parsed.error.empty()){status.setText(tr("Нота ")+juce::String(static_cast<int>(i)+1)+": "+utf(parsed.error),juce::dontSendNotification);return;}
                idea->notes.push_back(parsed.note);
            }
        }
        Record saved;const auto result=processor.editLibrary(Edit::save,candidate,saved);
        if(!result.succeeded()){report(result);return;}
        draft=saved;records=processor.libraryRecords(currentDomain);list.updateContent();showDraft();
        status.setText(tr("Сохранено · rev ")+juce::String(static_cast<juce::int64>(saved.revision)),juce::dontSendNotification);
    }
    void derive(Edit edit) {
        if(dirty){blocked();return;}
        Record saved;const auto result=processor.editLibrary(edit,draft,saved);
        if(!result.succeeded()){report(result);return;}
        currentDomain=Domain::user;domain.setSelectedId(1,juce::dontSendNotification);
        records=processor.libraryRecords(currentDomain);list.updateContent();draft=saved;showDraft();
        status.setText(tr("Создана самостоятельная запись. Оригинал не изменён."),juce::dontSendNotification);
    }
    SmartImproviserARAProcessor& processor;
    Domain currentDomain=Domain::user;
    Record draft,baseline;
    std::vector<Record> records;
    bool loading=false,dirty=false;
    juce::ComboBox domain;
    juce::ListBox list;
    juce::TextButton fresh,save,copy,variant,discard,reload,addNote,removeNote;
    juce::TextEditor name,text,rhythm,harmony,explanation;
    juce::Label status,info,headings;
    juce::Component notes;
    juce::Viewport viewport;
    std::vector<std::unique_ptr<NoteRow>> rows;
};
class LibraryWindow final : public juce::DocumentWindow
{
public:
    explicit LibraryWindow(SmartImproviserARAProcessor& processor)
        : juce::DocumentWindow(juce::String::fromUTF8("Библиотека · Smart Improviser"),
            juce::Colour::fromRGB(35,39,46),juce::DocumentWindow::closeButton)
    {
        setUsingNativeTitleBar(true);setContentOwned(new LibraryEditor(processor),true);
        setResizable(true,false);setResizeLimits(960,720,1600,1100);centreWithSize(960,720);setVisible(true);
    }
    void closeButtonPressed() override {setVisible(false);} // Keep unsaved draft while main editor exists.
};
