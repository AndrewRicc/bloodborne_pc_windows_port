# modules

Tutto quello che il port usa ma che non fa parte del codice sta qui. Git ignora il contenuto di
queste cartelle: niente di quello che ci metti finisce nel repository.

| Cartella | Cosa ci va | Chi la riempie |
|---|---|---|
| `game\CUSA03173\` | La tua copia di Bloodborne (versione europea, 1.09): `eboot.bin`, `dvdroot_ps4`, `sce_sys` | Tu, copiando la cartella |
| `dlss-sdk\` | L'SDK del DLSS di NVIDIA, usato solo per compilare | Il comando `git clone` della guida |

Se il gioco è già altrove sul disco non serve copiarlo: passa il percorso a `run.bat` oppure
sceglilo nel launcher. Vedi `GUIDA.md` nella cartella principale.
