import os
import glob
import re
import subprocess
import sys

def ensure_emoji_lib():
    try:
        import emoji
        return True
    except ImportError:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "emoji"])
        return True

ensure_emoji_lib()
import emoji

def remove_emojis(text):
    return emoji.replace_emoji(text, replace='')

count = 0
for filepath in glob.glob('**/*.md', recursive=True):
    # Skip build/engine dependencies if any exist, just focus on docs and root
    if 'node_modules' in filepath or '.gemini' in filepath or 'build' in filepath:
        continue
        
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            
        new_content = remove_emojis(content)
        
        if content != new_content:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(new_content)
            count += 1
            print(f"Removed emojis from {filepath}")
    except Exception as e:
        print(f"Error processing {filepath}: {e}")

print(f"Total files cleaned: {count}")
