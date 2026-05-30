# Projet ESP32 + HomeSpan + HomeKit

Ce projet utilise une carte **AZ-Delivery DevKit V4** (ESP32), la librairie HomeSpan pour l’intégration Apple HomeKit, et PlatformIO comme environnement de développement.[web:28]

## Configuration PlatformIO

Créer un fichier `platformio.ini` avec le contenu suivant :

```ini
[env:az-delivery-devkit-v4]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/53.03.10/platform-espressif32.zip
board = az-delivery-devkit-v4
framework = arduino

; ==============================
; Optimisations pour réduire la taille du binaire
; ==============================
build_flags =
    -Os                      ; optimise pour la taille
    -flto                    ; Link Time Optimization
    -DHOMEKIT_DISABLE_DEBUG  ; désactive debug HomeKit si nécessaire
    -fno-lto                 ; à enlever si conflit avec -flto

; ==============================
; Partition scheme pour plus de flash
; ==============================
board_build.partitions = partitions/huge_app.csv
board_build.flash_size = 4MB

; ==============================
; Librairies
; ==============================
lib_deps =
    FastLED@^3.10.1
    git+https://github.com/HomeSpan/HomeSpan.git#2.1.3

; ==============================
; Autres options utiles
; ==============================
monitor_speed = 115200
upload_speed = 921600
```
## Fichier partitions/huge_app.csv :

# Name,   Type, SubType, Offset,  Size,    Flags
nvs,      data, nvs,     0x9000,  0x7000,
app0,     app,  factory, 0x10000, 0x3F0000
```
Cette configuration réserve un maximum de flash à l’application tout en gardant une partition NVS pour les credentials Wi‑Fi et les données HomeKit.


Mode opératoire : effacer la configuration Wi‑Fi existante

HomeSpan stocke les identifiants Wi‑Fi dans la NVS de l’ESP32 et permet de les effacer sans perdre le pairing HomeKit (sauf action spécifique).[web:24]
1. Via la CLI série HomeSpan

    Connecter l’ESP32 en USB et ouvrir le moniteur série (115200 bauds).

    Laisser HomeSpan démarrer complètement.

    Dans le terminal série, taper la lettre X (majuscule, sans retour à la ligne supplémentaire si ton terminal en ajoute déjà un).

        La commande X supprime les credentials Wi‑Fi stockés en NVS et redémarre le device.[web:21]

    Après le reboot, HomeSpan indiquera qu’aucun Wi‑Fi n’est configuré et restera en attente de nouveaux identifiants.[web:22]

Cette méthode efface uniquement le Wi‑Fi, la configuration HomeKit (pairing) reste intacte.[web:24]
2. Via le bouton « Control Button » (si câblé)

Si tu as câblé un HomeSpan Control Button et un Status LED conformément au User Guide :[web:24]

    Mettre le device en Device Configuration Mode (combinaison d’appuis décrite dans la doc HomeSpan).

    Suivre la procédure « Erase stored WiFi Credentials » (appui prolongé, Led clignotant à un rythme spécifique).

    Relâcher le bouton quand le pattern correspondant à l’effacement Wi‑Fi est actif.

HomeSpan efface alors les credentials Wi‑Fi et redémarre, sans effacer le code HomeKit ou le pairing (sauf si tu utilises l’option qui supprime aussi les données HomeKit).[web:24]
