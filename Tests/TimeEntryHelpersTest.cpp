#include <juce_core/juce_core.h>
#include "../Source/TimeEntryHelpers.h"

class TimeEntryHelpersTest : public juce::UnitTest
{
public:
    TimeEntryHelpersTest() : juce::UnitTest("TimeEntryHelpers Testing") {}

    void runTest() override
    {
        beginTest("validateTime with valid input");
        {
            // Within range
            expect(TimeEntryHelpers::validateTime("00:00:10:000", 60.0) == TimeEntryHelpers::ValidationResult::Valid);
            expect(TimeEntryHelpers::validateTime("00:00:00:000", 60.0) == TimeEntryHelpers::ValidationResult::Valid);
            expect(TimeEntryHelpers::validateTime("00:01:00:000", 60.0) == TimeEntryHelpers::ValidationResult::Valid);
        }

        beginTest("validateTime with invalid input (parsing error)");
        {
             expect(TimeEntryHelpers::validateTime("invalid", 60.0) == TimeEntryHelpers::ValidationResult::Invalid);
             expect(TimeEntryHelpers::validateTime("", 60.0) == TimeEntryHelpers::ValidationResult::Invalid);
             expect(TimeEntryHelpers::validateTime("00:00", 60.0) == TimeEntryHelpers::ValidationResult::Invalid);
        }

        beginTest("validateTime with out of range input");
        {
            // Greater than total length
            expect(TimeEntryHelpers::validateTime("00:01:01:000", 60.0) == TimeEntryHelpers::ValidationResult::OutOfRange);

            // Negative input strings are parsed as positive magnitude by TimeUtils::parseTime
            // So "-00:00:10:000" -> 10.0 -> Valid if totalLength >= 10.0
             expect(TimeEntryHelpers::validateTime("-00:00:10:000", 60.0) == TimeEntryHelpers::ValidationResult::Valid);
        }
    }
};

static TimeEntryHelpersTest timeEntryHelpersTest;
