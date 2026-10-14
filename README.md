# PTZR CLOUD

Instrumento granular (AU / VST3 / Standalone), estéreo, com entrada MIDI.
Segure **uma nota** e a nuvem cria o pad (loops de voz, cordas, pianos granulares...)
e faz "piscar" notas melódicas dentro do mesmo tom (ex.: segura C -> aparecem G, C, D).

## Bibliotecas (10) x 4 presets = 40 presets
Ethereal Choir · Celestial Strings · Warm Halo Pad · Glass Piano · Tape Cloud ·
Morning Breeze · Grand Cathedral · Angel Bells · Sunrise Keys · Shimmer Air

As bibliotecas são geradas por código (síntese aditiva) ao carregar - não há samples externos.

## Controles
- **GRAIN**: Size, Density, Position, Scan, Spread, Pitch (granular), Detune
- **HARMONY**: Color (Sus/Major/Minor/Lydian), Harmony, Octave, Blink (velocidade), Sparkle (volume das notas piscando)
- **REVERB**: 6 tipos (Hall, Plate, Room, Cathedral, Shimmer, Infinite), Mix, Size, Damp, Pre-delay, Width, Shimmer + intervalo (Oct Up / 5th Up / Oct Down)
- **DELAY**: Stereo / Ping-Pong / Tape, Sync (host BPM) ou tempo livre, Feedback, Mix
- **TREMOLO**: Sine / Triangle / Square, Rate, Depth, Stereo
- **TAPE**: Wobble, Glitch (stutter + grãos invertidos)
- **TONE / ENV**: filtro, Attack, Release · **OUT**: Master
- MIDI: notas, velocity, pitch bend (±2), pedal de sustain (CC64), All Notes Off

## Compilar no Mac
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
O CMake baixa o JUCE sozinho. Com `COPY_PLUGIN_AFTER_BUILD` o AU vai para
`~/Library/Audio/Plug-Ins/Components` e o VST3 para `~/Library/Audio/Plug-Ins/VST3`.

## Compilar pelo GitHub
Suba tudo no repositório. Em **Actions -> Build PTZR CLOUD -> Run workflow** o GitHub
compila no macOS e disponibiliza `PTZR-CLOUD-macOS.zip` em *Artifacts*.

## Logo
A logo fica em `Resources/ptzr_logo.png` (branca, fundo transparente). Para trocar, substitua o arquivo e recompile.
