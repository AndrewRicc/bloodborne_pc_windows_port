// SPDX-License-Identifier: GPL-2.0-or-later
#include "bbport_i18n.h"

#include <cstdlib>
#include <cstring>

#include "bbport_settings.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace BbI18n {

namespace {

struct Entry {
    Str id;
    const char* text[LangCount]; ///< En, Ru, It, Es, Fr, De, Pt, Pl, Tr, Zh, Ja, Ko
};

// clang-format off
constexpr Entry kEntries[] = {
{Title, {
    "Bloodborne — settings  (Insert / L3+R3)",
    "Bloodborne — настройки  (Insert / L3+R3)",
    "Bloodborne — impostazioni  (Insert / L3+R3)",
    "Bloodborne — ajustes  (Insert / L3+R3)",
    "Bloodborne — paramètres  (Insert / L3+R3)",
    "Bloodborne — Einstellungen  (Insert / L3+R3)",
    "Bloodborne — configurações  (Insert / L3+R3)",
    "Bloodborne — ustawienia  (Insert / L3+R3)",
    "Bloodborne — ayarlar  (Insert / L3+R3)",
    "Bloodborne — 设置  (Insert / L3+R3)",
    "Bloodborne — 設定  (Insert / L3+R3)",
    "Bloodborne — 설정  (Insert / L3+R3)"}},
{FpsLine, {
    "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f мс)", "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)",
    "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)",
    "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)"}},
{SecUpscaler, {
    "Temporal upscaler", "Временной апскейлер", "Upscaler temporale", "Escalado temporal",
    "Upscaler temporel", "Temporaler Upscaler", "Upscaler temporal", "Upscaler czasowy",
    "Zamansal ölçekleyici", "时间性超分辨率", "テンポラル アップスケーラー", "시간적 업스케일러"}},
{UpOff, {
    "Off", "Выкл", "Disattivato", "Desactivado", "Désactivé", "Aus", "Desativado", "Wyłączony",
    "Kapalı", "关闭", "オフ", "끔"}},
{UpTaa, {
    "TAA (native anti-aliasing)", "TAA (нативное сглаживание)", "TAA (antialiasing nativo)",
    "TAA (antialiasing nativo)", "TAA (anticrénelage natif)", "TAA (natives Anti-Aliasing)",
    "TAA (anti-aliasing nativo)", "TAA (natywny antyaliasing)", "TAA (yerel kenar yumuşatma)",
    "TAA（原生抗锯齿）", "TAA（ネイティブ アンチエイリアス）", "TAA (네이티브 안티에일리어싱)"}},
{Upscaler, {
    "Upscaler", "Апскейлер", "Upscaler", "Escalador", "Upscaler", "Upscaler", "Upscaler",
    "Upscaler", "Ölçekleyici", "超分辨率", "アップスケーラー", "업스케일러"}},
{NotSupported, {
    "— not supported by the GPU", "— не поддерживается видеокартой", "— non supportato dalla GPU",
    "— no compatible con la GPU", "— non pris en charge par le GPU",
    "— von der GPU nicht unterstützt", "— não suportado pela GPU", "— nieobsługiwany przez GPU",
    "— GPU desteklemiyor", "— 显卡不支持", "— GPU 非対応", "— GPU 미지원"}},
{InProgress, {
    "— in progress", "— в работе", "— in sviluppo", "— en desarrollo", "— en cours",
    "— in Arbeit", "— em desenvolvimento", "— w przygotowaniu", "— geliştiriliyor", "— 开发中",
    "— 開発中", "— 개발 중"}},
{Fsr4Unavailable, {
    "FSR 4 unavailable: %s", "FSR 4 недоступен: %s", "FSR 4 non disponibile: %s",
    "FSR 4 no disponible: %s", "FSR 4 indisponible : %s", "FSR 4 nicht verfügbar: %s",
    "FSR 4 indisponível: %s", "FSR 4 niedostępny: %s", "FSR 4 kullanılamıyor: %s",
    "FSR 4 不可用：%s", "FSR 4 は使用できません: %s", "FSR 4 사용 불가: %s"}},
{Fsr4Fallback, {
    "The mode selected above is active. FSR 4 can be selected again.",
    "Активен режим, выбранный выше. FSR 4 можно выбрать снова.",
    "È attiva la modalità scelta sopra. FSR 4 si può riselezionare.",
    "Está activo el modo seleccionado arriba. Puedes volver a elegir FSR 4.",
    "Le mode choisi ci-dessus est actif. FSR 4 peut être sélectionné à nouveau.",
    "Der oben gewählte Modus ist aktiv. FSR 4 kann erneut gewählt werden.",
    "O modo selecionado acima está ativo. O FSR 4 pode ser selecionado novamente.",
    "Aktywny jest tryb wybrany powyżej. FSR 4 można wybrać ponownie.",
    "Yukarıda seçilen mod etkin. FSR 4 yeniden seçilebilir.",
    "当前使用上方所选的模式。可再次选择 FSR 4。",
    "上で選択したモードが有効です。FSR 4 は再度選択できます。",
    "위에서 선택한 모드가 활성화되어 있습니다. FSR 4를 다시 선택할 수 있습니다."}},
{DlssUnavailable, {
    "DLSS unavailable: %s", "DLSS недоступен: %s", "DLSS non disponibile: %s",
    "DLSS no disponible: %s", "DLSS indisponible : %s", "DLSS nicht verfügbar: %s",
    "DLSS indisponível: %s", "DLSS niedostępny: %s", "DLSS kullanılamıyor: %s",
    "DLSS 不可用：%s", "DLSS は使用できません: %s", "DLSS 사용 불가: %s"}},
{DlssHint, {
    "NVIDIA DLSS Super Resolution (beta): needs bbdlss.dll and nvngx_dlss.dll next to bbport.exe "
    "and an RTX GPU. Native AA = DLAA, the other presets are the DLSS modes. DLSS applies no "
    "sharpening: the slider above 1 adds an RCAS pass.",
    "NVIDIA DLSS Super Resolution (бета): bbdlss.dll и nvngx_dlss.dll рядом с bbport.exe, "
    "видеокарта RTX. Native AA = DLAA, остальные пресеты — режимы DLSS. Резкость DLSS "
    "не применяет: ползунок выше 1 добавляет проход RCAS.",
    "NVIDIA DLSS Super Resolution (beta): servono bbdlss.dll e nvngx_dlss.dll accanto a "
    "bbport.exe e una GPU RTX. Native AA = DLAA, gli altri preset sono le modalità DLSS. DLSS non "
    "applica nitidezza: il cursore oltre 1 aggiunge un passaggio RCAS.",
    "NVIDIA DLSS Super Resolution (beta): requiere bbdlss.dll y nvngx_dlss.dll junto a bbport.exe "
    "y una GPU RTX. Native AA = DLAA; los demás preajustes son los modos de DLSS. DLSS no aplica "
    "nitidez: el control por encima de 1 añade una pasada RCAS.",
    "NVIDIA DLSS Super Resolution (bêta) : nécessite bbdlss.dll et nvngx_dlss.dll à côté de "
    "bbport.exe et un GPU RTX. Native AA = DLAA, les autres préréglages sont les modes DLSS. DLSS "
    "n'applique pas de netteté : au-delà de 1, le curseur ajoute une passe RCAS.",
    "NVIDIA DLSS Super Resolution (Beta): benötigt bbdlss.dll und nvngx_dlss.dll neben bbport.exe "
    "sowie eine RTX-GPU. Native AA = DLAA, die übrigen Presets sind die DLSS-Modi. DLSS schärft "
    "nicht nach: Werte über 1 fügen einen RCAS-Durchgang hinzu.",
    "NVIDIA DLSS Super Resolution (beta): requer bbdlss.dll e nvngx_dlss.dll ao lado do "
    "bbport.exe e uma GPU RTX. Native AA = DLAA; as outras predefinições são os modos do DLSS. O "
    "DLSS não aplica nitidez: o controle acima de 1 adiciona uma passagem RCAS.",
    "NVIDIA DLSS Super Resolution (beta): wymaga bbdlss.dll i nvngx_dlss.dll obok bbport.exe "
    "oraz karty RTX. Native AA = DLAA, pozostałe presety to tryby DLSS. DLSS nie wyostrza obrazu: "
    "suwak powyżej 1 dodaje przebieg RCAS.",
    "NVIDIA DLSS Super Resolution (beta): bbport.exe yanında bbdlss.dll ve nvngx_dlss.dll ile bir "
    "RTX GPU gerekir. Native AA = DLAA, diğer ön ayarlar DLSS modlarıdır. DLSS keskinleştirme "
    "uygulamaz: 1'in üzerindeki değerler bir RCAS geçişi ekler.",
    "NVIDIA DLSS 超分辨率（测试版）：需要 bbport.exe 旁的 bbdlss.dll 和 nvngx_dlss.dll，以及 RTX "
    "显卡。Native AA = DLAA，其余预设对应 DLSS 模式。DLSS 不做锐化：滑块大于 1 时会额外执行一次 RCAS。",
    "NVIDIA DLSS 超解像（ベータ）：bbport.exe と同じフォルダーに bbdlss.dll と nvngx_dlss.dll、"
    "および RTX GPU が必要です。Native AA = DLAA、その他のプリセットは DLSS の各モードです。DLSS は"
    "シャープ化を行いません。スライダーを 1 より上げると RCAS パスが追加されます。",
    "NVIDIA DLSS 슈퍼 해상도(베타): bbport.exe 옆에 bbdlss.dll과 nvngx_dlss.dll, 그리고 RTX GPU가 "
    "필요합니다. Native AA = DLAA, 나머지 프리셋은 DLSS 모드입니다. DLSS는 선명화를 적용하지 않으며, "
    "슬라이더를 1보다 높이면 RCAS 패스가 추가됩니다."}},
{DlssInvertJitter, {
    "DLSS: invert jitter sign", "DLSS: обратный знак jitter", "DLSS: segno jitter invertito",
    "DLSS: invertir signo del jitter", "DLSS : inverser le signe du jitter",
    "DLSS: Jitter-Vorzeichen umkehren", "DLSS: inverter sinal do jitter",
    "DLSS: odwróć znak jittera", "DLSS: jitter işaretini ters çevir", "DLSS：反转抖动符号",
    "DLSS: ジッターの符号を反転", "DLSS: 지터 부호 반전"}},
{Fsr411Hint, {
    "FSR 4.1.1 in INT8 mode: the model from AMD's 4.1.1 DLL, reproduced in Vulkan (the output "
    "matches the DLL). One model for Native..Performance and a separate one for Ultra "
    "Performance. Assets: tools/fsr4cap/build_assets.sh (needs the DLL and Proton).",
    "FSR 4.1.1 в режиме INT8: модель из DLL AMD 4.1.1, воспроизведённая в Vulkan "
    "(результат совпадает с DLL). Одна модель для Native..Performance и отдельная "
    "для Ultra Performance. Ассеты: tools/fsr4cap/build_assets.sh (нужны DLL и Proton).",
    "FSR 4.1.1 in modalità INT8: modello dalla DLL AMD 4.1.1, riprodotto in Vulkan (risultato "
    "identico alla DLL). Un modello per Native..Performance e uno separato per Ultra "
    "Performance. Asset: tools/fsr4cap/build_assets.sh (servono DLL e Proton).",
    "FSR 4.1.1 en modo INT8: el modelo de la DLL 4.1.1 de AMD, reproducido en Vulkan (el "
    "resultado coincide con la DLL). Un modelo para Native..Performance y otro para Ultra "
    "Performance. Recursos: tools/fsr4cap/build_assets.sh (requiere la DLL y Proton).",
    "FSR 4.1.1 en mode INT8 : le modèle de la DLL AMD 4.1.1, reproduit en Vulkan (résultat "
    "identique à la DLL). Un modèle pour Native..Performance et un autre pour Ultra "
    "Performance. Ressources : tools/fsr4cap/build_assets.sh (nécessite la DLL et Proton).",
    "FSR 4.1.1 im INT8-Modus: das Modell aus AMDs 4.1.1-DLL, in Vulkan nachgebildet (Ergebnis "
    "identisch mit der DLL). Ein Modell für Native..Performance und ein eigenes für Ultra "
    "Performance. Assets: tools/fsr4cap/build_assets.sh (benötigt die DLL und Proton).",
    "FSR 4.1.1 em modo INT8: o modelo da DLL 4.1.1 da AMD, reproduzido em Vulkan (resultado "
    "idêntico ao da DLL). Um modelo para Native..Performance e outro para Ultra Performance. "
    "Recursos: tools/fsr4cap/build_assets.sh (requer a DLL e o Proton).",
    "FSR 4.1.1 w trybie INT8: model z biblioteki AMD 4.1.1 odtworzony w Vulkanie (wynik zgodny "
    "z DLL). Jeden model dla Native..Performance i osobny dla Ultra Performance. Zasoby: "
    "tools/fsr4cap/build_assets.sh (wymaga DLL i Protona).",
    "INT8 modunda FSR 4.1.1: AMD'nin 4.1.1 DLL'indeki model, Vulkan'da yeniden üretildi (sonuç "
    "DLL ile aynı). Native..Performance için bir model, Ultra Performance için ayrı bir model. "
    "Varlıklar: tools/fsr4cap/build_assets.sh (DLL ve Proton gerekir).",
    "INT8 模式的 FSR 4.1.1：在 Vulkan 中复现 AMD 4.1.1 DLL 的模型（结果与 DLL 一致）。"
    "Native..Performance 共用一个模型，Ultra Performance 使用单独模型。资源："
    "tools/fsr4cap/build_assets.sh（需要 DLL 和 Proton）。",
    "INT8 モードの FSR 4.1.1：AMD 4.1.1 DLL のモデルを Vulkan で再現（結果は DLL と一致）。"
    "Native..Performance 用に 1 つ、Ultra Performance 用に別のモデル。アセット: "
    "tools/fsr4cap/build_assets.sh（DLL と Proton が必要）。",
    "INT8 모드 FSR 4.1.1: AMD 4.1.1 DLL의 모델을 Vulkan에서 재현했습니다(결과는 DLL과 동일). "
    "Native..Performance용 모델 하나와 Ultra Performance용 별도 모델. 에셋: "
    "tools/fsr4cap/build_assets.sh (DLL과 Proton 필요)."}},
{Fsr4Hint, {
    "FSR 4 in INT8 mode (model v07 from the AMD FidelityFX SDK sources). Higher quality than "
    "FSR 3.1, but a heavier pass. Changing the preset rebuilds the model (short pause). Assets: "
    "tools/fetch_fsr4_assets.sh.",
    "FSR 4 в режиме INT8 (модель v07 из исходников AMD FidelityFX SDK). Качество выше, "
    "чем у FSR 3.1, но проход тяжелее. Смена пресета пересобирает модель (короткая "
    "пауза). Ассеты: tools/fetch_fsr4_assets.sh.",
    "FSR 4 in modalità INT8 (modello v07 dai sorgenti AMD FidelityFX SDK). Qualità superiore a "
    "FSR 3.1, ma passaggio più pesante. Cambiare preset ricostruisce il modello (breve pausa). "
    "Asset: tools/fetch_fsr4_assets.sh.",
    "FSR 4 en modo INT8 (modelo v07 de las fuentes del AMD FidelityFX SDK). Más calidad que "
    "FSR 3.1, pero una pasada más pesada. Cambiar el preajuste reconstruye el modelo (breve "
    "pausa). Recursos: tools/fetch_fsr4_assets.sh.",
    "FSR 4 en mode INT8 (modèle v07 issu des sources de l'AMD FidelityFX SDK). Meilleure qualité "
    "que FSR 3.1, mais passe plus lourde. Changer de préréglage reconstruit le modèle (courte "
    "pause). Ressources : tools/fetch_fsr4_assets.sh.",
    "FSR 4 im INT8-Modus (Modell v07 aus den Quellen des AMD FidelityFX SDK). Bessere Qualität "
    "als FSR 3.1, aber ein aufwendigerer Durchgang. Ein Preset-Wechsel baut das Modell neu "
    "(kurze Pause). Assets: tools/fetch_fsr4_assets.sh.",
    "FSR 4 em modo INT8 (modelo v07 do código do AMD FidelityFX SDK). Qualidade superior ao "
    "FSR 3.1, mas passagem mais pesada. Mudar a predefinição reconstrói o modelo (pausa curta). "
    "Recursos: tools/fetch_fsr4_assets.sh.",
    "FSR 4 w trybie INT8 (model v07 ze źródeł AMD FidelityFX SDK). Lepsza jakość niż FSR 3.1, "
    "ale cięższy przebieg. Zmiana presetu przebudowuje model (krótka pauza). Zasoby: "
    "tools/fetch_fsr4_assets.sh.",
    "INT8 modunda FSR 4 (AMD FidelityFX SDK kaynaklarından v07 modeli). FSR 3.1'den daha "
    "kaliteli ama daha ağır bir geçiş. Ön ayar değişikliği modeli yeniden oluşturur (kısa "
    "duraklama). Varlıklar: tools/fetch_fsr4_assets.sh.",
    "INT8 模式的 FSR 4（来自 AMD FidelityFX SDK 源码的 v07 模型）。画质高于 FSR 3.1，但开销更大。"
    "切换预设会重建模型（短暂停顿）。资源：tools/fetch_fsr4_assets.sh。",
    "INT8 モードの FSR 4（AMD FidelityFX SDK ソースの v07 モデル）。FSR 3.1 より高画質ですが負荷が"
    "大きくなります。プリセットを変更するとモデルを再構築します（短い停止）。アセット: "
    "tools/fetch_fsr4_assets.sh。",
    "INT8 모드 FSR 4(AMD FidelityFX SDK 소스의 v07 모델). FSR 3.1보다 화질이 좋지만 부하가 "
    "큽니다. 프리셋을 바꾸면 모델을 다시 구성합니다(짧은 멈춤). 에셋: tools/fetch_fsr4_assets.sh."}},
{Fsr4AutoExposure, {
    "FSR 4: auto exposure", "FSR 4: авто-экспозиция", "FSR 4: auto-esposizione",
    "FSR 4: exposición automática", "FSR 4 : exposition automatique",
    "FSR 4: automatische Belichtung", "FSR 4: exposição automática",
    "FSR 4: automatyczna ekspozycja", "FSR 4: otomatik pozlama", "FSR 4：自动曝光",
    "FSR 4: 自動露出", "FSR 4: 자동 노출"}},
{Fsr4InvertJitter, {
    "FSR 4: invert jitter sign", "FSR 4: обратный знак jitter", "FSR 4: segno jitter invertito",
    "FSR 4: invertir signo del jitter", "FSR 4 : inverser le signe du jitter",
    "FSR 4: Jitter-Vorzeichen umkehren", "FSR 4: inverter sinal do jitter",
    "FSR 4: odwróć znak jittera", "FSR 4: jitter işaretini ters çevir", "FSR 4：反转抖动符号",
    "FSR 4: ジッターの符号を反転", "FSR 4: 지터 부호 반전"}},
{Fsr4CheckHint, {
    "For ghosting checks: the FSR 4 network normalizes color by exposure and uses it to decide "
    "when to discard past frames. Applied immediately, no restart.",
    "Проверка при гостинге: сеть FSR 4 нормирует цвет по экспозиции и по ней решает, "
    "когда отбросить прошлые кадры. Меняются сразу, без перезапуска.",
    "Da provare in caso di ghosting: la rete FSR 4 normalizza il colore in base all'esposizione e "
    "la usa per decidere quando scartare i frame precedenti. Effetto immediato, senza riavvio.",
    "Para comprobar el ghosting: la red de FSR 4 normaliza el color según la exposición y la usa "
    "para decidir cuándo descartar fotogramas anteriores. Se aplica al instante, sin reiniciar.",
    "Pour tester le ghosting : le réseau FSR 4 normalise la couleur selon l'exposition et s'en "
    "sert pour décider quand écarter les images précédentes. Effet immédiat, sans redémarrage.",
    "Zum Prüfen von Ghosting: Das FSR-4-Netz normalisiert die Farbe anhand der Belichtung und "
    "entscheidet damit, wann frühere Bilder verworfen werden. Wirkt sofort, ohne Neustart.",
    "Para testar ghosting: a rede do FSR 4 normaliza a cor pela exposição e a usa para decidir "
    "quando descartar quadros anteriores. Aplicado na hora, sem reiniciar.",
    "Do sprawdzania smużenia: sieć FSR 4 normalizuje kolor według ekspozycji i na tej podstawie "
    "decyduje, kiedy odrzucić poprzednie klatki. Działa od razu, bez restartu.",
    "Gölgelenme (ghosting) testi için: FSR 4 ağı rengi pozlamaya göre normalleştirir ve önceki "
    "kareleri ne zaman atacağına buna göre karar verir. Yeniden başlatmadan hemen uygulanır.",
    "用于排查拖影：FSR 4 网络按曝光归一化颜色，并据此决定何时丢弃历史帧。立即生效，无需重启。",
    "ゴースト確認用：FSR 4 のネットワークは露出で色を正規化し、それを基に過去フレームを破棄する"
    "タイミングを決めます。再起動なしで即座に反映。",
    "고스팅 확인용: FSR 4 네트워크는 노출로 색상을 정규화하고 이를 기준으로 이전 프레임을 버릴 "
    "시점을 정합니다. 재시작 없이 즉시 적용됩니다."}},
{Preset, {
    "Preset", "Пресет", "Preset", "Preajuste", "Préréglage", "Preset", "Predefinição", "Preset",
    "Ön ayar", "预设", "プリセット", "프리셋"}},
{PresetItem, {
    "%s (x%.1f, render %dx%d)", "%s (x%.1f, рендер %dx%d)", "%s (x%.1f, render %dx%d)",
    "%s (x%.1f, render %dx%d)", "%s (x%.1f, rendu %dx%d)", "%s (x%.1f, Render %dx%d)",
    "%s (x%.1f, render %dx%d)", "%s (x%.1f, render %dx%d)", "%s (x%.1f, işleme %dx%d)",
    "%s（x%.1f，渲染 %dx%d）", "%s（x%.1f、レンダー %dx%d）", "%s (x%.1f, 렌더 %dx%d)"}},
{TaaText, {
    "TAA smooths the scene at the output resolution, without an FSR model or upscaling. The "
    "saved FSR preset returns when FSR is selected.",
    "TAA сглаживает сцену в разрешении вывода, без модели FSR и апскейлинга. "
    "Сохранённый пресет FSR восстановится при выборе FSR.",
    "TAA leviga la scena alla risoluzione di output, senza modello FSR né upscaling. Il preset "
    "FSR salvato verrà ripristinato scegliendo FSR.",
    "TAA suaviza la escena a la resolución de salida, sin modelo FSR ni escalado. El preajuste "
    "de FSR guardado vuelve al elegir FSR.",
    "TAA lisse la scène à la résolution de sortie, sans modèle FSR ni upscaling. Le préréglage "
    "FSR enregistré revient en choisissant FSR.",
    "TAA glättet die Szene in der Ausgabeauflösung, ohne FSR-Modell und ohne Upscaling. Das "
    "gespeicherte FSR-Preset kehrt bei Auswahl von FSR zurück.",
    "O TAA suaviza a cena na resolução de saída, sem modelo FSR nem upscaling. A predefinição do "
    "FSR salva volta ao escolher o FSR.",
    "TAA wygładza scenę w rozdzielczości wyjściowej, bez modelu FSR i bez skalowania. Zapisany "
    "preset FSR wróci po wybraniu FSR.",
    "TAA, sahneyi FSR modeli ve ölçekleme olmadan çıkış çözünürlüğünde yumuşatır. Kayıtlı FSR ön "
    "ayarı, FSR seçildiğinde geri gelir.",
    "TAA 以输出分辨率平滑画面，不使用 FSR 模型，也不做超分辨率。选择 FSR 时会恢复已保存的 FSR 预设。",
    "TAA は出力解像度でシーンを平滑化し、FSR モデルやアップスケーリングは使いません。FSR を選ぶと"
    "保存済みの FSR プリセットに戻ります。",
    "TAA는 FSR 모델이나 업스케일링 없이 출력 해상도에서 장면을 부드럽게 합니다. FSR을 선택하면 "
    "저장된 FSR 프리셋이 복원됩니다."}},
{ActiveRender, {
    "Active scene render: %d x %d", "Активный рендер сцены: %d x %d",
    "Render scena attivo: %d x %d", "Render de escena activo: %d x %d",
    "Rendu de scène actif : %d x %d", "Aktive Szenenauflösung: %d x %d",
    "Render de cena ativo: %d x %d", "Aktywny render sceny: %d x %d",
    "Etkin sahne işleme: %d x %d", "当前场景渲染：%d x %d", "現在のシーン描画: %d x %d",
    "현재 장면 렌더: %d x %d"}},
{StartupPreset, {
    "Startup preset: %s", "Пресет при запуске: %s", "Preset all'avvio: %s",
    "Preajuste al iniciar: %s", "Préréglage au démarrage : %s", "Preset beim Start: %s",
    "Predefinição ao iniciar: %s", "Preset przy starcie: %s", "Başlangıç ön ayarı: %s",
    "启动时预设：%s", "起動時のプリセット: %s", "시작 시 프리셋: %s"}},
{AutoRenderHint, {
    "With an output other than 1080p the whole game is drawn at the preset resolution (patched "
    "at start): fastest on the Steam Deck and weak GPUs. Changing the preset or the output "
    "resolution needs a restart. «Live resolution change» below allows changes without a "
    "restart (post-processing then stays at 1080p, slower).",
    "При выводе не 1080p вся игра рисуется в разрешении пресета (патч при запуске): "
    "это быстрее всего на Steam Deck и слабых GPU. Смена пресета или разрешения "
    "вывода — после перезапуска. Пункт «Смена разрешения на лету» ниже включает "
    "смену без перезапуска (постобработка тогда остаётся в 1080p, медленнее).",
    "Con output diverso da 1080p tutto il gioco è disegnato alla risoluzione del preset (patch "
    "all'avvio): è la scelta più veloce su Steam Deck e GPU deboli. Cambiare preset o "
    "risoluzione di output richiede il riavvio. L'opzione «Cambio risoluzione al volo» qui sotto "
    "permette di cambiarli senza riavvio (il post-processing resta a 1080p, più lento).",
    "Con una salida distinta de 1080p todo el juego se dibuja a la resolución del preajuste "
    "(parche al iniciar): lo más rápido en Steam Deck y GPU modestas. Cambiar el preajuste o la "
    "resolución de salida requiere reiniciar. «Cambio de resolución en vivo», más abajo, permite "
    "cambiarlos sin reiniciar (el posprocesado queda entonces en 1080p, más lento).",
    "Avec une sortie autre que 1080p, tout le jeu est rendu à la résolution du préréglage (patch "
    "au démarrage) : le plus rapide sur Steam Deck et les GPU modestes. Changer de préréglage ou "
    "de résolution de sortie nécessite un redémarrage. « Changement de résolution à la volée », "
    "plus bas, permet de les changer sans redémarrer (le post-traitement reste alors en 1080p, "
    "plus lent).",
    "Bei einer anderen Ausgabe als 1080p wird das ganze Spiel in der Preset-Auflösung gezeichnet "
    "(Patch beim Start): am schnellsten auf dem Steam Deck und schwachen GPUs. Preset- oder "
    "Ausgabeauflösungswechsel erfordern einen Neustart. „Auflösung live ändern“ unten erlaubt "
    "Wechsel ohne Neustart (die Nachbearbeitung bleibt dann bei 1080p, langsamer).",
    "Com saída diferente de 1080p, o jogo inteiro é desenhado na resolução da predefinição (patch "
    "ao iniciar): o mais rápido no Steam Deck e em GPUs fracas. Mudar a predefinição ou a "
    "resolução de saída exige reiniciar. «Mudança de resolução ao vivo», abaixo, permite mudar "
    "sem reiniciar (o pós-processamento fica então em 1080p, mais lento).",
    "Przy wyjściu innym niż 1080p cała gra jest rysowana w rozdzielczości presetu (łatka przy "
    "starcie): najszybciej na Steam Deck i słabych GPU. Zmiana presetu lub rozdzielczości "
    "wyjściowej wymaga restartu. Opcja „Zmiana rozdzielczości na żywo” poniżej pozwala zmieniać "
    "je bez restartu (postprocessing zostaje wtedy w 1080p, wolniej).",
    "1080p dışındaki bir çıkışta oyunun tamamı ön ayar çözünürlüğünde çizilir (başlangıçta "
    "yama): Steam Deck ve zayıf GPU'larda en hızlısıdır. Ön ayar veya çıkış çözünürlüğü "
    "değişikliği yeniden başlatma gerektirir. Aşağıdaki «Anında çözünürlük değişimi» yeniden "
    "başlatmadan değiştirmeye izin verir (son işleme 1080p'de kalır, daha yavaş).",
    "输出不是 1080p 时，整个游戏按预设分辨率绘制（启动时打补丁）：在 Steam Deck 和较弱显卡上最快。"
    "更改预设或输出分辨率需要重启。下方的「实时更改分辨率」可免重启更改（此时后期处理保持 1080p，较慢）。",
    "出力が 1080p 以外のとき、ゲーム全体をプリセットの解像度で描画します（起動時パッチ）："
    "Steam Deck や非力な GPU で最速。プリセットや出力解像度の変更には再起動が必要です。下の"
    "「解像度のライブ変更」で再起動なしに変更できます（その場合ポストプロセスは 1080p のままで、"
    "遅くなります）。",
    "출력이 1080p가 아니면 게임 전체를 프리셋 해상도로 그립니다(시작 시 패치): Steam Deck과 저사양 "
    "GPU에서 가장 빠릅니다. 프리셋이나 출력 해상도를 바꾸려면 재시작해야 합니다. 아래의 「실시간 "
    "해상도 변경」을 켜면 재시작 없이 바꿀 수 있습니다(이때 후처리는 1080p로 유지되어 더 느림)."}},
{RenderResHint, {
    "BB_RENDER_RES fixes the scene size at start. Remove this explicit variable to change "
    "resolution and presets without restarting the game.",
    "BB_RENDER_RES фиксирует размер сцены при запуске. Уберите эту явную переменную "
    "для смены разрешения и пресетов без перезапуска игры.",
    "BB_RENDER_RES fissa la dimensione della scena all'avvio. Rimuovi questa variabile per "
    "cambiare risoluzione e preset senza riavviare il gioco.",
    "BB_RENDER_RES fija el tamaño de la escena al iniciar. Quita esta variable para cambiar "
    "resolución y preajustes sin reiniciar el juego.",
    "BB_RENDER_RES fixe la taille de la scène au démarrage. Supprimez cette variable pour changer "
    "de résolution et de préréglage sans redémarrer le jeu.",
    "BB_RENDER_RES legt die Szenengröße beim Start fest. Entferne diese Variable, um Auflösung "
    "und Presets ohne Neustart zu ändern.",
    "BB_RENDER_RES fixa o tamanho da cena ao iniciar. Remova essa variável para mudar resolução "
    "e predefinições sem reiniciar o jogo.",
    "BB_RENDER_RES ustala rozmiar sceny przy starcie. Usuń tę zmienną, aby zmieniać "
    "rozdzielczość i presety bez restartu gry.",
    "BB_RENDER_RES sahne boyutunu başlangıçta sabitler. Çözünürlüğü ve ön ayarları oyunu "
    "yeniden başlatmadan değiştirmek için bu değişkeni kaldırın.",
    "BB_RENDER_RES 会在启动时固定场景尺寸。移除该变量即可在不重启游戏的情况下更改分辨率和预设。",
    "BB_RENDER_RES は起動時にシーンサイズを固定します。この変数を外すと、ゲームを再起動せずに"
    "解像度やプリセットを変更できます。",
    "BB_RENDER_RES는 시작 시 장면 크기를 고정합니다. 이 변수를 제거하면 게임을 재시작하지 않고 "
    "해상도와 프리셋을 바꿀 수 있습니다."}},
{NativeAaHint, {
    "Native AA: the upscaler works as anti-aliasing. The other presets lower the scene's render "
    "resolution relative to the output. The interface is drawn at the output resolution. The "
    "preset applies from the next frame without restarting the game.",
    "Native AA: FSR работает как сглаживание. Остальные пресеты уменьшают разрешение "
    "отрисовки сцены относительно вывода. Интерфейс рисуется в разрешении вывода. "
    "Пресет применяется со следующего кадра без перезапуска игры.",
    "Native AA: l'upscaler funziona come antialiasing. Gli altri preset riducono la risoluzione "
    "di rendering della scena rispetto all'output. L'interfaccia è disegnata alla risoluzione di "
    "output. Il preset si applica dal frame successivo senza riavviare il gioco.",
    "Native AA: el escalador funciona como antialiasing. Los demás preajustes reducen la "
    "resolución de render de la escena respecto a la salida. La interfaz se dibuja a la "
    "resolución de salida. El preajuste se aplica desde el fotograma siguiente sin reiniciar.",
    "Native AA : l'upscaler sert d'anticrénelage. Les autres préréglages réduisent la résolution "
    "de rendu de la scène par rapport à la sortie. L'interface est rendue à la résolution de "
    "sortie. Le préréglage s'applique dès l'image suivante, sans redémarrage.",
    "Native AA: Der Upscaler arbeitet als Kantenglättung. Die übrigen Presets senken die "
    "Renderauflösung der Szene gegenüber der Ausgabe. Die Oberfläche wird in der "
    "Ausgabeauflösung gezeichnet. Das Preset wirkt ab dem nächsten Bild ohne Neustart.",
    "Native AA: o upscaler funciona como anti-aliasing. As outras predefinições reduzem a "
    "resolução de renderização da cena em relação à saída. A interface é desenhada na resolução "
    "de saída. A predefinição vale a partir do quadro seguinte, sem reiniciar.",
    "Native AA: upscaler działa jako antyaliasing. Pozostałe presety obniżają rozdzielczość "
    "renderowania sceny względem wyjścia. Interfejs jest rysowany w rozdzielczości wyjściowej. "
    "Preset działa od następnej klatki, bez restartu gry.",
    "Native AA: ölçekleyici kenar yumuşatma olarak çalışır. Diğer ön ayarlar sahnenin işleme "
    "çözünürlüğünü çıkışa göre düşürür. Arayüz çıkış çözünürlüğünde çizilir. Ön ayar, oyunu "
    "yeniden başlatmadan bir sonraki karede uygulanır.",
    "Native AA：超分辨率器仅用作抗锯齿。其余预设会相对输出降低场景渲染分辨率。界面按输出分辨率绘制。"
    "预设从下一帧起生效，无需重启游戏。",
    "Native AA：アップスケーラーをアンチエイリアスとして使います。その他のプリセットは出力に対して"
    "シーンの描画解像度を下げます。UI は出力解像度で描画されます。プリセットは再起動なしで次の"
    "フレームから反映されます。",
    "Native AA: 업스케일러가 안티에일리어싱으로 동작합니다. 나머지 프리셋은 출력 대비 장면 렌더 "
    "해상도를 낮춥니다. UI는 출력 해상도로 그려집니다. 프리셋은 재시작 없이 다음 프레임부터 "
    "적용됩니다."}},
{Sharpen, {
    "Sharpening (RCAS)", "Резкость (RCAS)", "Nitidezza (RCAS)", "Nitidez (RCAS)",
    "Netteté (RCAS)", "Schärfe (RCAS)", "Nitidez (RCAS)", "Wyostrzanie (RCAS)",
    "Keskinleştirme (RCAS)", "锐化（RCAS）", "シャープ化 (RCAS)", "선명화 (RCAS)"}},
{Sharpness, {
    "Sharpness strength", "Сила резкости", "Intensità nitidezza", "Intensidad de nitidez",
    "Intensité de netteté", "Schärfestärke", "Intensidade da nitidez", "Siła wyostrzania",
    "Keskinlik gücü", "锐化强度", "シャープ化の強さ", "선명화 강도"}},
{SharpnessHint, {
    "Up to 1: the upscaler's own sharpening (RCAS). Above 1 another RCAS pass is added. "
    "Ctrl+click the slider to type an exact value.",
    "До 1 — резкость самого апскейлера (RCAS). Выше 1 добавляется ещё один проход RCAS. "
    "Ctrl+клик по ползунку — ввести точное значение.",
    "Fino a 1: nitidezza dell'upscaler stesso (RCAS). Oltre 1 si aggiunge un altro passaggio "
    "RCAS. Ctrl+clic sul cursore per inserire un valore esatto.",
    "Hasta 1: la nitidez del propio escalador (RCAS). Por encima de 1 se añade otra pasada RCAS. "
    "Ctrl+clic en el control para escribir un valor exacto.",
    "Jusqu'à 1 : la netteté propre à l'upscaler (RCAS). Au-delà de 1, une passe RCAS "
    "supplémentaire est ajoutée. Ctrl+clic sur le curseur pour saisir une valeur exacte.",
    "Bis 1: die Schärfung des Upscalers selbst (RCAS). Über 1 kommt ein weiterer "
    "RCAS-Durchgang hinzu. Strg+Klick auf den Regler, um einen genauen Wert einzugeben.",
    "Até 1: a nitidez do próprio upscaler (RCAS). Acima de 1 é adicionada outra passagem RCAS. "
    "Ctrl+clique no controle para digitar um valor exato.",
    "Do 1: wyostrzanie samego upscalera (RCAS). Powyżej 1 dodawany jest kolejny przebieg RCAS. "
    "Ctrl+klik na suwaku, aby wpisać dokładną wartość.",
    "1'e kadar: ölçekleyicinin kendi keskinleştirmesi (RCAS). 1'in üzerinde bir RCAS geçişi "
    "daha eklenir. Tam değer girmek için kaydırıcıya Ctrl+tıklayın.",
    "不超过 1：超分辨率器自带的锐化（RCAS）。大于 1 时额外执行一次 RCAS。按住 Ctrl 单击滑块可输入精确数值。",
    "1 まで：アップスケーラー自身のシャープ化（RCAS）。1 を超えると RCAS パスを追加します。"
    "Ctrl+クリックで正確な値を入力できます。",
    "1까지: 업스케일러 자체의 선명화(RCAS). 1을 넘으면 RCAS 패스가 하나 더 추가됩니다. 슬라이더를 "
    "Ctrl+클릭하면 정확한 값을 입력할 수 있습니다."}},
{Jitter, {
    "Sub-pixel jitter", "Субпиксельный сдвиг (jitter)", "Spostamento subpixel (jitter)",
    "Desplazamiento subpíxel (jitter)", "Décalage sous-pixel (jitter)",
    "Subpixel-Versatz (Jitter)", "Deslocamento subpixel (jitter)",
    "Przesunięcie subpikselowe (jitter)", "Alt piksel kaydırma (jitter)", "亚像素抖动（jitter）",
    "サブピクセル ジッター", "서브픽셀 지터"}},
{JitterHint, {
    "Each frame the scene shifts by a fraction of a pixel and the upscaler gathers more detail "
    "from several frames. Without it you only get history-based smoothing.",
    "Каждый кадр сцена сдвигается на долю пикселя, и апскейлер собирает из нескольких "
    "кадров больше деталей. Без него получается только сглаживание по истории.",
    "Ogni frame la scena viene spostata di una frazione di pixel e l'upscaler ricava più "
    "dettagli da più frame. Senza, si ottiene solo l'antialiasing basato sulla cronologia.",
    "Cada fotograma la escena se desplaza una fracción de píxel y el escalador reúne más detalle "
    "de varios fotogramas. Sin él solo se obtiene suavizado por historial.",
    "À chaque image, la scène se décale d'une fraction de pixel et l'upscaler rassemble plus de "
    "détails sur plusieurs images. Sans cela, on n'obtient qu'un lissage par historique.",
    "Pro Bild wird die Szene um einen Bruchteil eines Pixels verschoben, und der Upscaler "
    "sammelt aus mehreren Bildern mehr Details. Ohne ihn gibt es nur eine Glättung über den "
    "Verlauf.",
    "A cada quadro a cena se desloca uma fração de pixel e o upscaler reúne mais detalhes de "
    "vários quadros. Sem isso, há apenas suavização pelo histórico.",
    "W każdej klatce scena przesuwa się o ułamek piksela, a upscaler zbiera więcej szczegółów z "
    "kilku klatek. Bez tego jest tylko wygładzanie na podstawie historii.",
    "Her karede sahne bir pikselin kesri kadar kaydırılır ve ölçekleyici birkaç kareden daha "
    "fazla ayrıntı toplar. Bu olmadan yalnızca geçmişe dayalı yumuşatma elde edilir.",
    "每帧将画面偏移不足一个像素，超分辨率器从多帧中收集更多细节。关闭时只有基于历史帧的平滑。",
    "毎フレーム、シーンを 1 ピクセル未満ずらし、アップスケーラーが複数フレームからより多くの"
    "ディテールを集めます。オフの場合は履歴による平滑化のみになります。",
    "매 프레임 장면을 픽셀 일부만큼 이동시키고, 업스케일러가 여러 프레임에서 더 많은 디테일을 "
    "모읍니다. 끄면 히스토리 기반 스무딩만 적용됩니다."}},
{SecReactive, {
    "Reactive mask", "Маска реактивности", "Maschera reattiva", "Máscara reactiva",
    "Masque réactif", "Reaktive Maske", "Máscara reativa", "Maska reaktywna", "Reaktif maske",
    "反应遮罩", "リアクティブ マスク", "리액티브 마스크"}},
{ReactiveOn, {
    "Enable mask", "Включить маску", "Attiva maschera", "Activar máscara", "Activer le masque",
    "Maske aktivieren", "Ativar máscara", "Włącz maskę", "Maskeyi etkinleştir", "启用遮罩",
    "マスクを有効化", "마스크 사용"}},
{ReactiveHint, {
    "Marks transparent effects (particles, haze) so the upscaler relies less on past frames. "
    "Fewer trails behind effects, but shimmer returns under them.",
    "Помечает прозрачные эффекты (частицы, дымку), чтобы апскейлер меньше опирался на "
    "прошлые кадры. Меньше шлейфов за эффектами, но под ними возвращается дрожание.",
    "Marca gli effetti trasparenti (particelle, foschia) perché l'upscaler si basi meno sui "
    "frame precedenti. Meno scie dietro gli effetti, ma sotto di essi torna il tremolio.",
    "Marca los efectos transparentes (partículas, neblina) para que el escalador dependa menos "
    "de los fotogramas anteriores. Menos estelas tras los efectos, pero vuelve el temblor bajo "
    "ellos.",
    "Marque les effets transparents (particules, brume) pour que l'upscaler s'appuie moins sur "
    "les images précédentes. Moins de traînées derrière les effets, mais le scintillement "
    "revient dessous.",
    "Markiert transparente Effekte (Partikel, Dunst), damit sich der Upscaler weniger auf "
    "frühere Bilder stützt. Weniger Schlieren hinter Effekten, darunter kehrt aber das Flimmern "
    "zurück.",
    "Marca efeitos transparentes (partículas, névoa) para que o upscaler dependa menos dos "
    "quadros anteriores. Menos rastros atrás dos efeitos, mas o tremor volta sob eles.",
    "Oznacza efekty przezroczyste (cząsteczki, mgiełkę), aby upscaler mniej polegał na "
    "poprzednich klatkach. Mniej smug za efektami, ale pod nimi wraca migotanie.",
    "Saydam efektleri (parçacıklar, pus) işaretler; böylece ölçekleyici önceki karelere daha az "
    "dayanır. Efektlerin arkasında daha az iz kalır ama altlarında titreme geri gelir.",
    "标记透明效果（粒子、薄雾），让超分辨率器少依赖历史帧。效果后方的拖尾减少，但其下方的闪烁会重新出现。",
    "半透明エフェクト（パーティクル、もや）をマークし、アップスケーラーが過去フレームに頼りすぎない"
    "ようにします。エフェクトの残像は減りますが、その下のちらつきが戻ります。",
    "반투명 효과(파티클, 안개)를 표시해 업스케일러가 이전 프레임에 덜 의존하게 합니다. 효과 뒤의 "
    "잔상은 줄지만 그 아래에서 떨림이 다시 생깁니다."}},
{Scale, {
    "Scale", "Масштаб", "Scala", "Escala", "Échelle", "Skalierung", "Escala", "Skala", "Ölçek",
    "缩放", "スケール", "배율"}},
{Threshold, {
    "Threshold", "Порог", "Soglia", "Umbral", "Seuil", "Schwelle", "Limiar", "Próg", "Eşik",
    "阈值", "しきい値", "임계값"}},
{Maximum, {
    "Maximum", "Максимум", "Massimo", "Máximo", "Maximum", "Maximum", "Máximo", "Maksimum",
    "Maksimum", "最大值", "最大", "최대"}},
{ShowMask, {
    "Show mask (debug)", "Показать маску (отладка)", "Mostra maschera (debug)",
    "Mostrar máscara (depuración)", "Afficher le masque (débogage)", "Maske anzeigen (Debug)",
    "Mostrar máscara (depuração)", "Pokaż maskę (debug)", "Maskeyi göster (hata ayıklama)",
    "显示遮罩（调试）", "マスクを表示（デバッグ）", "마스크 표시 (디버그)"}},
{ObjectMotion, {
    "Character motion vectors", "Векторы движения персонажей", "Vettori di movimento personaggi",
    "Vectores de movimiento de personajes", "Vecteurs de mouvement des personnages",
    "Bewegungsvektoren der Figuren", "Vetores de movimento dos personagens",
    "Wektory ruchu postaci", "Karakter hareket vektörleri", "角色运动矢量",
    "キャラクターのモーションベクター", "캐릭터 모션 벡터"}},
{ObjectMotionHint, {
    "Exact vectors for animated objects: clothes and weapons break up less in motion. The static "
    "scene gets no extra pass. Applies after restarting the game.",
    "Точные векторы для анимированных объектов: одежда и оружие меньше рассыпаются "
    "при движении. Статичная сцена не получает дополнительный проход. "
    "Изменение применяется после перезапуска игры.",
    "Vettori precisi per gli oggetti animati: vestiti e armi si sfaldano meno in movimento. La "
    "scena statica non riceve un passaggio aggiuntivo. La modifica si applica dopo il riavvio "
    "del gioco.",
    "Vectores exactos para objetos animados: la ropa y las armas se deshacen menos en "
    "movimiento. La escena estática no recibe una pasada extra. Se aplica tras reiniciar el "
    "juego.",
    "Vecteurs exacts pour les objets animés : vêtements et armes se désagrègent moins en "
    "mouvement. La scène statique ne reçoit pas de passe supplémentaire. S'applique après "
    "redémarrage du jeu.",
    "Genaue Vektoren für animierte Objekte: Kleidung und Waffen zerfallen in Bewegung weniger. "
    "Die statische Szene bekommt keinen zusätzlichen Durchgang. Wirkt nach einem Neustart des "
    "Spiels.",
    "Vetores exatos para objetos animados: roupas e armas se desfazem menos em movimento. A cena "
    "estática não recebe passagem extra. Aplica-se após reiniciar o jogo.",
    "Dokładne wektory dla animowanych obiektów: ubrania i broń mniej się rozpadają w ruchu. "
    "Statyczna scena nie dostaje dodatkowego przebiegu. Działa po restarcie gry.",
    "Hareketli nesneler için kesin vektörler: kıyafetler ve silahlar hareket halinde daha az "
    "dağılır. Statik sahne ek bir geçiş almaz. Oyun yeniden başlatıldıktan sonra uygulanır.",
    "为动画物体提供精确矢量：衣物和武器在运动中更少破碎。静态场景不会增加额外处理。重启游戏后生效。",
    "アニメーションするオブジェクトに正確なベクターを使用：服や武器が動いたときに崩れにくくなります。"
    "静的なシーンには追加パスはありません。ゲーム再起動後に反映。",
    "애니메이션 오브젝트용 정확한 벡터: 옷과 무기가 움직일 때 덜 깨집니다. 정적인 장면에는 추가 "
    "패스가 없습니다. 게임을 재시작하면 적용됩니다."}},
{ShowMotion, {
    "Show motion vectors (debug)", "Показать векторы движения (отладка)",
    "Mostra vettori di movimento (debug)", "Mostrar vectores de movimiento (depuración)",
    "Afficher les vecteurs de mouvement (débogage)", "Bewegungsvektoren anzeigen (Debug)",
    "Mostrar vetores de movimento (depuração)", "Pokaż wektory ruchu (debug)",
    "Hareket vektörlerini göster (hata ayıklama)", "显示运动矢量（调试）",
    "モーションベクターを表示（デバッグ）", "모션 벡터 표시 (디버그)"}},
{MotionHint, {
    "Red/green: horizontal/vertical motion (8 pixels = full brightness). Blue: the pixel got an "
    "exact object vector, not just camera motion. A moving object without blue and without "
    "red/green is treated as still by the upscaler, hence the trail.",
    "Красный/зелёный: движение по горизонтали/вертикали (8 пикселей = полная яркость). "
    "Синий: пиксель получил точный вектор объекта, а не только движение камеры. "
    "Движущийся предмет без синего и без красного/зелёного апскейлер считает "
    "неподвижным, отсюда шлейф.",
    "Rosso/verde: movimento orizzontale/verticale (8 pixel = luminosità piena). Blu: il pixel "
    "ha ricevuto il vettore preciso dell'oggetto, non solo il movimento della camera. Un oggetto "
    "in movimento senza blu né rosso/verde viene considerato fermo dall'upscaler, da qui la scia.",
    "Rojo/verde: movimiento horizontal/vertical (8 píxeles = brillo máximo). Azul: el píxel "
    "recibió el vector exacto del objeto, no solo el movimiento de la cámara. Un objeto en "
    "movimiento sin azul ni rojo/verde se considera quieto, de ahí la estela.",
    "Rouge/vert : mouvement horizontal/vertical (8 pixels = pleine luminosité). Bleu : le pixel a "
    "reçu le vecteur exact de l'objet, pas seulement le mouvement de la caméra. Un objet mobile "
    "sans bleu ni rouge/vert est considéré immobile par l'upscaler, d'où la traînée.",
    "Rot/Grün: horizontale/vertikale Bewegung (8 Pixel = volle Helligkeit). Blau: Das Pixel hat "
    "den genauen Objektvektor erhalten, nicht nur die Kamerabewegung. Ein bewegtes Objekt ohne "
    "Blau und ohne Rot/Grün gilt für den Upscaler als ruhend, daher die Schliere.",
    "Vermelho/verde: movimento horizontal/vertical (8 pixels = brilho máximo). Azul: o pixel "
    "recebeu o vetor exato do objeto, não só o movimento da câmera. Um objeto em movimento sem "
    "azul nem vermelho/verde é tratado como parado pelo upscaler, daí o rastro.",
    "Czerwony/zielony: ruch poziomy/pionowy (8 pikseli = pełna jasność). Niebieski: piksel "
    "dostał dokładny wektor obiektu, a nie tylko ruch kamery. Poruszający się obiekt bez "
    "niebieskiego i bez czerwonego/zielonego upscaler uznaje za nieruchomy, stąd smuga.",
    "Kırmızı/yeşil: yatay/dikey hareket (8 piksel = tam parlaklık). Mavi: piksel yalnızca kamera "
    "hareketini değil, nesnenin kesin vektörünü aldı. Mavisi ve kırmızı/yeşili olmayan hareketli "
    "bir nesneyi ölçekleyici sabit sayar; iz bu yüzden oluşur.",
    "红/绿：水平/垂直运动（8 像素 = 最亮）。蓝：该像素获得了物体的精确矢量，而不仅是镜头运动。"
    "运动中的物体若既无蓝色也无红/绿，会被超分辨率器视为静止，从而产生拖尾。",
    "赤/緑：水平/垂直の動き（8 ピクセル = 最大輝度）。青：カメラの動きだけでなく、オブジェクトの"
    "正確なベクターを得たピクセル。青も赤/緑もない動く物体はアップスケーラーに静止と見なされ、"
    "残像の原因になります。",
    "빨강/초록: 가로/세로 움직임(8픽셀 = 최대 밝기). 파랑: 카메라 움직임뿐 아니라 오브젝트의 정확한 "
    "벡터를 받은 픽셀. 파랑도 빨강/초록도 없는 움직이는 물체는 업스케일러가 정지한 것으로 보아 "
    "잔상이 생깁니다."}},
{SecOutput, {
    "Output resolution", "Разрешение вывода", "Risoluzione di output", "Resolución de salida",
    "Résolution de sortie", "Ausgabeauflösung", "Resolução de saída", "Rozdzielczość wyjściowa",
    "Çıkış çözünürlüğü", "输出分辨率", "出力解像度", "출력 해상도"}},
{OutputRes, {
    "Output resolution", "Разрешение вывода", "Risoluzione di output", "Resolución de salida",
    "Résolution de sortie", "Ausgabeauflösung", "Resolução de saída", "Rozdzielczość wyjściowa",
    "Çıkış çözünürlüğü", "输出分辨率", "出力解像度", "출력 해상도"}},
{OutputHintFixed, {
    "Size of the final frame and interface. The preset sets the scene size relative to the "
    "output: 4K Performance = 1920x1080. Applies after restarting the game.",
    "Размер готового кадра и интерфейса. Пресет задаёт размер сцены относительно "
    "вывода: 4K Performance = 1920x1080. Применяется после перезапуска игры.",
    "Dimensione del frame finale e dell'interfaccia. Il preset definisce la dimensione della "
    "scena rispetto all'output: 4K Performance = 1920x1080. Si applica dopo il riavvio del gioco.",
    "Tamaño del fotograma final y de la interfaz. El preajuste fija el tamaño de la escena "
    "respecto a la salida: 4K Performance = 1920x1080. Se aplica tras reiniciar el juego.",
    "Taille de l'image finale et de l'interface. Le préréglage fixe la taille de la scène par "
    "rapport à la sortie : 4K Performance = 1920x1080. S'applique après redémarrage du jeu.",
    "Größe des fertigen Bildes und der Oberfläche. Das Preset bestimmt die Szenengröße relativ "
    "zur Ausgabe: 4K Performance = 1920x1080. Wirkt nach einem Neustart des Spiels.",
    "Tamanho do quadro final e da interface. A predefinição define o tamanho da cena em relação "
    "à saída: 4K Performance = 1920x1080. Aplica-se após reiniciar o jogo.",
    "Rozmiar gotowej klatki i interfejsu. Preset ustala rozmiar sceny względem wyjścia: "
    "4K Performance = 1920x1080. Działa po restarcie gry.",
    "Son karenin ve arayüzün boyutu. Ön ayar, sahne boyutunu çıkışa göre belirler: "
    "4K Performance = 1920x1080. Oyun yeniden başlatıldıktan sonra uygulanır.",
    "最终画面和界面的尺寸。预设决定场景相对输出的尺寸：4K Performance = 1920x1080。重启游戏后生效。",
    "最終フレームと UI のサイズ。プリセットは出力に対するシーンサイズを決めます："
    "4K Performance = 1920x1080。ゲーム再起動後に反映。",
    "최종 프레임과 UI의 크기. 프리셋이 출력 대비 장면 크기를 정합니다: 4K Performance = 1920x1080. "
    "게임을 재시작하면 적용됩니다."}},
{OutputHintLive, {
    "The size of the final frame and interface changes at the next frame boundary. The preset "
    "sets the scene size relative to the output: 4K Performance = 1920x1080. A size change "
    "resets the upscaler history and may cause a short pause.",
    "Размер готового кадра и интерфейса меняется на границе следующего кадра. "
    "Пресет задаёт размер сцены относительно вывода: 4K Performance = 1920x1080. "
    "Смена размера сбрасывает историю FSR и может вызвать короткую паузу.",
    "La dimensione del frame finale e dell'interfaccia cambia al frame successivo. Il preset "
    "definisce la dimensione della scena rispetto all'output: 4K Performance = 1920x1080. Il "
    "cambio azzera la cronologia dell'upscaler e può causare una breve pausa.",
    "El tamaño del fotograma final y de la interfaz cambia en el siguiente fotograma. El "
    "preajuste fija el tamaño de la escena respecto a la salida: 4K Performance = 1920x1080. El "
    "cambio reinicia el historial del escalador y puede causar una breve pausa.",
    "La taille de l'image finale et de l'interface change à l'image suivante. Le préréglage fixe "
    "la taille de la scène par rapport à la sortie : 4K Performance = 1920x1080. Le changement "
    "réinitialise l'historique de l'upscaler et peut causer une courte pause.",
    "Die Größe des fertigen Bildes und der Oberfläche ändert sich mit dem nächsten Bild. Das "
    "Preset bestimmt die Szenengröße relativ zur Ausgabe: 4K Performance = 1920x1080. Ein "
    "Wechsel setzt den Upscaler-Verlauf zurück und kann kurz pausieren.",
    "O tamanho do quadro final e da interface muda no quadro seguinte. A predefinição define o "
    "tamanho da cena em relação à saída: 4K Performance = 1920x1080. A mudança zera o histórico "
    "do upscaler e pode causar uma pausa curta.",
    "Rozmiar gotowej klatki i interfejsu zmienia się od następnej klatki. Preset ustala rozmiar "
    "sceny względem wyjścia: 4K Performance = 1920x1080. Zmiana resetuje historię upscalera i "
    "może wywołać krótką pauzę.",
    "Son karenin ve arayüzün boyutu bir sonraki karede değişir. Ön ayar, sahne boyutunu çıkışa "
    "göre belirler: 4K Performance = 1920x1080. Değişiklik ölçekleyici geçmişini sıfırlar ve "
    "kısa bir duraklamaya yol açabilir.",
    "最终画面和界面的尺寸会在下一帧切换。预设决定场景相对输出的尺寸：4K Performance = 1920x1080。"
    "更改会重置超分辨率历史，可能造成短暂停顿。",
    "最終フレームと UI のサイズは次のフレームで切り替わります。プリセットは出力に対するシーンサイズを"
    "決めます：4K Performance = 1920x1080。変更するとアップスケーラーの履歴がリセットされ、短く止まる"
    "ことがあります。",
    "최종 프레임과 UI 크기는 다음 프레임에 바뀝니다. 프리셋이 출력 대비 장면 크기를 정합니다: "
    "4K Performance = 1920x1080. 변경하면 업스케일러 히스토리가 초기화되어 잠깐 멈출 수 있습니다."}},
{LiveAuto, {
    "Auto (by GPU)", "Авто (по видеокарте)", "Auto (in base alla GPU)", "Auto (según la GPU)",
    "Auto (selon le GPU)", "Auto (nach GPU)", "Auto (pela GPU)", "Auto (wg GPU)",
    "Otomatik (GPU'ya göre)", "自动（按显卡）", "自動（GPU に応じて）", "자동 (GPU에 따라)"}},
{LiveOff, {
    "Off (faster)", "Выключена (быстрее)", "Disattivo (più veloce)", "Desactivado (más rápido)",
    "Désactivé (plus rapide)", "Aus (schneller)", "Desativado (mais rápido)",
    "Wyłączona (szybciej)", "Kapalı (daha hızlı)", "关闭（更快）", "オフ（高速）", "끔 (더 빠름)"}},
{LiveOn, {
    "On", "Включена", "Attivo", "Activado", "Activé", "An", "Ativado", "Włączona", "Açık", "开启",
    "オン", "켬"}},
{LiveRes, {
    "Live resolution change", "Смена разрешения на лету", "Cambio risoluzione al volo",
    "Cambio de resolución en vivo", "Changement de résolution à la volée",
    "Auflösung live ändern", "Mudança de resolução ao vivo", "Zmiana rozdzielczości na żywo",
    "Anında çözünürlük değişimi", "实时更改分辨率", "解像度のライブ変更", "실시간 해상도 변경"}},
{LiveHint, {
    "On: output resolution and preset change without a restart, but the game's post-processing "
    "stays at 1080p — noticeably slower on the Steam Deck and older GPUs. Off: everything is "
    "drawn at the preset resolution, changes need a restart. Auto turns it on for strong "
    "discrete GPUs. Applies after restarting the game.",
    "Включена: разрешение вывода и пресет меняются без перезапуска, но постобработка игры "
    "остаётся в 1080p — на Steam Deck и старых видеокартах это заметно медленнее. "
    "Выключена: всё рисуется в разрешении пресета, смена — через перезапуск. Авто включает "
    "её на мощных дискретных видеокартах. Применяется после перезапуска игры.",
    "Attivo: risoluzione di output e preset cambiano senza riavvio, ma il post-processing del "
    "gioco resta a 1080p (sensibilmente più lento su Steam Deck e GPU datate). Disattivo: tutto "
    "è disegnato alla risoluzione del preset, il cambio richiede il riavvio. Auto lo attiva "
    "sulle GPU dedicate potenti. Si applica dopo il riavvio del gioco.",
    "Activado: la resolución de salida y el preajuste cambian sin reiniciar, pero el posprocesado "
    "del juego queda en 1080p (notablemente más lento en Steam Deck y GPU antiguas). "
    "Desactivado: todo se dibuja a la resolución del preajuste y los cambios requieren "
    "reiniciar. Auto lo activa en GPU dedicadas potentes. Se aplica tras reiniciar el juego.",
    "Activé : la résolution de sortie et le préréglage changent sans redémarrage, mais le "
    "post-traitement du jeu reste en 1080p (nettement plus lent sur Steam Deck et les anciens "
    "GPU). Désactivé : tout est rendu à la résolution du préréglage, les changements demandent "
    "un redémarrage. Auto l'active sur les GPU dédiés puissants. S'applique après redémarrage du "
    "jeu.",
    "An: Ausgabeauflösung und Preset wechseln ohne Neustart, die Nachbearbeitung des Spiels "
    "bleibt aber bei 1080p (auf dem Steam Deck und älteren GPUs spürbar langsamer). Aus: Alles "
    "wird in der Preset-Auflösung gezeichnet, Wechsel erfordern einen Neustart. Auto schaltet es "
    "auf starken dedizierten GPUs ein. Wirkt nach einem Neustart des Spiels.",
    "Ativado: resolução de saída e predefinição mudam sem reiniciar, mas o pós-processamento do "
    "jogo fica em 1080p (bem mais lento no Steam Deck e em GPUs antigas). Desativado: tudo é "
    "desenhado na resolução da predefinição e as mudanças exigem reiniciar. Auto ativa em GPUs "
    "dedicadas potentes. Aplica-se após reiniciar o jogo.",
    "Włączona: rozdzielczość wyjściowa i preset zmieniają się bez restartu, ale postprocessing "
    "gry zostaje w 1080p (wyraźnie wolniej na Steam Deck i starszych GPU). Wyłączona: wszystko "
    "jest rysowane w rozdzielczości presetu, zmiany wymagają restartu. Auto włącza ją na mocnych "
    "dedykowanych GPU. Działa po restarcie gry.",
    "Açık: çıkış çözünürlüğü ve ön ayar yeniden başlatmadan değişir, ancak oyunun son işlemesi "
    "1080p'de kalır (Steam Deck ve eski GPU'larda belirgin şekilde daha yavaş). Kapalı: her şey "
    "ön ayar çözünürlüğünde çizilir, değişiklikler yeniden başlatma gerektirir. Otomatik, güçlü "
    "harici GPU'larda açar. Oyun yeniden başlatıldıktan sonra uygulanır.",
    "开启：输出分辨率和预设无需重启即可更改，但游戏后期处理保持 1080p（在 Steam Deck 和旧显卡上明显"
    "更慢）。关闭：全部按预设分辨率绘制，更改需要重启。自动：在性能强的独立显卡上开启。重启游戏后生效。",
    "オン：出力解像度とプリセットを再起動なしで変更できますが、ゲームのポストプロセスは 1080p の"
    "ままです（Steam Deck や旧 GPU では明らかに遅くなります）。オフ：すべてプリセット解像度で描画し、"
    "変更には再起動が必要です。自動：高性能な単体 GPU でオンにします。ゲーム再起動後に反映。",
    "켬: 출력 해상도와 프리셋을 재시작 없이 바꿀 수 있지만 게임 후처리는 1080p로 유지됩니다"
    "(Steam Deck과 구형 GPU에서 눈에 띄게 느림). 끔: 모두 프리셋 해상도로 그리며 변경하려면 "
    "재시작해야 합니다. 자동은 고성능 외장 GPU에서 켭니다. 게임을 재시작하면 적용됩니다."}},
{SecEffects, {
    "Game effects (after restart)", "Эффекты игры (после перезапуска)",
    "Effetti di gioco (dopo il riavvio)", "Efectos del juego (tras reiniciar)",
    "Effets du jeu (après redémarrage)", "Spieleffekte (nach Neustart)",
    "Efeitos do jogo (após reiniciar)", "Efekty gry (po restarcie)",
    "Oyun efektleri (yeniden başlatma sonrası)", "游戏效果（重启后生效）",
    "ゲーム エフェクト（再起動後）", "게임 효과 (재시작 후)"}},
{LodMax, {
    "Highest (-2)", "Максимальная (-2)", "Massimo (-2)", "Máximo (-2)", "Maximal (-2)",
    "Höchste (-2)", "Máximo (-2)", "Najwyższa (-2)", "En yüksek (-2)", "最高（-2）", "最高 (-2)",
    "최고 (-2)"}},
{LodGame, {
    "As in game", "Как в игре", "Come nel gioco", "Como en el juego", "Comme dans le jeu",
    "Wie im Spiel", "Como no jogo", "Jak w grze", "Oyundaki gibi", "与游戏相同", "ゲーム標準",
    "게임 기본값"}},
{LodLower, {
    "Lower (1)", "Ниже (1)", "Basso (1)", "Bajo (1)", "Réduit (1)", "Niedriger (1)", "Baixo (1)",
    "Niższa (1)", "Düşük (1)", "较低（1）", "低 (1)", "낮음 (1)"}},
{LodMin, {
    "Lowest (2)", "Минимальная (2)", "Minimo (2)", "Mínimo (2)", "Minimal (2)", "Niedrigste (2)",
    "Mínimo (2)", "Najniższa (2)", "En düşük (2)", "最低（2）", "最低 (2)", "최저 (2)"}},
{ModelLod, {
    "Model detail", "Детализация моделей", "Dettaglio modelli", "Detalle de modelos",
    "Détail des modèles", "Modelldetail", "Detalhe dos modelos", "Szczegółowość modeli",
    "Model ayrıntısı", "模型细节", "モデルの詳細度", "모델 디테일"}},
{EffChromatic, {
    "Chromatic aberration", "Хроматическая аберрация", "Aberrazione cromatica",
    "Aberración cromática", "Aberration chromatique", "Chromatische Aberration",
    "Aberração cromática", "Aberracja chromatyczna", "Renk sapması", "色差", "色収差", "색수차"}},
{EffDof, {
    "Depth of field (DoF)", "Глубина резкости (DoF)", "Profondità di campo (DoF)",
    "Profundidad de campo (DoF)", "Profondeur de champ (DoF)", "Tiefenschärfe (DoF)",
    "Profundidade de campo (DoF)", "Głębia ostrości (DoF)", "Alan derinliği (DoF)",
    "景深（DoF）", "被写界深度 (DoF)", "피사계 심도 (DoF)"}},
{EffMotionBlur, {
    "Motion blur", "Размытие в движении", "Motion blur", "Desenfoque de movimiento",
    "Flou de mouvement", "Bewegungsunschärfe", "Desfoque de movimento", "Rozmycie ruchu",
    "Hareket bulanıklığı", "动态模糊", "モーションブラー", "모션 블러"}},
{EffSsao, {
    "SSAO shading", "Затенение SSAO", "Occlusione SSAO", "Oclusión SSAO", "Occlusion SSAO",
    "SSAO-Verschattung", "Oclusão SSAO", "Cieniowanie SSAO", "SSAO gölgelendirme",
    "SSAO 环境光遮蔽", "SSAO（アンビエントオクルージョン）", "SSAO 앰비언트 오클루전"}},
{EffGameAa, {
    "Game's own anti-aliasing", "Собственное сглаживание игры", "Antialiasing del gioco",
    "Antialiasing propio del juego", "Anticrénelage du jeu", "Spieleigene Kantenglättung",
    "Anti-aliasing do próprio jogo", "Własny antyaliasing gry",
    "Oyunun kendi kenar yumuşatması", "游戏自带抗锯齿", "ゲーム本来のアンチエイリアス",
    "게임 자체 안티에일리어싱"}},
{EffDynamicShadows, {
    "Shadows from dynamic lights", "Тени от динамических источников", "Ombre da luci dinamiche",
    "Sombras de luces dinámicas", "Ombres des lumières dynamiques",
    "Schatten dynamischer Lichter", "Sombras de luzes dinâmicas",
    "Cienie od dynamicznych świateł", "Dinamik ışık gölgeleri", "动态光源阴影", "動的光源の影",
    "동적 광원 그림자"}},
{EffSsr, {
    "SSR reflections (not in the original game)", "Отражения SSR (не было в игре)",
    "Riflessi SSR (non nel gioco originale)", "Reflejos SSR (no estaban en el juego)",
    "Réflexions SSR (absentes du jeu original)", "SSR-Spiegelungen (nicht im Originalspiel)",
    "Reflexos SSR (não existiam no jogo)", "Odbicia SSR (nie było w grze)",
    "SSR yansımaları (orijinal oyunda yok)", "SSR 反射（原版游戏没有）",
    "SSR 反射（オリジナルにはなし）", "SSR 반사 (원작에 없음)"}},
{EffSkipIntro, {
    "Skip intro videos at start", "Пропуск заставок при запуске", "Salta i filmati iniziali",
    "Omitir vídeos de inicio", "Passer les vidéos d'introduction", "Intro-Videos überspringen",
    "Pular vídeos de abertura", "Pomiń filmy na starcie", "Açılış videolarını atla",
    "跳过开场视频", "起動時のムービーをスキップ", "시작 영상 건너뛰기"}},
{EffDebugCamera, {
    "Free camera (Cross + L3)", "Свободная камера (Cross + L3)", "Camera libera (Cross + L3)",
    "Cámara libre (Cross + L3)", "Caméra libre (Cross + L3)", "Freie Kamera (Cross + L3)",
    "Câmera livre (Cross + L3)", "Wolna kamera (Cross + L3)", "Serbest kamera (Cross + L3)",
    "自由视角（Cross + L3）", "フリーカメラ (Cross + L3)", "자유 카메라 (Cross + L3)"}},
{EffDebugMenu, {
    "Debug menu (needs font files)", "Debug menu (нужны файлы шрифтов)",
    "Debug menu (servono i font)", "Menú de depuración (requiere fuentes)",
    "Menu de débogage (polices requises)", "Debug-Menü (benötigt Schriftdateien)",
    "Menu de depuração (requer fontes)", "Menu debug (wymaga plików czcionek)",
    "Hata ayıklama menüsü (yazı tipi dosyaları gerekir)", "调试菜单（需要字体文件）",
    "デバッグメニュー（フォントファイルが必要）", "디버그 메뉴 (글꼴 파일 필요)"}},
{EffectsHint, {
    "Effects are switched on and off by game patches at start (patches/Bloodborne.xml). Motion "
    "blur and shadows from dynamic lights are noticeably heavy on the GPU.",
    "Эффекты включаются и выключаются патчами игры при запуске (patches/Bloodborne.xml). "
    "Размытие в движении и тени от динамических источников заметно нагружают GPU.",
    "Gli effetti vengono attivati e disattivati con patch del gioco all'avvio "
    "(patches/Bloodborne.xml). Motion blur e ombre da luci dinamiche pesano molto sulla GPU.",
    "Los efectos se activan y desactivan con parches del juego al iniciar "
    "(patches/Bloodborne.xml). El desenfoque de movimiento y las sombras de luces dinámicas "
    "cargan bastante la GPU.",
    "Les effets sont activés et désactivés par des patchs du jeu au démarrage "
    "(patches/Bloodborne.xml). Le flou de mouvement et les ombres des lumières dynamiques pèsent "
    "nettement sur le GPU.",
    "Effekte werden beim Start per Spielpatch ein- und ausgeschaltet (patches/Bloodborne.xml). "
    "Bewegungsunschärfe und Schatten dynamischer Lichter belasten die GPU spürbar.",
    "Os efeitos são ligados e desligados por patches do jogo ao iniciar "
    "(patches/Bloodborne.xml). Desfoque de movimento e sombras de luzes dinâmicas pesam bastante "
    "na GPU.",
    "Efekty są włączane i wyłączane łatkami gry przy starcie (patches/Bloodborne.xml). "
    "Rozmycie ruchu i cienie od dynamicznych świateł wyraźnie obciążają GPU.",
    "Efektler başlangıçta oyun yamalarıyla açılıp kapatılır (patches/Bloodborne.xml). Hareket "
    "bulanıklığı ve dinamik ışık gölgeleri GPU'yu belirgin şekilde yorar.",
    "这些效果在启动时通过游戏补丁开关（patches/Bloodborne.xml）。动态模糊和动态光源阴影对显卡负担明显。",
    "エフェクトは起動時にゲームパッチで切り替えます（patches/Bloodborne.xml）。モーションブラーと"
    "動的光源の影は GPU 負荷が大きめです。",
    "효과는 시작 시 게임 패치로 켜고 끕니다(patches/Bloodborne.xml). 모션 블러와 동적 광원 그림자는 "
    "GPU 부하가 큽니다."}},
{DebugHint, {
    "Free camera: hold Cross and press L3 (keyboard: Space + Z). Debug menu: left touchpad / "
    "Tab. Needs DbgFont14h.ccm and DbgFont14h.tpf in dvdroot_ps4/font from Nexus mod #253. "
    "Right touchpad: Backspace.",
    "Свободная камера: удерживайте Cross и нажимайте L3 (клавиатура: Space + Z). "
    "Debug menu: левый touchpad / Tab. Нужны DbgFont14h.ccm и DbgFont14h.tpf "
    "в dvdroot_ps4/font из мода Nexus #253. Правый touchpad: Backspace.",
    "Camera libera: tieni Cross e premi L3 (tastiera: Space + Z). Debug menu: touchpad sinistro "
    "/ Tab. Servono DbgFont14h.ccm e DbgFont14h.tpf in dvdroot_ps4/font dalla mod Nexus #253. "
    "Touchpad destro: Backspace.",
    "Cámara libre: mantén Cross y pulsa L3 (teclado: Space + Z). Menú de depuración: touchpad "
    "izquierdo / Tab. Requiere DbgFont14h.ccm y DbgFont14h.tpf en dvdroot_ps4/font del mod de "
    "Nexus n.º 253. Touchpad derecho: Backspace.",
    "Caméra libre : maintenez Cross et appuyez sur L3 (clavier : Space + Z). Menu de débogage : "
    "pavé tactile gauche / Tab. Nécessite DbgFont14h.ccm et DbgFont14h.tpf dans dvdroot_ps4/font "
    "(mod Nexus n° 253). Pavé tactile droit : Backspace.",
    "Freie Kamera: Cross halten und L3 drücken (Tastatur: Space + Z). Debug-Menü: linkes "
    "Touchpad / Tab. Benötigt DbgFont14h.ccm und DbgFont14h.tpf in dvdroot_ps4/font aus der "
    "Nexus-Mod #253. Rechtes Touchpad: Backspace.",
    "Câmera livre: segure Cross e aperte L3 (teclado: Space + Z). Menu de depuração: touchpad "
    "esquerdo / Tab. Requer DbgFont14h.ccm e DbgFont14h.tpf em dvdroot_ps4/font do mod #253 do "
    "Nexus. Touchpad direito: Backspace.",
    "Wolna kamera: przytrzymaj Cross i naciśnij L3 (klawiatura: Space + Z). Menu debug: lewy "
    "touchpad / Tab. Wymaga DbgFont14h.ccm i DbgFont14h.tpf w dvdroot_ps4/font z moda Nexus "
    "#253. Prawy touchpad: Backspace.",
    "Serbest kamera: Cross'u basılı tutup L3'e basın (klavye: Space + Z). Hata ayıklama menüsü: "
    "sol dokunmatik yüzey / Tab. Nexus #253 modundan dvdroot_ps4/font içinde DbgFont14h.ccm ve "
    "DbgFont14h.tpf gerekir. Sağ dokunmatik yüzey: Backspace.",
    "自由视角：按住 Cross 再按 L3（键盘：Space + Z）。调试菜单：左触摸板 / Tab。需要 Nexus 模组 #253 "
    "中的 DbgFont14h.ccm 和 DbgFont14h.tpf，放在 dvdroot_ps4/font。右触摸板：Backspace。",
    "フリーカメラ：Cross を押しながら L3（キーボード：Space + Z）。デバッグメニュー：タッチパッド左 / "
    "Tab。Nexus の MOD #253 にある DbgFont14h.ccm と DbgFont14h.tpf を dvdroot_ps4/font に置く必要が"
    "あります。タッチパッド右：Backspace。",
    "자유 카메라: Cross를 누른 채 L3(키보드: Space + Z). 디버그 메뉴: 터치패드 왼쪽 / Tab. Nexus "
    "모드 #253의 DbgFont14h.ccm과 DbgFont14h.tpf를 dvdroot_ps4/font에 넣어야 합니다. 터치패드 "
    "오른쪽: Backspace."}},
{RestartNeeded, {
    "Changes apply after restarting the game", "Изменения применятся после перезапуска игры",
    "Le modifiche verranno applicate dopo il riavvio del gioco",
    "Los cambios se aplicarán tras reiniciar el juego",
    "Les changements s'appliqueront après redémarrage du jeu",
    "Änderungen wirken nach einem Neustart des Spiels", "As mudanças valem após reiniciar o jogo",
    "Zmiany zadziałają po restarcie gry",
    "Değişiklikler oyun yeniden başlatıldıktan sonra uygulanır", "更改将在重启游戏后生效",
    "変更はゲーム再起動後に反映されます", "변경 사항은 게임을 재시작하면 적용됩니다"}},
{ApplyRestart, {
    "Apply and restart the game", "Применить и перезапустить игру", "Applica e riavvia il gioco",
    "Aplicar y reiniciar el juego", "Appliquer et redémarrer le jeu",
    "Übernehmen und Spiel neu starten", "Aplicar e reiniciar o jogo",
    "Zastosuj i uruchom grę ponownie", "Uygula ve oyunu yeniden başlat", "应用并重启游戏",
    "適用してゲームを再起動", "적용하고 게임 재시작"}},
{SecOther, {
    "Other", "Прочее", "Altro", "Otros", "Divers", "Sonstiges", "Outros", "Inne", "Diğer", "其他",
    "その他", "기타"}},
{ShowFps, {
    "FPS counter in the corner", "Счётчик FPS в углу", "Contatore FPS in un angolo",
    "Contador de FPS en la esquina", "Compteur de FPS dans le coin", "FPS-Anzeige in der Ecke",
    "Contador de FPS no canto", "Licznik FPS w rogu", "Köşede FPS sayacı", "角落显示帧率",
    "画面隅に FPS を表示", "구석에 FPS 표시"}},
{Language, {
    "Language", "Язык", "Lingua", "Idioma", "Langue", "Sprache", "Idioma", "Język", "Dil", "语言",
    "言語", "언어"}},
{Close, {
    "Close", "Закрыть", "Chiudi", "Cerrar", "Fermer", "Schließen", "Fechar", "Zamknij", "Kapat",
    "关闭", "閉じる", "닫기"}},
{QuitGame, {
    "Quit game", "Выйти из игры", "Esci dal gioco", "Salir del juego", "Quitter le jeu",
    "Spiel beenden", "Sair do jogo", "Wyjdź z gry", "Oyundan çık", "退出游戏", "ゲームを終了",
    "게임 종료"}},
{QuitConfirm, {
    "Press again to quit", "Нажмите ещё раз для выхода", "Premi di nuovo per uscire",
    "Pulsa de nuevo para salir", "Appuyez à nouveau pour quitter", "Zum Beenden erneut drücken",
    "Pressione novamente para sair", "Naciśnij ponownie, aby wyjść", "Çıkmak için tekrar basın",
    "再按一次退出", "もう一度押すと終了", "다시 누르면 종료"}},
{SavedIn, {
    "Settings are saved in bbport.ini", "Настройки сохраняются в bbport.ini",
    "Impostazioni salvate in bbport.ini", "Los ajustes se guardan en bbport.ini",
    "Les paramètres sont enregistrés dans bbport.ini",
    "Einstellungen werden in bbport.ini gespeichert",
    "As configurações são salvas em bbport.ini", "Ustawienia są zapisywane w bbport.ini",
    "Ayarlar bbport.ini dosyasına kaydedilir", "设置保存在 bbport.ini",
    "設定は bbport.ini に保存されます", "설정은 bbport.ini에 저장됩니다"}},
{FpsCounter, {
    "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f мс  %s", "%.0f FPS  %.1f ms  %s",
    "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f ms  %s",
    "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f ms  %s",
    "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f ms  %s"}},
{NamePrompt, {
    "Press Enter to Confirm, Esc to Cancel", "Enter — подтвердить, Esc — отмена",
    "Invio per confermare, Esc per annullare", "Enter para confirmar, Esc para cancelar",
    "Entrée pour valider, Échap pour annuler", "Enter zum Bestätigen, Esc zum Abbrechen",
    "Enter para confirmar, Esc para cancelar", "Enter — zatwierdź, Esc — anuluj",
    "Onaylamak için Enter, iptal için Esc", "按 Enter 确认，Esc 取消",
    "Enter で決定、Esc でキャンセル", "Enter로 확인, Esc로 취소"}},
{ShowHud, {
    "Performance HUD", "Нативный оверлей производительности (HUD)", "HUD delle prestazioni",
    "HUD de rendimiento", "HUD de performances", "Leistungs-HUD", "HUD de desempenho",
    "HUD wydajności", "Performans HUD'u", "性能 HUD", "パフォーマンス HUD", "성능 HUD"}},
{HudHint, {
    "In-game telemetry overlay (F11 / Shift+Tab): FPS, frametime graph, VRAM, CPU, RAM and "
    "render metrics.",
    "Внутриигровой оверлей телеметрии (F11 / Shift+Tab): FPS, график фреймтайма, VRAM, CPU, RAM "
    "и метрики рендера",
    "Overlay di telemetria in gioco (F11 / Shift+Tab): FPS, grafico del frametime, VRAM, CPU, "
    "RAM e metriche di rendering.",
    "Superposición de telemetría en el juego (F11 / Shift+Tab): FPS, gráfico de tiempos de "
    "fotograma, VRAM, CPU, RAM y métricas de render.",
    "Affichage de télémétrie en jeu (F11 / Shift+Tab) : FPS, graphique du temps d'image, VRAM, "
    "CPU, RAM et métriques de rendu.",
    "Telemetrie-Overlay im Spiel (F11 / Shift+Tab): FPS, Frametime-Graph, VRAM, CPU, RAM und "
    "Rendermetriken.",
    "Sobreposição de telemetria no jogo (F11 / Shift+Tab): FPS, gráfico de tempo de quadro, "
    "VRAM, CPU, RAM e métricas de renderização.",
    "Nakładka telemetrii w grze (F11 / Shift+Tab): FPS, wykres czasu klatki, VRAM, CPU, RAM i "
    "metryki renderowania.",
    "Oyun içi telemetri katmanı (F11 / Shift+Tab): FPS, kare süresi grafiği, VRAM, CPU, RAM ve "
    "işleme ölçümleri.",
    "游戏内遥测浮层（F11 / Shift+Tab）：帧率、帧时间曲线、显存、CPU、内存和渲染指标。",
    "ゲーム内テレメトリー表示（F11 / Shift+Tab）：FPS、フレームタイムのグラフ、VRAM、CPU、RAM、"
    "描画メトリクス。",
    "게임 내 텔레메트리 오버레이(F11 / Shift+Tab): FPS, 프레임 타임 그래프, VRAM, CPU, RAM 및 렌더 "
    "지표."}},
{HudQuadTopLeft, {
    "Top left", "Сверху слева", "In alto a sinistra", "Arriba a la izquierda",
    "En haut à gauche", "Oben links", "Superior esquerdo", "Lewy górny róg", "Sol üst", "左上",
    "左上", "왼쪽 위"}},
{HudQuadTopRight, {
    "Top right", "Сверху справа", "In alto a destra", "Arriba a la derecha", "En haut à droite",
    "Oben rechts", "Superior direito", "Prawy górny róg", "Sağ üst", "右上", "右上", "오른쪽 위"}},
{HudQuadBottomLeft, {
    "Bottom left", "Снизу слева", "In basso a sinistra", "Abajo a la izquierda",
    "En bas à gauche", "Unten links", "Inferior esquerdo", "Lewy dolny róg", "Sol alt", "左下",
    "左下", "왼쪽 아래"}},
{HudQuadBottomRight, {
    "Bottom right", "Снизу справа", "In basso a destra", "Abajo a la derecha",
    "En bas à droite", "Unten rechts", "Inferior direito", "Prawy dolny róg", "Sağ alt", "右下",
    "右下", "오른쪽 아래"}},
{HudPosition, {
    "HUD position", "Расположение HUD", "Posizione HUD", "Posición del HUD", "Position du HUD",
    "HUD-Position", "Posição do HUD", "Położenie HUD", "HUD konumu", "HUD 位置", "HUD の位置",
    "HUD 위치"}},
{HudOpacity, {
    "HUD opacity", "Прозрачность HUD", "Opacità HUD", "Opacidad del HUD", "Opacité du HUD",
    "HUD-Deckkraft", "Opacidade do HUD", "Krycie HUD", "HUD opaklığı", "HUD 不透明度",
    "HUD の不透明度", "HUD 불투명도"}},
{HudScale, {
    "HUD scale", "Масштаб HUD", "Scala HUD", "Escala del HUD", "Échelle du HUD", "HUD-Größe",
    "Escala do HUD", "Skala HUD", "HUD ölçeği", "HUD 缩放", "HUD のスケール", "HUD 배율"}},
{SecMk, {
    "Mouse and keyboard controls (M&K)", "Управление мышью и клавиатурой (M&K)",
    "Controlli mouse e tastiera (M&K)", "Controles de ratón y teclado (M&K)",
    "Commandes souris et clavier (M&K)", "Maus- und Tastatursteuerung (M&K)",
    "Controles de mouse e teclado (M&K)", "Sterowanie myszą i klawiaturą (M&K)",
    "Fare ve klavye kontrolleri (M&K)", "键鼠操作（M&K）", "マウスとキーボード操作 (M&K)",
    "마우스 및 키보드 조작 (M&K)"}},
{MkEnable, {
    "Enable mouse and keyboard controls", "Включить управление мышью и клавиатурой",
    "Attiva i controlli mouse e tastiera", "Activar controles de ratón y teclado",
    "Activer les commandes souris et clavier", "Maus- und Tastatursteuerung aktivieren",
    "Ativar controles de mouse e teclado", "Włącz sterowanie myszą i klawiaturą",
    "Fare ve klavye kontrollerini etkinleştir", "启用键鼠操作",
    "マウスとキーボード操作を有効化", "마우스 및 키보드 조작 사용"}},
{MkHint, {
    "Souls-style PC control scheme: mouse look (cursor captured), LMB: normal attack (R1), "
    "Shift+LMB: strong attack (R2), RMB: gun / left hand (L2), MMB: lock on (R3), Tab: transform "
    "weapon (L1).",
    "Схема управления в стиле Souls PC: обзор мышью (захват курсора), ЛКМ - обычная атака (R1), "
    "Shift+ЛКМ - сильная (R2), ПКМ - выстрел/левая рука (L2), СКМ - захват цели (R3), Tab - "
    "трансформация оружия (L1)",
    "Schema di controllo in stile Souls per PC: visuale con il mouse (cursore catturato), clic "
    "sinistro: attacco normale (R1), Shift+clic sinistro: attacco potente (R2), clic destro: arma "
    "da fuoco / mano sinistra (L2), clic centrale: aggancio del bersaglio (R3), Tab: "
    "trasformazione dell'arma (L1).",
    "Esquema de control estilo Souls para PC: cámara con el ratón (cursor capturado), clic "
    "izquierdo: ataque normal (R1), Shift+clic izquierdo: ataque fuerte (R2), clic derecho: arma "
    "de fuego / mano izquierda (L2), clic central: fijar objetivo (R3), Tab: transformar el arma "
    "(L1).",
    "Schéma de contrôle PC façon Souls : caméra à la souris (curseur capturé), clic gauche : "
    "attaque normale (R1), Shift+clic gauche : attaque puissante (R2), clic droit : arme à feu / "
    "main gauche (L2), clic molette : verrouillage de cible (R3), Tab : transformation de l'arme "
    "(L1).",
    "Steuerung im Stil der Souls-PC-Spiele: Maus-Kamera (Mauszeiger gefangen), LMB: normaler "
    "Angriff (R1), Shift+LMB: starker Angriff (R2), RMB: Schusswaffe / linke Hand (L2), MMB: Ziel "
    "erfassen (R3), Tab: Waffe transformieren (L1).",
    "Esquema de controle no estilo Souls para PC: câmera com o mouse (cursor capturado), clique "
    "esquerdo: ataque normal (R1), Shift+clique esquerdo: ataque forte (R2), clique direito: arma "
    "de fogo / mão esquerda (L2), clique do meio: travar mira (R3), Tab: transformar a arma (L1).",
    "Sterowanie w stylu Souls na PC: kamera myszą (kursor przechwycony), LPM: zwykły atak (R1), "
    "Shift+LPM: silny atak (R2), PPM: broń palna / lewa ręka (L2), ŚPM: namierzanie celu (R3), "
    "Tab: transformacja broni (L1).",
    "Souls tarzı PC kontrol düzeni: fareyle bakış (imleç yakalanır), sol tık: normal saldırı "
    "(R1), Shift+sol tık: güçlü saldırı (R2), sağ tık: ateşli silah / sol el (L2), orta tık: "
    "hedefe kilitlenme (R3), Tab: silahı dönüştürme (L1).",
    "魂系 PC 风格操作：鼠标控制视角（锁定光标），左键：普通攻击（R1），Shift+左键：重攻击（R2），"
    "右键：枪械/左手（L2），中键：锁定目标（R3），Tab：武器变形（L1）。",
    "Souls 系 PC 風の操作：マウスで視点操作（カーソルを固定）、左クリック：通常攻撃 (R1)、"
    "Shift+左クリック：強攻撃 (R2)、右クリック：銃／左手 (L2)、中クリック：ロックオン (R3)、"
    "Tab：武器変形 (L1)。",
    "소울 PC 스타일 조작: 마우스 시점(커서 고정), 좌클릭: 일반 공격(R1), Shift+좌클릭: 강공격(R2), "
    "우클릭: 총기/왼손(L2), 휠 클릭: 록온(R3), Tab: 무기 변형(L1)."}},
{MkSensX, {
    "Horizontal sensitivity (X)", "Чувствительность по горизонтали (X)",
    "Sensibilità orizzontale (X)", "Sensibilidad horizontal (X)", "Sensibilité horizontale (X)",
    "Horizontale Empfindlichkeit (X)", "Sensibilidade horizontal (X)", "Czułość pozioma (X)",
    "Yatay hassasiyet (X)", "水平灵敏度（X）", "水平感度 (X)", "가로 감도 (X)"}},
{MkSensY, {
    "Vertical sensitivity (Y)", "Чувствительность по вертикали (Y)", "Sensibilità verticale (Y)",
    "Sensibilidad vertical (Y)", "Sensibilité verticale (Y)", "Vertikale Empfindlichkeit (Y)",
    "Sensibilidade vertical (Y)", "Czułość pionowa (Y)", "Dikey hassasiyet (Y)",
    "垂直灵敏度（Y）", "垂直感度 (Y)", "세로 감도 (Y)"}},
{MkInvertX, {
    "Invert horizontal look (X)", "Инвертировать обзор по горизонтали (X)",
    "Inverti visuale orizzontale (X)", "Invertir cámara horizontal (X)",
    "Inverser la caméra horizontale (X)", "Horizontale Kamera umkehren (X)",
    "Inverter câmera horizontal (X)", "Odwróć kamerę w poziomie (X)",
    "Yatay bakışı ters çevir (X)", "反转水平视角（X）", "水平視点を反転 (X)", "가로 시점 반전 (X)"}},
{MkInvertY, {
    "Invert vertical look (Y)", "Инвертировать обзор по вертикали (Y)",
    "Inverti visuale verticale (Y)", "Invertir cámara vertical (Y)",
    "Inverser la caméra verticale (Y)", "Vertikale Kamera umkehren (Y)",
    "Inverter câmera vertical (Y)", "Odwróć kamerę w pionie (Y)", "Dikey bakışı ters çevir (Y)",
    "反转垂直视角（Y）", "垂直視点を反転 (Y)", "세로 시점 반전 (Y)"}},
{MkDeadzone, {
    "Mouse dead zone", "Мёртвая зона мыши", "Zona morta del mouse", "Zona muerta del ratón",
    "Zone morte de la souris", "Maus-Totzone", "Zona morta do mouse", "Martwa strefa myszy",
    "Fare ölü bölgesi", "鼠标死区", "マウスのデッドゾーン", "마우스 데드존"}},
{MkSmoothing, {
    "Mouse movement smoothing", "Сглаживание движений мыши",
    "Smorzamento del movimento del mouse", "Suavizado del movimiento del ratón",
    "Lissage des mouvements de la souris", "Glättung der Mausbewegung",
    "Suavização do movimento do mouse", "Wygładzanie ruchu myszy", "Fare hareketi yumuşatma",
    "鼠标移动平滑", "マウス移動のスムージング", "마우스 움직임 스무딩"}},
};
// clang-format on

static_assert(sizeof(kEntries) / sizeof(kEntries[0]) == StrCount, "one entry per string");

constexpr bool Ordered() {
    for (int i = 0; i < StrCount; ++i) {
        if (kEntries[i].id != i) return false;
    }
    return true;
}
static_assert(Ordered(), "entries in Str order");

constexpr const char* kNames[LangCount] = {
    "English", "Русский", "Italiano", "Español", "Français", "Deutsch", "Português (Brasil)",
    "Polski", "Türkçe", "简体中文", "日本語", "한국어"};
constexpr const char* kCodes[LangCount] = {"en", "ru", "it", "es", "fr", "de",
                                           "pt", "pl", "tr", "zh", "ja", "ko"};

} // namespace

Lang SystemLanguage() {
    static const Lang detected = [] {
#ifdef _WIN32
        switch (PRIMARYLANGID(GetUserDefaultUILanguage())) {
        case LANG_RUSSIAN: return Ru;
        case LANG_ITALIAN: return It;
        case LANG_SPANISH: return Es;
        case LANG_FRENCH: return Fr;
        case LANG_GERMAN: return De;
        case LANG_PORTUGUESE: return Pt;
        case LANG_POLISH: return Pl;
        case LANG_TURKISH: return Tr;
        case LANG_CHINESE: return Zh;
        case LANG_JAPANESE: return Ja;
        case LANG_KOREAN: return Ko;
        default: return En;
        }
#else
        const char* env = std::getenv("LC_ALL");
        if (!env || !env[0]) env = std::getenv("LC_MESSAGES");
        if (!env || !env[0]) env = std::getenv("LANG");
        if (env) {
            for (int l = 0; l < LangCount; ++l) {
                if (std::strncmp(env, kCodes[l], 2) == 0) return Lang(l);
            }
        }
        return En;
#endif
    }();
    return detected;
}

Lang Current() {
    const int lang = BbSettings::Get().language;
    return lang >= 0 && lang < LangCount ? Lang(lang) : SystemLanguage();
}

const char* T(Str id) {
    const char* text = kEntries[id].text[Current()];
    return text ? text : kEntries[id].text[En];
}

const char* Name(int lang) {
    return lang >= 0 && lang < LangCount ? kNames[lang] : "Auto";
}

const char* Code(int lang) {
    return lang >= 0 && lang < LangCount ? kCodes[lang] : "auto";
}

} // namespace BbI18n
