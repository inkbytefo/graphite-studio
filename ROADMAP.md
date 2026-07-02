# Graphite Studio - Yol Haritası (Roadmap)

Graphite Studio'nun geliştirilme süreci adımları ve güncel durumu aşağıda belirtilmiştir. Katkıda bulunmak (contribute) isteyen geliştiriciler bu sıralamayı takip edebilirler.

---

## 📍 Faz 1: Proje Kurulumu ve Arayüz Temelleri `(Tamamlandı)`
- [x] CMake build sisteminin kurulması ve bağımlılıkların (`GLFW`, `OpenGL`, `GLAD`, `Dear ImGui`) tanımlanması
- [x] Grafik arayüz için özel **Segoe UI** font entegrasyonu
- [x] Piksel düzeyinde doğru koyu tema renk paletinin tasarlanması ve uygulanması
- [x] Sabit ekran yerleşimi (Toolbar, Canvas, Properties, Layers, StatusBar)
- [x] Arayüz yerleşimini tek tıkla varsayılan düzene sıfırlama seçeneği (Reset Layout)

---

## 📍 Faz 2: OpenGL Canvas & Viewport `(Sıradaki)`
- [ ] GPU doku (texture) yükleme ve dinamik çizim tuvali
- [ ] Tuval üzerinde akıcı kaydırma (Pan) ve yakınlaştırma (Zoom) kontrolleri
- [ ] Yüksek oranda yakınlaştırma yapıldığında piksel ızgarasının (pixel grid) otomatik çizilmesi
- [ ] Görseli pencereye otomatik sığdırma (Fit to Window) algoritmaları

---

## 📍 Faz 3: Katman Sistemi (Layer Stack) & Blending
- [ ] Çoklu katman (Layer) veri yapısının ve hiyerarşisinin kurulması
- [ ] GLSL Shader'ları ile donanım hızlandırmalı katman harmanlama modları (Multiply, Screen, Overlay, Soft Light vb.)
- [ ] Katman opaklığı (opacity) ve görünürlük (visibility) ayarlarının çekirdek motorla bağlanması

---

## 📍 Faz 4: Docking Arayüzü & Gelişmiş Paneller
- [ ] Layers, History, Color, Toolbar panellerinin işlevsel hale getirilmesi
- [ ] Geri Al / İleri Al (Undo / Redo) komut motorunun tasarımı (Command Pattern)
- [ ] Sürükle-bırak (drag and drop) ile katman sırasını değiştirebilme

---

## 📍 Faz 5: Fırça Motoru & Seçim Araçları
- [ ] Grafik tablet basınç hassasiyetine duyarlı fırça (Brush) motoru
- [ ] Lasso (Kement) ve Marquee (Seçim Çerçevesi) araçları
- [ ] Seçim alanları etrafında hareketli çizgi efekti (Karınca Yürüyüşü / Marching Ants)

---

## 📍 Faz 6: Filtreler, Ayarlamalar & PSD Desteği
- [ ] Gaussian Blur, Keskinleştirme (Sharpen) gibi filtrelerin GPU/GLSL üzerinde uygulanması
- [ ] Curves (Eğriler) ve Levels (Düzeyler) gibi renk ayarlama katmanları
- [ ] Popüler dosya formatlarının (PNG, JPG, BMP) yanı sıra PSD dosya okuma/yazma desteği
