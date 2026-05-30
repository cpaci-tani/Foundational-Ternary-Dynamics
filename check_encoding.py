with open('docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md', 'r', encoding='utf-8') as f:
    text = f.read()

bad_chars = ['✅', '—', '§', '�']
found = [c for c in bad_chars if c in text]
if found:
    print('Garbled characters found:', found)
else:
    print('No garbled characters found.')
