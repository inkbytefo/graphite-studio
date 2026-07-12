# Graphite Studio - Yol Haritası (Roadmap)

Graphite Studio'nun geliştirilme süreci adımları, Adobe Photoshop 2026 ile yapılan karşılaştırmalı analiz doğrultusunda güncellenmiştir. Profesyonel bir katmanlı görsel düzenleyici hedefine ulaşmak için eksiklerimiz fazlara bölünerek planlanmıştır.

---

## 📊 Adobe Photoshop 2026 Kıyaslama Analizi ve Eksiklerimiz

Mevcut arayüz ve GPU altyapımız oldukça hızlı ve kararlı çalışmakla birlikte, Photoshop seviyesinde bir iş akışı için aşağıdaki kritik eksiklikler tespit edilmiştir:

1. **Tahribatsız Düzenleme (Non-Destructive Editing) Eksikliği**:
   - *Photoshop:* Maskeler (Layer Masks) ve Ayarlama Katmanları (Adjustment Layers) ile pikselleri bozmadan renk ve ışık düzenlemesi yapılabilir.
   - *Graphite Studio:* Filtre ve çizimler doğrudan katman piksellerine yazılmaktadır (Destructive).
2. **Proje Kaydetme ve Katman Yapısının Korunması**:
   - *Photoshop:* Çoklu katmanları, opaklıkları ve blend modlarını koruyan PSD/PSB formatını destekler.
   - *Graphite Studio:* Sadece PNG/JPG (düzleştirilmiş veya tek katmanlı) çıktı alabilmektedir; projeyi katmanlarıyla kaydedip açacak bir formatı yoktur.
3. **Seçim Motoru (Selection Engine) ve Karınca Yürüyüşü (Marching Ants)**:
   - *Photoshop:* Kement, Sihirli Değnek ve hızlı seçim araçları ile piksel alanları seçilip maskelenebilir. Aktif seçim sınırı animasyonlu "marching ants" çizgisiyle gösterilir.
   - *Graphite Studio:* Seçim mantığı henüz bulunmamaktadır.
4. **Vektör Katmanları ve Akıllı Nesneler (Smart Objects)**:
   - *Photoshop:* Pen (Kalem) aracı ve hazır şekiller vektörel olarak saklanır ve kalite kaybı olmadan boyutlandırılır.
   - *Graphite Studio:* Tamamen raster (piksel) tabanlıdır.

---

## 📍 Faz 1: Proje Kurulumu ve Arayüz Temelleri `(Tamamlandı)`
- [x] CMake build sisteminin kurulması ve bağımlılıkların (`GLFW`, `OpenGL`, `GLAD`, `Dear ImGui`) tanımlanması
- [x] Grafik arayüz için özel **Segoe UI** font entegrasyonu
- [x] Piksel düzeyinde doğru koyu tema renk paletinin tasarlanması ve uygulanması
- [x] Sabit ekran yerleşimi (Toolbar, Canvas, Properties, Layers, StatusBar)
- [x] Arayüz yerleşimini tek tıkla varsayılan düzene sıfırlama seçeneği (Reset Layout)

## 📍 Faz 2: OpenGL Canvas & Viewport `(Tamamlandı)`
- [x] GPU doku (texture) yükleme ve dinamik çizim tuvali
- [x] Tuval üzerinde akıcı kaydırma (Pan) ve yakınlaştırma (Zoom) kontrolleri
- [x] Yüksek oranda yakınlaştırma yapıldığında piksel ızgarasının (pixel grid) otomatik çizilmesi
- [x] Görseli pencereye otomatik sığdırma (Fit to Window) algoritmaları

## 📍 Faz 3: Katman Sistemi (Layer Stack) & Blending `(Tamamlandı)`
- [x] Çoklu katman (Layer) hiyerarşik veri yapısının kurulması
- [x] CPU piksel önbelleği ile hızlı layer kopyalama ve birleştirme işlemleri
- [x] GLSL Shader'ları ile donanım hızlandırmalı katman harmanlama modları (Multiply, Screen, Overlay, Soft Light vb.)
- [x] Katman opaklığı (opacity) ve görünürlük (visibility) ayarlarının entegrasyonu

## 📍 Faz 4: Docking Arayüzü & Gelişmiş Paneller `(Tamamlandı)`
- [x] Layers, History, Color, Toolbar panellerinin işlevsel hale getirilmesi
- [x] Geri Al / İleri Al (Undo / Redo) komut motorunun tasarımı (Command Pattern) ve entegrasyonu
- [x] Sürükle-bırak (drag and drop) ile katman sırasını arayüzden dinamik değiştirebilme

## 📍 Faz 5: Fırça Motoru & Temel Çizim `(Tamamlandı)`
- [x] Alt-piksel hassasiyetli fırça (Brush) motoru ve çizgi interpolasyonu
- [x] Silgi (Eraser) aracı ve alfa kompozisyon mantığının (`BlendMath.h`) ayrıştırılması

## 📍 Faz 6: GPU Modernizasyonu, DSA ve RAII `(Tamamlandı)`
- [x] OpenGL API versiyonunun 4.6 Core Profile standardına yükseltilmesi
- [x] Direct State Access (DSA) sayesinde global durum kirliliğine yol açmayan kaynak yönetimi
- [x] `GLBuffer`, `GLVertexArray`, `GLTexture`, `GLFramebuffer` RAII sarmalayıcıları ile sızdırmaz kaynak yönetimi
- [x] `glGetTextureSubImage` ile FBO bağlama gereksinimi duymadan doku verisi okuma
- [x] Rule of Zero mimarisi ile temiz, hatasız taşıma (move) operatörleri ve bellek yönetimi

---

## 📍 Faz 7: Proje Kaydetme & Çoklu Katman Dosya Formatı `(Sıradaki)`
- [ ] Çoklu katman yapısını, opaklıklarını, görünürlüklerini ve blend modlarını koruyarak kaydeden özel **.gsp (Graphite Studio Project)** dosya yapısının tasarlanması (JSON meta verisi + sıkıştırılmış katman pikselleri paketi)
- [ ] Katmanlı projeyi açma (Load Project) ve kaldığı yerden devam edebilme desteği
- [ ] Dosya açma ve kaydetme esnasında arayüzde ilerleme çubuğu (Progress Bar) gösterimi
- [ ] **(UI/UX)** Dikey araç çubuğundaki harf kısayolları yerine grafik simgelerin (ikonlar) yerleştirilmesi
- [ ] **(UI/UX)** Options Bar'ın etkinleştirilmesi (Fırça boyutu, sertliği ve opaklığı için üst barda sürgüler)

## 📍 Faz 8: Seçim Motoru & Canlı Seçim Sınırları `(Planlanıyor)`
- [ ] Dikdörtgen, Daire ve Serbest Kement (Lasso) seçim araçları
- [ ] GPU tabanlı, animasyonlu kesikli çizgi efekti (**Karınca Yürüyüşü / Marching Ants**) shader implementasyonu
- [ ] Seçilen alanı maskeleme, kopyalama, silme ve sadece seçili alana fırça darbelerini sınırlama (Selection Masking)
- [ ] **(UI/UX)** Sağ panele HSB/RGB Renk Paleti (Color Panel) ve hazır renk önizleme paletlerinin (Swatches) eklenmesi

## 📍 Faz 9: Donanım Hızlandırmalı Filtreler ve Ayarlamalar `(Planlanıyor)`
- [ ] GLSL Shader tabanlı gerçek zamanlı önizlemeli filtreler (Gaussian Blur, Sharpen, HSL Adjustment, Grayscale)
- [ ] Filtrelerin parametrelerini ayarlamak için dinamik ImGui araç pencereleri (Dialogs)
- [ ] **(UI/UX)** Katmanlar panelinde her katman için küçük dinamik piksel önizlemelerinin (Layer Thumbnails) çizdirilmesi

## 📍 Faz 10: Tahribatsız Maskeleme & Ayarlama Katmanları `(Vizyon)`
- [ ] Katman Maskeleri (Layer Masks): Her katmana bağlanabilen 8-bit gri tonlamalı maske dokuları
- [ ] Renk ve Işık Ayarlama Katmanları (Adjustment Layers - Curves, Levels, Brightness/Contrast)
- [ ] Vektör Şekil Katmanları (Rectangle, Ellipse, Line) ve Pen Tool temeli
- [ ] **(UI/UX)** Çoklu belge sekmeleri (Multi-tab canvas view) mimarisi ile aynı anda birden çok projeyle çalışabilme

