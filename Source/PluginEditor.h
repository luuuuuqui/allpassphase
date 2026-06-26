#pragma once

#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>

class AllPassPhaseAudioProcessor;

class AllPassPhaseAudioProcessorEditor final : public juce::AudioProcessorEditor {
  public:
	explicit AllPassPhaseAudioProcessorEditor(AllPassPhaseAudioProcessor& processor);
	~AllPassPhaseAudioProcessorEditor() override = default;

	void resized() override;

  private:
	using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

	void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);

	AllPassPhaseAudioProcessor& audioProcessor;

	juce::Slider frequencySlider;
	juce::Slider qSlider;
	juce::Slider iterationsSlider;
	juce::Slider mixSlider;

	juce::Label frequencyLabel;
	juce::Label qLabel;
	juce::Label iterationsLabel;
	juce::Label mixLabel;

	std::unique_ptr<SliderAttachment> frequencyAttachment;
	std::unique_ptr<SliderAttachment> qAttachment;
	std::unique_ptr<SliderAttachment> iterationsAttachment;
	std::unique_ptr<SliderAttachment> mixAttachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AllPassPhaseAudioProcessorEditor)
};
