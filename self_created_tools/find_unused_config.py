import re
import os
import subprocess

def get_config_variables(filepath):
    variables = []
    with open(filepath, 'r') as f:
        content = f.read()

    # Simple regex to find "const type name =" or "constexpr type name ="
    # Handling nested namespaces is tricky but let's try to capture names
    # Improve regex to capture variable names after const/constexpr

    # Matches: const juce::Colour myVar
    # Matches: constexpr float myVar
    # Matches: static constexpr int myVar
    pattern = re.compile(r'(?:const|constexpr)\s+(?:[\w:<>]+)\s+(\w+)\s*(?:=|;|\{)')

    for line in content.splitlines():
        line = line.strip()
        if line.startswith('//') or line.startswith('#'):
            continue

        match = pattern.search(line)
        if match:
            var_name = match.group(1)
            # Skip common keywords if regex is too broad
            if var_name not in ['struct', 'class', 'namespace', 'return']:
                variables.append(var_name)
    return variables

def check_usage(var_name, source_dir):
    # Grep for the variable name in the source directory
    # exclude Config.h and Config.cpp from the count (or check if count > definition count)

    try:
        result = subprocess.run(
            ['grep', '-r', var_name, source_dir],
            capture_output=True, text=True
        )
        lines = result.stdout.splitlines()
        # Filter out Config.h and Config.cpp
        usage_count = 0
        for line in lines:
            if 'Config.h' in line or 'Config.cpp' in line:
                continue
            usage_count += 1
        return usage_count
    except Exception as e:
        print(f"Error checking {var_name}: {e}")
        return 0

def main():
    config_h = 'Source/Config.h'
    source_dir = 'Source'

    if not os.path.exists(config_h):
        print(f"File not found: {config_h}")
        return

    variables = get_config_variables(config_h)
    print(f"Found {len(variables)} variables in {config_h}")

    unused = []
    for var in variables:
        count = check_usage(var, source_dir)
        if count == 0:
            unused.append(var)
            print(f"POTENTIALLY UNUSED: {var}")
        else:
             # print(f"Used: {var} ({count})")
             pass

if __name__ == "__main__":
    main()
