import os

target_snippets = [
    "juce::File audioFile(TEST_FILE_PATH)",
    "TESTING_MODE",
    "TEST_FILE_PATH"
]

for root, dirs, files in os.walk("."):
    for file in files:
        if file.endswith(".cpp") or file.endswith(".h"):
            filepath = os.path.join(root, file)
            try:
                with open(filepath, "r") as f:
                    content = f.read()
                    for snippet in target_snippets:
                        if snippet in content:
                            print(f"Found '{snippet}' in {filepath}")
            except Exception as e:
                print(f"Could not read {filepath}: {e}")
