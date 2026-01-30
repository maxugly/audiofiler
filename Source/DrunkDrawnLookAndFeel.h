#pragma once

#include <JuceHeader.h>

/**
    Custom LookAndFeel for procedurally generated "drunk drawn" buttons
    with wobbly edges and randomized colors.
*/
class DrunkDrawnLookAndFeel : public juce::LookAndFeel_V4
{
public:
    struct ColorRange
    {
        juce::Colour baseColor;
        float hueVariation = 0.1f;
        float satVariation = 0.15f;
        float brightVariation = 0.15f;

        juce::Colour getRandomColor(juce::Random& rng) const
        {
            float h = baseColor.getHue() + rng.nextFloat() * hueVariation * 2.0f - hueVariation;
            float s = juce::jlimit(0.0f, 1.0f, baseColor.getSaturation() + rng.nextFloat() * satVariation * 2.0f - satVariation);
            float b = juce::jlimit(0.0f, 1.0f, baseColor.getBrightness() + rng.nextFloat() * brightVariation * 2.0f - brightVariation);

            while (h < 0.0f) h += 1.0f;
            while (h > 1.0f) h -= 1.0f;

            return juce::Colour::fromHSV(h, s, b, baseColor.getFloatAlpha());
        }
    };

    struct WobbleSettings
    {
        float edgeWobble = 3.0f;
        float cornerRadiusVariation = 0.3f;
        float controlPointVariation = 0.4f;
        int segmentsPerSide = 8;
        float strokeWidthVariation = 0.3f;
        float rotationVariation = 2.0f;
    };

    DrunkDrawnLookAndFeel()
    {
        buttonOffColorRange = { juce::Colour(0xff444444), 0.05f, 0.1f, 0.2f };
        buttonOnColorRange = { juce::Colour(0xffff1493), 0.08f, 0.15f, 0.15f };
        buttonOutlineColorRange = { juce::Colour(0xff00ffff), 0.1f, 0.15f, 0.2f };
        textColorRange = { juce::Colours::white, 0.0f, 0.0f, 0.1f };
        textBoxColorRange = { juce::Colour(0xff2a2a2a), 0.05f, 0.1f, 0.15f };
    }

    // Setters
    void setBaseAlpha(float newAlpha) { baseAlpha = newAlpha; }
    void setButtonOffColorRange(const ColorRange& range) { buttonOffColorRange = range; }
    void setButtonOnColorRange(const ColorRange& range) { buttonOnColorRange = range; }
    void setButtonOutlineColorRange(const ColorRange& range) { buttonOutlineColorRange = range; }
    void setTextColorRange(const ColorRange& range) { textColorRange = range; }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(4.0f);

        // C++ style cast to avoid warnings
        auto buttonId = button.getName().hashCode() + static_cast<int>(reinterpret_cast<intptr_t>(&button));
        juce::Random rng(buttonId);

        const ColorRange& fillRange = button.getToggleState() ? buttonOnColorRange : buttonOffColorRange;

        juce::Colour fillColor = fillRange.getRandomColor(rng).withAlpha(baseAlpha);
        juce::Colour outlineColor = buttonOutlineColorRange.getRandomColor(rng).withAlpha(baseAlpha);

        if (shouldDrawButtonAsDown) {
            fillColor = fillColor.brighter(0.2f);
            outlineColor = outlineColor.brighter(0.2f);
        } else if (shouldDrawButtonAsHighlighted) {
            fillColor = fillColor.brighter(0.1f);
            outlineColor = outlineColor.brighter(0.1f);
        }

        juce::Path wobblePath = createWobblyRoundedRectangle(bounds, rng);

        float rotation = (rng.nextFloat() * 2.0f - 1.0f) * wobbleSettings.rotationVariation;
        wobblePath.applyTransform(juce::AffineTransform::rotation(
            rotation * juce::MathConstants<float>::pi / 180.0f, bounds.getCentreX(), bounds.getCentreY()));

        g.setColour(fillColor);
        g.fillPath(wobblePath);

        float strokeWidth = baseStrokeWidth + (rng.nextFloat() * 2.0f - 1.0f) * wobbleSettings.strokeWidthVariation * baseStrokeWidth;
        g.setColour(outlineColor);
        g.strokePath(wobblePath, juce::PathStrokeType(strokeWidth));
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                       bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto buttonId = button.getName().hashCode() + static_cast<int>(reinterpret_cast<intptr_t>(&button));
        juce::Random rng(buttonId + 12345);

        juce::Colour textColor = textColorRange.getRandomColor(rng); // Text stays 100% alpha

        if (shouldDrawButtonAsDown) textColor = textColor.brighter(0.2f);
        else if (shouldDrawButtonAsHighlighted) textColor = textColor.brighter(0.1f);

        g.setColour(textColor);
        g.setFont(button.getHeight() * 0.45f);
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
    }

    // ... (Text Editor functions use the same pattern: static_cast and baseAlpha)
    void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
    {
        auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(2.0f);
        auto editorId = textEditor.getName().hashCode() + static_cast<int>(reinterpret_cast<intptr_t>(&textEditor));
        juce::Random rng(editorId);
        g.setColour(buttonOutlineColorRange.getRandomColor(rng).withAlpha(baseAlpha));
        g.strokePath(createWobblyRoundedRectangle(bounds, rng), juce::PathStrokeType(baseStrokeWidth));
    }

    void fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
    {
        auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(2.0f);
        auto editorId = textEditor.getName().hashCode() + static_cast<int>(reinterpret_cast<intptr_t>(&textEditor));
        juce::Random rng(editorId);
        g.setColour(textBoxColorRange.getRandomColor(rng).withAlpha(baseAlpha));
        g.fillPath(createWobblyRoundedRectangle(bounds, rng));
    }

private:
    juce::Path createWobblyRoundedRectangle (juce::Rectangle<float> bounds, juce::Random& rng) const
    {
        juce::Path path;
        std::array<float, 4> cornerRadii;
        for (int i = 0; i < 4; ++i)
            cornerRadii[i] = juce::jmax (2.0f, baseCornerRadius * (1.0f + (rng.nextFloat() * 2.0f - 1.0f) * wobbleSettings.cornerRadiusVariation));

        auto x = bounds.getX(); auto y = bounds.getY();
        auto w = bounds.getWidth(); auto h = bounds.getHeight();
        int segs = wobbleSettings.segmentsPerSide;
        float wobble = wobbleSettings.edgeWobble;
        float cpVar = wobbleSettings.controlPointVariation; // Now being used below!

        auto addW = [&](float fx, float fy) {
            return juce::Point<float> (fx + (rng.nextFloat() * 2.0f - 1.0f) * wobble,
                                     fy + (rng.nextFloat() * 2.0f - 1.0f) * wobble);
        };

        auto startPoint = addW (x + cornerRadii[0], y);
        path.startNewSubPath (startPoint);

        // Updated Edge Drawer using cpVar for the "Drunk" effect
        auto drawEdge = [&](float x1, float y1, float x2, float y2, bool horizontal) {
            for (int i = 1; i <= segs; ++i) {
                float t = (float)i / (float)segs;
                auto nextP = horizontal ? addW (x1 + t * (x2 - x1), y1) : addW (x1, y1 + t * (y2 - y1));

                // Use cpVar to randomize the influence of the control points
                auto cp1 = addW (startPoint.x + (nextP.x - startPoint.x) * (0.33f + rng.nextFloat() * cpVar),
                                 startPoint.y + (nextP.y - startPoint.y) * (0.33f + rng.nextFloat() * cpVar));
                auto cp2 = addW (startPoint.x + (nextP.x - startPoint.x) * (0.67f + rng.nextFloat() * cpVar),
                                 startPoint.y + (nextP.y - startPoint.y) * (0.67f + rng.nextFloat() * cpVar));

                path.cubicTo (cp1, cp2, nextP);
                startPoint = nextP;
            }
        };

        // Draw the 4 wobbly sides
        drawEdge (x + cornerRadii[0], y, x + w - cornerRadii[1], y, true);
        path.addCentredArc (x + w - cornerRadii[1], y + cornerRadii[1], cornerRadii[1], cornerRadii[1], 0, 0, juce::MathConstants<float>::halfPi, true);
        startPoint = { x + w, y + cornerRadii[1] };

        drawEdge (x + w, y + cornerRadii[1], x + w, y + h - cornerRadii[2], false);
        path.addCentredArc (x + w - cornerRadii[2], y + h - cornerRadii[2], cornerRadii[2], cornerRadii[2], 0, juce::MathConstants<float>::halfPi, juce::MathConstants<float>::pi, true);
        startPoint = { x + w - cornerRadii[2], y + h };

        drawEdge (x + w - cornerRadii[2], y + h, x + cornerRadii[3], y + h, true);
        path.addCentredArc (x + cornerRadii[3], y + h - cornerRadii[3], cornerRadii[3], cornerRadii[3], 0, juce::MathConstants<float>::pi, juce::MathConstants<float>::pi * 1.5f, true);
        startPoint = { x, y + h - cornerRadii[3] };

        drawEdge (x, y + h - cornerRadii[3], x, y + cornerRadii[0], false);
        path.addCentredArc (x + cornerRadii[0], y + cornerRadii[0], cornerRadii[0], cornerRadii[0], 0, juce::MathConstants<float>::pi * 1.5f, juce::MathConstants<float>::twoPi, true);

        path.closeSubPath();
        return path;
    }

    ColorRange buttonOffColorRange, buttonOnColorRange, buttonOutlineColorRange, textColorRange, textBoxColorRange;
    WobbleSettings wobbleSettings;
    float baseStrokeWidth = 2.5f;
    float baseCornerRadius = 15.0f;
    float baseAlpha = 1.0f; // <--- This was the missing symbol!

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrunkDrawnLookAndFeel)
};