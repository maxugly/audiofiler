import sys

def main():
    file_path = "Source/WaveformRenderer.cpp"
    with open(file_path, "r") as f:
        content = f.read()

    # Remove include
    content = content.replace('#include "PlaybackCursorGlow.h"\n', "")

    # Replace method body
    old_method = """void WaveformRenderer::drawPlaybackCursor(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const
{
    const auto waveformBounds = controlPanel.getWaveformBounds();
    PlaybackCursorGlow::renderGlow(g, controlPanel, waveformBounds);
}"""

    new_method = """void WaveformRenderer::drawPlaybackCursor(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const
{
    const auto waveformBounds = controlPanel.getWaveformBounds();
    const float drawPosition = (float)audioPlayer.getTransportSource().getCurrentPosition();
    const float x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();

    drawGlowingLine(g, (int)x, waveformBounds.getY(), waveformBounds.getBottom(), Config::Colors::playbackText);
}"""

    if old_method not in content:
        print("Error: Could not find the old method body.")
        sys.exit(1)

    content = content.replace(old_method, new_method)

    with open(file_path, "w") as f:
        f.write(content)

if __name__ == "__main__":
    main()
