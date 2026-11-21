# 8. Preset di fabbrica

Sono **punti di partenza**, tarati per sorgenti intorno a 0 VU (−18 dBFS RMS). La quantità di
compressione dipende dal livello della traccia: regola Peak Reduction finché il VU (modo GR) mostra
la riduzione desiderata, poi Gain per il livello. I parametri non elencati restano al default; i
preset non toccano mai Bypass né la modalità del VU.

| Preset | Impostazioni | GR di picco misurata* | Uscita/ingresso* |
|---|---|---|---|
| Init | tutti i default (PR 30, Gain 50, Compress, Mix 100 %, Colour 50 %, 2x) | 1,0 dB | −1,0 dB |
| Vocal - Smooth Leveling | PR 52, Gain 62, Colour 45 %, HPF detector 80 Hz | 7,3 dB | −0,1 dB |
| Bass - Even Sustain | PR 58, Gain 65, Colour 60 % | 9,7 dB | −0,7 dB |
| Drums - Parallel Smash | PR 82, Gain 76, **Limit**, **Mix 35 %**, Colour 70 %, HPF 60 Hz, 4x | 24,7 dB | −2,2 dB |
| Gentle Glue | PR 38, Gain 53, Colour 35 % | 2,7 dB | −0,9 dB |
| Parallel Bus (100% Wet) | PR 75, Gain 72, **Limit**, Mix 100 %, Colour 60 % — per aux/bus esterno | 20,9 dB | −7,7 dB |

\* Misure del test `presets` su un two-tone 220 Hz + 2,75 kHz a −18 dBFS RMS, 48 kHz, ultimo
secondo di 4 s. Con materiale musicale reale i valori saranno diversi.

Uso suggerito del preset per batteria: in insert con Mix 35 % il compresso pesante si somma al
dry pulito; prova anche `Mix Law = Dry Constant` con Mix 25–50 % per il classico parallelo
"aggiuntivo".
