import re

def modify_file():
    with open("Source/LoopPresenter.cpp", "r") as f:
        content = f.read()

    # Modify setLoopInPosition
    # Looking for the closing brace of setLoopInPosition.
    # It starts at line 32. Let's find the function body.
    # It ends before "void LoopPresenter::setLoopOutPosition".

    # We can match the function signature and replace the whole block or inject code.
    # But since I have the whole file content, I can replace carefully.

    # 1. Modify setLoopInPosition
    # Find the end of setLoopInPosition (before setLoopOutPosition)
    match_in = re.search(r'(void LoopPresenter::setLoopInPosition.*?)(void LoopPresenter::setLoopOutPosition)', content, re.DOTALL)
    if match_in:
        func_body = match_in.group(1)
        # Find the last closing brace
        last_brace_index = func_body.rfind('}')
        if last_brace_index != -1:
            new_func_body = func_body[:last_brace_index] + '    ensureLoopOrder();\n' + func_body[last_brace_index:]
            content = content.replace(func_body, new_func_body)

    # 2. Modify setLoopOutPosition
    # Find the end of setLoopOutPosition (before ensureLoopOrder)
    match_out = re.search(r'(void LoopPresenter::setLoopOutPosition.*?)(void LoopPresenter::ensureLoopOrder)', content, re.DOTALL)
    if match_out:
        func_body = match_out.group(1)
        # Find the last closing brace
        last_brace_index = func_body.rfind('}')
        if last_brace_index != -1:
            new_func_body = func_body[:last_brace_index] + '    ensureLoopOrder();\n' + func_body[last_brace_index:]
            content = content.replace(func_body, new_func_body)

    # 3. Modify applyLoopInFromEditor
    # Remove the blocking check
    pattern_block_in = r'if \(loopOutPosition > -1\.0 && newPosition > loopOutPosition\)\s*\{[^}]*\}'
    content = re.sub(pattern_block_in, '', content, count=1, flags=re.DOTALL)

    # Add updateLoopLabels() before return true
    # We look for "return true;" inside applyLoopInFromEditor context.
    # To be safe, let's find the function first.
    start_idx = content.find("bool LoopPresenter::applyLoopInFromEditor")
    end_idx = content.find("bool LoopPresenter::applyLoopOutFromEditor")
    if start_idx != -1 and end_idx != -1:
        func_content = content[start_idx:end_idx]
        if "updateLoopLabels();" not in func_content:
             new_func_content = func_content.replace("return true;", "updateLoopLabels();\n        return true;")
             content = content.replace(func_content, new_func_content)

    # 4. Modify applyLoopOutFromEditor
    # Remove the blocking check
    pattern_block_out = r'if \(loopInPosition > -1\.0 && newPosition < loopInPosition\)\s*\{[^}]*\}'
    content = re.sub(pattern_block_out, '', content, count=1, flags=re.DOTALL)

    # Add updateLoopLabels() before return true
    # Find the function first.
    start_idx = content.find("bool LoopPresenter::applyLoopOutFromEditor")
    end_idx = content.find("void LoopPresenter::syncEditorToPosition") # Next function
    if start_idx != -1 and end_idx != -1:
        func_content = content[start_idx:end_idx]
        if "updateLoopLabels();" not in func_content:
             new_func_content = func_content.replace("return true;", "updateLoopLabels();\n        return true;")
             content = content.replace(func_content, new_func_content)

    with open("Source/LoopPresenter.cpp", "w") as f:
        f.write(content)

if __name__ == "__main__":
    modify_file()
