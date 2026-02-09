#include "PlaybackTextPresenter.h"

#include "ControlPanel.h"
#include "Config.h"
#include "AudioPlayer.h"

PlaybackTextPresenter::PlaybackTextPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void PlaybackTextPresenter::render(juce::Graphics& g) const
{
    const int textY = owner.getBottomRowTopY() - Config::playbackTimeTextOffsetY;
    auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

    g.setColour(Config::playbackTextColor);
    g.setFont(Config::playbackTextSize);
    g.drawText(buildCurrentTime(), leftX, textY, Config::playbackTextWidth, Config::playbackTextHeight, juce::Justification::left, false);
    g.drawText(getTotalTimeStaticString(), centreX, textY, Config::playbackTextWidth, 20, juce::Justification::centred, false);
    g.drawText(buildRemainingTime(), rightX, textY, Config::playbackTextWidth, 20, juce::Justification::right, false);
}

juce::String PlaybackTextPresenter::buildCurrentTime() const
{
    return owner.formatTime(owner.getAudioPlayer().getTransportSource().getCurrentPosition());
}

juce::String PlaybackTextPresenter::buildRemainingTime() const
{
    const auto total = owner.getAudioPlayer().getThumbnail().getTotalLength();
    const auto remaining = juce::jmax(0.0, total - owner.getAudioPlayer().getTransportSource().getCurrentPosition());
    return "-" + owner.formatTime(remaining);
}
