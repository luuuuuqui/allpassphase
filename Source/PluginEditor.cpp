#include "PluginEditor.h"

#include "PluginProcessor.h"

AllPassPhaseAudioProcessorEditor::AllPassPhaseAudioProcessorEditor(AllPassPhaseAudioProcessor& processor)
    : AudioProcessorEditor(processor), audioProcessor(processor) {
	configureSlider(frequencySlider, frequencyLabel, "Frequency");
	configureSlider(qSlider, qLabel, "Q");
	configureSlider(iterationsSlider, iterationsLabel, "Intensity");
	configureSlider(mixSlider, mixLabel, "Mix");

	iterationsSlider.setRange(0.0, 100.0, 1.0);

	frequencyAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "Frequency", frequencySlider);
	qAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "Q", qSlider);
	iterationsAttachment =
	    std::make_unique<SliderAttachment>(audioProcessor.parameters, "Iterations", iterationsSlider);
	mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "Mix", mixSlider);

	setSize(420, 212);
}

void AllPassPhaseAudioProcessorEditor::resized() {
	auto bounds = getLocalBounds().reduced(16);
	const auto rowHeight = 40;
	const auto labelWidth = 92;

	auto layoutSlider = [&](juce::Label& label, juce::Slider& slider) {
		auto row = bounds.removeFromTop(rowHeight);
		label.setBounds(row.removeFromLeft(labelWidth));
		slider.setBounds(row);
		bounds.removeFromTop(8);
	};

	layoutSlider(frequencyLabel, frequencySlider);
	layoutSlider(qLabel, qSlider);
	layoutSlider(iterationsLabel, iterationsSlider);
	layoutSlider(mixLabel, mixSlider);
}

void AllPassPhaseAudioProcessorEditor::configureSlider(juce::Slider& slider, juce::Label& label,
                                                       const juce::String& text) {
	slider.setSliderStyle(juce::Slider::LinearHorizontal);
	slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 88, 24);
	addAndMakeVisible(slider);

	label.setText(text, juce::dontSendNotification);
	label.setJustificationType(juce::Justification::centredLeft);
	addAndMakeVisible(label);
}
