curl -L https://github.com/jdecked/twemoji/archive/refs/tags/v15.1.0.tar.gz | tar xzf - -C out
python scripts/pack_emojis.py > out/emojis.c