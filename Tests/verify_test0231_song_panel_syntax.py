"""Guard the Song End branch from accidentally commenting out its closing brace."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'Source/TrackerSongPanel.cpp').read_text()
start = source.index('void TrackerSongPanel::currentValue(double value){')
end = source.index('void TrackerSongPanel::adjustValue(', start)
function = source[start:end]
assert 'if(n>=0)processor.trackerSongs.at(n); // Ensure the Song End target exists.' in function
assert '}else if(row<0){' in function

# Ignore comments and string literals while checking the block delimiter balance.
def structural_braces(text):
    braces = 0
    state = 'normal'
    i = 0
    while i < len(text):
        c = text[i]
        following = text[i + 1] if i + 1 < len(text) else ''
        if state == 'normal':
            if c == '/' and following == '/':
                state = 'line_comment'; i += 1
            elif c == '/' and following == '*':
                state = 'block_comment'; i += 1
            elif c == '"':
                state = 'string'
            elif c == "'":
                state = 'character'
            elif c == '{':
                braces += 1
            elif c == '}':
                braces -= 1
                assert braces >= 0, 'unexpected closing brace'
        elif state == 'line_comment' and c == '\n':
            state = 'normal'
        elif state == 'block_comment' and c == '*' and following == '/':
            state = 'normal'; i += 1
        elif state in ('string', 'character'):
            if c == '\\':
                i += 1
            elif (c == '"' and state == 'string') or (c == "'" and state == 'character'):
                state = 'normal'
        i += 1
    assert state == 'normal', f'unclosed comment or string: {state}'
    return braces

assert structural_braces(function) == 0, 'currentValue() has an unmatched brace'
assert structural_braces(source) == 0, 'TrackerSongPanel.cpp has an unmatched brace'
print('OK TEST0.2.35 Song End C++ structural regression')
