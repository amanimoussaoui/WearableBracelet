# SafeMind - dossier du projet

- logos/            Logos SafeMind (PNG + SVG, version bouclier et version Alzheimer/myosotis)
- maquettes/        Maquette interactive de l'app (HTML) et logo anime (HTML)
- supabase/schema.sql          Schema de la base Supabase (a executer dans SQL Editor)
- supabase/bracelet/           Code du bracelet ESP8266 : envoi des mesures vers Supabase
- supabase/app/                Services React Native : supabase.ts + braceletClient.ts

Ordre conseille : 1) schema.sql  2) firmware bracelet (+ 3 lignes dans config.h)
3) services de l'app  4) tests (voir les explications de la conversation).

Rappel securite : ne partagez jamais config.h (mot de passe WiFi, DEVICE_KEY).
