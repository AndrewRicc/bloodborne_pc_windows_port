# Bloodborne su PC con DLSS e menu multilingua: guida rapida

Questo repository è il port per Windows di Bloodborne (di Mrsuss60, basato su bbport di
deadinside28) con due aggiunte: il **DLSS** di NVIDIA come upscaler e il **menu in 12 lingue**.

Il gioco non è incluso: serve una copia tua, estratta da una PS4.

## Com'è organizzata la cartella

| Cosa | A cosa serve |
|---|---|
| `setup.bat` | Fa tutto il setup: controlla i requisiti, installa quello che manca, compila |
| `launch_gui.bat` | Avvia il launcher grafico |
| `run.bat` | Avvia il gioco direttamente |
| `modules\game\` | **Qui va la cartella del gioco** (`CUSA03173`) |
| `modules\dlss-sdk\` | Qui finisce l'SDK del DLSS di NVIDIA (lo scarica `setup.bat`) |
| `out\` | Il gioco compilato (`bbport.exe` e le sue DLL) |
| `user\savedata\` | I salvataggi |
| `bbport.ini` | Le impostazioni scelte nel menu |
| `scripts\windows\` | Gli script usati da `setup.bat` |
| `gpu\`, `src\`, `third_party\` | Il codice |

`modules`, `out`, `user` e `bbport.ini` sono ignorati da git.

## Cosa ti serve

- **Il gioco**: Bloodborne versione europea (`CUSA03173`) aggiornato alla **1.09**. La cartella
  deve contenere `eboot.bin`, `dvdroot_ps4` e `sce_sys`.
- **PC**: Windows 10 o 11 a 64 bit, CPU con AVX2, scheda video con Vulkan 1.3 e driver aggiornati.
- **Spazio**: circa 15 GB, più il gioco.
- **Per il DLSS**: una NVIDIA RTX. Senza, il gioco funziona lo stesso con FSR.
- **Git**, per scaricare il progetto. Se non ce l'hai:

```powershell
winget install --id Git.Git -e
```

Poi chiudi e riapri PowerShell. Tutto il resto lo installa `setup.bat`.

## 1. Scarica il progetto

Apri PowerShell nella cartella dove vuoi tenerlo e lancia:

```powershell
git clone -c core.longpaths=true --branch dlss-i18n https://github.com/AndrewRicc/bloodborne_pc_windows_port.git bloodborne
```

```powershell
cd bloodborne
```

Non scaricarlo come ZIP da GitHub: mancherebbero dei componenti.

## 2. Metti il gioco in `modules\game`

Copia la cartella `CUSA03173` dentro `modules\game`, così:

```
bloodborne\modules\game\CUSA03173\eboot.bin
bloodborne\modules\game\CUSA03173\dvdroot_ps4\
bloodborne\modules\game\CUSA03173\sce_sys\
```

Il port legge quella cartella senza modificarla. Se il gioco è già altrove sul disco e non vuoi
copiarlo, lascialo dov'è: indicherai il percorso al passo 4.

## 3. Lancia il setup

```powershell
.\setup.bat
```

Lo script controlla ogni requisito e installa solo quello che manca:

| Controllo | Se manca |
|---|---|
| Windows a 64 bit, CPU con AVX2 | Si ferma: il gioco non può funzionare |
| Python 3.10+ | Lo installa con winget |
| MSYS2 in `C:\msys64` (il compilatore) | Lo installa con winget e lo aggiorna |
| Librerie di MSYS2 (circa 2,5 GB) | Le installa |
| Visual Studio Build Tools con C++ (circa 7 GB) | Li installa, solo se hai una RTX |
| SDK DLSS in `modules\dlss-sdk` (circa 1,6 GB) | Lo scarica, solo se hai una RTX |
| Sottomoduli del codice e correzione di FSR | Li scarica e la applica |

Poi compila: la prima volta servono 20-30 minuti. Alla fine deve comparire `Setup completato.`

Windows può chiedere conferma per le installazioni (finestre "Vuoi consentire..."): rispondi sì.

Se lo script si ferma con un errore, risolvi quello che indica e **rilancialo**: i passi già fatti
vengono saltati. Vale anche dopo un aggiornamento del codice, per ricompilare.

Opzioni:

- `.\setup.bat -SoloControllo` dice cosa manca senza installare né compilare nulla.
- `.\setup.bat -SenzaDlss` salta Visual Studio e l'SDK di NVIDIA (circa 9 GB in meno).

## 4. Avvia il gioco

Con il launcher (consigliato): doppio clic su `launch_gui.bat`, oppure

```powershell
.\launch_gui.bat
```

Se il gioco è in `modules\game\CUSA03173` il launcher lo trova da solo; altrimenti premi
**Browse** e scegli il tuo `eboot.bin`. Lascia 60 FPS e 1080p e premi **Launch Bloodborne**.

Senza launcher:

```powershell
.\run.bat
```

Con il gioco in un'altra cartella:

```powershell
.\run.bat "D:\percorso\CUSA03173"
```

Al primo avvio qualche scatto è normale: il gioco sta preparando gli shader.

## 5. In gioco

- **Ins** sulla tastiera, o **L3 + R3** sul controller, apre il menu.
- **Lingua**: è la prima voce. "Auto" segue la lingua di Windows.
- **Upscaler**: scegli **DLSS**. *Native AA* è la massima qualità senza guadagno di FPS;
  *Quality*, *Balanced*, *Performance* e *Ultra Performance* abbassano la risoluzione interna.
- **Spostamento subpixel (jitter)**: deve restare attivo, altrimenti il DLSS lavora male.
- **F11** o **Shift+Tab** mostra l'HUD delle prestazioni.
- Le scelte vengono salvate in `bbport.ini` alla chiusura del menu.

Se con il DLSS l'immagine trema, prova la casella **DLSS: segno jitter invertito**. Se resta un
problema, torna a FSR 3.1 dallo stesso menu.

**Stato del DLSS.** Si inizializza e gira senza errori. La qualità dell'immagine in partita non è
ancora stata verificata a fondo.

## Problemi frequenti

| Sintomo | Cosa fare |
|---|---|
| `setup.bat` dice che la cartella non è un clone git | Hai scaricato lo ZIP: rifai il passo 1 con `git clone`. |
| Errore `429` durante le librerie | Il server di MSYS2 è occupato: rilancia `setup.bat` dopo qualche minuto. |
| `cannot open output file ... bbport.exe` | Il gioco è aperto: chiudilo e rilancia `setup.bat`. |
| Python installato ma "non ancora visibile" | Chiudi PowerShell, riaprilo e rilancia `setup.bat`. |
| DLSS grigio nel menu, "non supportato dalla GPU" | Serve una RTX con driver recente. Controlla che in `out` ci siano `bbdlss.dll` e `nvngx_dlss.dll`; se mancano rilancia `setup.bat`. |
| "FSR 4 (INT8)" dà errore `missing fsr4_shaders/...` | Mancano gli shader di FSR 4: rilancia `setup.bat`, che li scarica (circa 45 MB). |
| "FSR 4.1.1 (INT8)" grigio, "non supportato dalla GPU" | Su Windows è normale, anche con una GPU AMD recente: vedi la nota sugli upscaler qui sotto. |
| Scritta arancione "DLSS non disponibile: ..." | Il gioco è passato da solo a FSR 3.1; il testo dopo i due punti dice il motivo. |
| `No eboot.bin in ...` | Il gioco non è in `modules\game\CUSA03173`: copialo lì o passa il percorso a `run.bat`. |
| Gioco lentissimo | Sta usando la grafica integrata: in Impostazioni > Sistema > Schermo > Grafica aggiungi `out\bbport.exe` e scegli "Prestazioni elevate". |

## Quali upscaler funzionano su Windows

| Upscaler | Cosa serve |
|---|---|
| FSR 3.1, TAA | Niente: funzionano su ogni GPU. |
| FSR 4 (INT8) | Gli shader in `fsr4_shaders` (li scarica `setup.bat`) e una GPU con le funzioni Vulkan richieste (interi a 8 bit, prodotto scalare intero, derivate nei compute shader). Funziona su AMD, NVIDIA e Intel recenti. |
| FSR 4.1.1 (INT8) | L'estensione Vulkan `VK_VALVE_shader_mixed_float_dot_product` nel driver, più un set di file (`fsr4_411`) che va generato dalla DLL di AMD con strumenti che girano solo su Linux. In pratica oggi su Windows non è utilizzabile. |
| DLSS | Una NVIDIA RTX. |

Per sapere se il driver espone l'estensione di FSR 4.1.1:

```powershell
vulkaninfo | Select-String mixed_float_dot_product
```

Se non stampa nulla, il driver non la supporta e la voce resta grigia.

## Limiti noti dello script di setup

- MSYS2 deve stare in `C:\msys64`, il percorso predefinito.
- Lo script è stato provato su un PC con tutto già installato. I rami che installano i programmi
  mancanti non sono ancora stati provati su un PC pulito.
