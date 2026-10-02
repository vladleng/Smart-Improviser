#pragma once
#include "ara/ARAPluginProcessor.h"
#include "core/analysis/LibrarySearch.h"
#include <array>
#include <charconv>
#include <cmath>
class LibrarySearchView final : public juce::Component,private juce::ListBoxModel
{
    using Query=smartimproviser::harmony::LibrarySearchQuery;
    using State=smartimproviser::harmony::LibrarySearchState;
    static juce::String tr(const char* s){return juce::String::fromUTF8(s);}
    static juce::String utf(const std::string& s){return juce::String::fromUTF8(s.c_str());}
    static const char* stateName(State state){
        switch(state){
        case State::eligible:return "Подходит";
        case State::draft:return "Набросок";
        case State::harmonicMismatch:return "Гармония не совпадает";
        case State::tensionMismatch:return "Tension не совпадает";
        case State::tensionUnknown:return "Tension не назначена";
        case State::insufficientContext:return "Не хватает контекста";
        case State::insufficientMetadata:return "Не хватает метаданных";
        case State::metadataMismatch:return "Фильтр не совпадает";
        default:return "Некорректные данные";
        }
    }
public:
    LibrarySearchView(SmartImproviserARAProcessor& p,std::function<Query()> provider)
        :processor(p),contextProvider(std::move(provider)) {
        for(auto* field:{&text,&tag,&conceptRuleId,&position,&curve})addAndMakeVisible(*field);
        text.setTextToShowWhenEmpty(tr("Текст (с учётом регистра)"),juce::Colours::grey);
        tag.setTextToShowWhenEmpty(tr("Точный тег"),juce::Colours::grey);
        conceptRuleId.setTextToShowWhenEmpty(tr("Concept ID"),juce::Colours::grey);
        position.setTextToShowWhenEmpty(tr("Позиция в обороте (от 1)"),juce::Colours::grey);
        curve.setTextToShowWhenEmpty(tr("Кривая: начало,конец,T; … (абсолютные beats песни, T=1/2/3)"),juce::Colours::grey);
        for(auto* box:{&tension,&role,&pattern,&function})addAndMakeVisible(*box);
        tension.addItem(tr("Все tensions"),1);tension.addItem("T1",2);tension.addItem("T2",3);tension.addItem("T3",4);
        tension.setSelectedId(processor.tensionFilter()+1,juce::dontSendNotification);
        role.addItem(tr("Любая роль"),1);
        const std::array<const char*,6> roles{"Высказывание","Развитие","Подготовка","Кульминация","Разрешение","Спад"};
        for(int i=0;i<6;++i)role.addItem(tr(roles[static_cast<std::size_t>(i)]),i+2);
        role.setSelectedId(1,juce::dontSendNotification);
        pattern.addItem(tr("Любой оборот"),1);
        const std::array<const char*,16> patterns{"ii–V–I","iiø–V–i","iv–V–i","V–I","I–VI–ii–V","iii–vi–ii–V",
            "Кадансовая цепочка","Побочная доминанта","Тритоновая замена","Backdoor","iv–I","Проходящий dim",
            "Общий тон dim","Цепочка доминант","Модальный vamp","iiø–V–I"};
        for(int i=0;i<16;++i)pattern.addItem(tr(patterns[static_cast<std::size_t>(i)]),i+2);
        pattern.setSelectedId(1,juce::dontSendNotification);
        function.addItem(tr("Любая функция"),1);
        for(const auto* name:{"Тоника","Субдоминанта","Доминанта","Замещающая доминанта","Другая"})
            function.addItem(tr(name),function.getNumItems()+1);
        function.setSelectedId(1,juce::dontSendNotification);
        run.setButtonText(tr("Подобрать по текущему контексту"));addAndMakeVisible(run);
        run.onClick=[this]{search();};
        list.setModel(this);list.setRowHeight(38);list.setColour(juce::ListBox::backgroundColourId,juce::Colour::fromRGB(42,46,53));addAndMakeVisible(list);
        details.setReadOnly(true);details.setMultiLine(true);details.setScrollbarsShown(true);addAndMakeVisible(details);
        summary.setJustificationType(juce::Justification::topLeft);summary.setColour(juce::Label::textColourId,juce::Colours::orange);addAndMakeVisible(summary);
        setSize(1000,720);search();
    }
    void paint(juce::Graphics& g)override{g.fillAll(juce::Colour::fromRGB(35,39,46));}
    void resized()override {
        auto a=getLocalBounds().reduced(16);auto first=a.removeFromTop(32);
        text.setBounds(first.removeFromLeft(220));first.removeFromLeft(8);tag.setBounds(first.removeFromLeft(150));
        first.removeFromLeft(8);conceptRuleId.setBounds(first.removeFromLeft(230));first.removeFromLeft(8);tension.setBounds(first);
        a.removeFromTop(8);auto second=a.removeFromTop(32);
        role.setBounds(second.removeFromLeft(180));second.removeFromLeft(8);pattern.setBounds(second.removeFromLeft(200));
        second.removeFromLeft(8);position.setBounds(second.removeFromLeft(230));second.removeFromLeft(8);function.setBounds(second);
        a.removeFromTop(8);curve.setBounds(a.removeFromTop(32));a.removeFromTop(8);
        run.setBounds(a.removeFromTop(34));a.removeFromTop(8);summary.setBounds(a.removeFromTop(90));a.removeFromTop(8);
        list.setBounds(a.removeFromLeft(350));a.removeFromLeft(12);details.setBounds(a);
    }
private:
    int getNumRows()override{return static_cast<int>(result.entries.size());}
    void paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool selected)override{
        if(row<0 || row>=getNumRows())return;
        const auto& entry=result.entries[static_cast<std::size_t>(row)];
        if(selected)g.fillAll(juce::Colour::fromRGB(65,79,96));
        g.setColour(entry.eligible()?juce::Colours::lightgreen:juce::Colours::orange);
        g.drawText((entry.record.name.empty()?tr("(Без названия)"):utf(entry.record.name))+" · "+tr(stateName(entry.state)),8,0,width-16,height,juce::Justification::centredLeft,true);
    }
    void selectedRowsChanged(int row)override{
        using namespace smartimproviser::harmony;
        if(row<0 || row>=getNumRows()){details.clear();return;}
        const auto& entry=result.entries[static_cast<std::size_t>(row)];
        juce::String display=utf(entry.record.name)+"\n"+tr(stateName(entry.state))+"\n\n";
        for(const auto& why:entry.explanations)display+=utf(why)+"\n";
        display+="\n"+utf(entry.record.explanation)+"\n";
        if(const auto* idea=std::get_if<Idea>(&entry.record.content))display+=tr("\nТекст идеи: ")+utf(idea->text);
        else {
            const auto& phrase=std::get<Phrase>(entry.record.content);
            display+=tr("\nСохранённые ноты (не транспонируются поиском):\n");
            for(const auto& note:phrase.notes)display+=tr("Ступень ")+juce::String::repeatedString(note.pitch.chromaticOffset<0?"b":"#",std::abs(note.pitch.chromaticOffset))
                +juce::String(note.pitch.degree)+tr(" · аккорд ")+juce::String(static_cast<juce::int64>(note.pitch.chordIndex)+1)
                +tr(" · начало ")+juce::String(note.beatOffset)+tr(" · длина ")+juce::String(note.durationBeats)
                +(note.target?tr(" · цель"):juce::String{})+"\n";
            for(std::size_t i=0;i<entry.assessment.contexts.size();++i){
                const auto& context=entry.assessment.contexts[i];
                if(!context.actualChord.valid)continue;
                display+=tr("\nАккорд ")+juce::String(static_cast<int>(i)+1)+": "+utf(normalizedChordSymbol(context.actualChord));
                if(context.actualNextChord.valid)display+=tr("\nФактический следующий: ")+utf(normalizedChordSymbol(context.actualNextChord));
                if(context.destination.available)display+=context.destination.confirmed?tr("\nЦель разрешения подтверждена."):tr("\nЦель разрешения предполагается; фактический следующий аккорд остаётся отдельным.");
                if(!context.sourceRuleId.empty())display+=tr("\nИсточник: ")+utf(context.sourceRuleId)+tr(" · версия ")+juce::String(context.sourceRuleVersion)
                    +tr(" · трактовок: ")+juce::String(static_cast<int>(context.sourceEvidenceAlternatives.size()));
            }
        }
        details.setText(display,false);details.moveCaretToTop(false);
    }
    void search(){
        using namespace smartimproviser::harmony;
        const auto reject=[this](const juce::String& message){result={};list.updateContent();details.clear();summary.setText(message,juce::dontSendNotification);};
        processor.reloadUserLibrary(); // Explicit search refresh, never timer/audio polling.
        auto q=contextProvider();q.match.requestedTension=requestedPhraseTension(tension.getSelectedId()-1);
        q.text=text.getText().toStdString();q.tag=tag.getText().trim().toStdString();q.conceptRuleId=conceptRuleId.getText().trim().toStdString();
        if(role.getSelectedId()>1)q.role=static_cast<PhraseRole>(role.getSelectedId()-1);
        if(pattern.getSelectedId()>1)q.pattern=static_cast<HarmonicPatternType>(pattern.getSelectedId());
        if(function.getSelectedId()>1)q.function=static_cast<HarmonicFunction>(function.getSelectedId()-1);
        if(position.getText().trim().isNotEmpty()){
            auto value=position.getText().trim().toStdString();int n=0;auto parsed=std::from_chars(value.data(),value.data()+value.size(),n);
            if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size() || n<1){reject(tr("Позиция: целое число от 1."));return;}
            q.patternPosition=n-1;
        }
        if(curve.getText().trim().isNotEmpty()){
            TensionCurve desired;
            for(auto row:juce::StringArray::fromTokens(curve.getText(),";","")){
                auto values=juce::StringArray::fromTokens(row,",","");
                if(values.size()!=3){reject(tr("Кривая: начало,конец,T; …"));return;}
                std::array<double,3> numbers{};
                for(int i=0;i<3;++i){
                    auto value=values[i].trim().toStdString();auto parsed=std::from_chars(value.data(),value.data()+value.size(),numbers[static_cast<std::size_t>(i)]);
                    if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size() || !std::isfinite(numbers[static_cast<std::size_t>(i)])){
                        reject(tr("Кривая содержит неверное число."));return;}
                }
                if(numbers[2]<1 || numbers[2]>3 || std::floor(numbers[2])!=numbers[2]){reject(tr("Уровень кривой: 1, 2 или 3."));return;}
                desired.spans.push_back({numbers[0],numbers[1],static_cast<TensionLevel>(static_cast<int>(numbers[2]))});
            }
            if(!validateTensionCurve(desired).valid){reject(tr("Кривая содержит неверные границы или пересечения."));return;}
            q.curve=desired;
        }
        auto records=processor.libraryRecords(LibraryDomain::common);auto personal=processor.libraryRecords(LibraryDomain::user);
        records.insert(records.end(),personal.begin(),personal.end());result=searchLibrary(records,q);
        auto message=tr("Подходит: ")+juce::String(static_cast<int>(result.eligibleCount))+" / "+juce::String(getNumRows())+"\n"+utf(result.explanation);
        if(!q.match.slots.empty())message+=tr("\nСнимок: ")+utf(normalizedChordSymbol(q.match.slots.front().material.context.currentChord))
            +tr(" · beats песни ")+juce::String(q.placementStartBeat)+tr(". После смены аккорда нажмите «Подобрать».");
        else message+=tr("\nНет известных границ текущего аккорда. Установите курсор перед следующим аккордом.");
        if(processor.userLibraryStorageStatus()!=LibraryStorageStatus::success)message+=tr("\nЕсть ошибка чтения библиотеки; откройте «Библиотека» и проверьте статус.");
        summary.setText(message,juce::dontSendNotification);list.updateContent();if(getNumRows()>0){list.selectRow(0);selectedRowsChanged(0);}else details.clear();
    }
    SmartImproviserARAProcessor& processor;
    std::function<Query()> contextProvider;
    smartimproviser::harmony::LibrarySearchResult result;
    juce::TextEditor text,tag,conceptRuleId,position,curve,details;
    juce::ComboBox tension,role,pattern,function;
    juce::TextButton run;
    juce::Label summary;
    juce::ListBox list;
};
class LibrarySearchWindow final : public juce::DocumentWindow
{
public:
    LibrarySearchWindow(SmartImproviserARAProcessor& p,std::function<smartimproviser::harmony::LibrarySearchQuery()> provider)
        :juce::DocumentWindow(juce::String::fromUTF8("Подбор фраз · Smart Improviser"),juce::Colour::fromRGB(35,39,46),juce::DocumentWindow::closeButton){
        setUsingNativeTitleBar(true);setContentOwned(new LibrarySearchView(p,std::move(provider)),true);
        setResizable(true,false);setResizeLimits(1000,720,1700,1200);centreWithSize(1000,720);setVisible(true);
    }
    void closeButtonPressed()override{setVisible(false);}
};
