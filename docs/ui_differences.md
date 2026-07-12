# Graphite Studio — Arayüz (UI/UX) Karşılaştırma Analizi

Adobe Photoshop 2026 arayüz tasarımı ve kullanıcı deneyimi (UX) standartları ile Graphite Studio arayüzü karşılaştırıldığında tespit edilen temel farklar ve iyileştirme önerileri aşağıda listelenmiştir.

---

## 🔍 Temel Arayüz Farkları ve Eksiklikler

### 1. Dikey Araç Çubuğu (Toolbar)
*   **Mevcut Durum:** Araç çubuğunda araçlar simgeler yerine harflerle (V, M, L, W vb.) temsil edilmektedir. Renk seçici bölümünde sadece üst üste binen renk kutuları vardır.
*   **Photoshop 2026 Standartları:**
    *   **Görsel Simgeler (Icons):** Her araç kendine has vektörel bir simge ile temsil edilir.
    *   **Alt Araç Grupları (Flyout Menus):** Bir araca uzun tıklandığında ilişkili alt araçlar açılır (Örn: Brush aracına basılı tutulduğunda Pencil veya Color Replacement araçlarının çıkması).
    *   **Zengin İpuçları (Rich Tooltips):** Fareyle bir aracın üzerine gelindiğinde, aracın ne işe yaradığını gösteren kısa animasyonlu ipuçları gösterilir.
    *   **Hızlı Renk Değişimi:** Renk seçici üzerinde ön ve arka plan renklerini tek tıkla takas etmeyi (Swap) sağlayan küçük bir ok simgesi (Klavye kısayolu: `X`) ve renkleri siyah-beyaza sıfırlayan varsayılan renk düğmesi (`D`) yer alır.

### 2. Özellikler Çubuğu (Options Bar)
*   **Mevcut Durum:** Tuvalin hemen üstündeki Options Bar oldukça sadedir ve çoğu araç için pasif durumdadır (Örn: "Move Tool | No properties to adjust...").
*   **Photoshop 2026 Standartları:**
    *   Araç seçildiğinde o araca özel tüm ayarlar dinamik olarak listelenir.
    *   **Fırça Modunda:** Fırça ucu önizlemesi, Fırça Çapı (Size), Sertlik (Hardness), Opaklık (Opacity) ve Akış (Flow) sürgüleri ile yumuşatma (Smoothing) ayarı yer alır.
    *   **Move Modunda:** Katmanları sola, ortaya, sağa, yukarı veya aşağı hizalama (Alignment) düğmeleri gösterilir.

### 3. Çoklu Belge Sekmeleri (Multi-tab Document Interface)
*   **Mevcut Durum:** Tek bir tuval ekranı vardır. Aynı anda birden fazla dosya veya proje üzerinde çalışılamaz.
*   **Photoshop 2026 Standartları:**
    *   Üst üste sekmeli yapı sayesinde aynı anda birden fazla görsel açık tutulabilir.
    *   Sekmeler sürüklenerek yan yana koyulabilir veya bağımsız pencereler haline getirilebilir (Floating Windows).

### 4. Gelişmiş Katmanlar Paneli (Layers Panel)
*   **Mevcut Durum:** Katman listesi sadece isimlerden oluşur. blend modu ve opaklık ayarları basittir.
*   **Photoshop 2026 Standartları:**
    *   **Katman Küçük Resimleri (Thumbnails):** Her katman isminin solunda, katmanın içeriğini gösteren dinamik bir önizleme resmi yer alır.
    *   **Katman Gruplama (Groups/Folders):** Katmanlar klasörler altında gruplanabilir ve toplu şekilde taşınabilir.
    *   **Katman Filtreleme:** Arayüzün en üstünde katmanları isme, türe (yazı, şekil, piksel) veya efekt durumuna göre arama ve filtreleme barı yer alır.
    *   **Katman Efektleri (Layer Styles):** Katmana çift tıklandığında Gölge (Drop Shadow), Kontur (Stroke) veya Dış Işıma (Outer Glow) gibi canlı efekt paneli açılır.

### 5. Renk Seçim Paneli (Color Wheel & Swatches)
*   **Mevcut Durum:** Aktif bir renk seçim paneli yer almamaktadır; renk seçimi ImGui'nin standart renk penceresiyle dikey araç çubuğunun altında yapılmaktadır.
*   **Photoshop 2026 Standartları:**
    *   Dinamik Renk Çarkı (Color Wheel), RGB/HSB sürgüleri ve önceden kaydedilmiş renk paleti (Swatches) panelleri ayrı birer pencere olarak sağ tarafta yer alır.

---

## 🛠️ Arayüz Modernizasyon Yol Haritası

Arayüzümüzü daha premium ve Photoshop standartlarına yakın hale getirmek için yol haritasına aşağıdaki arayüz iyileştirme hedefleri eklenmiştir:

### Kısa Vadeli Hedefler (Faz 7-8)
*   **Simge Kütüphanesi Entegrasyonu:** Harfler yerine FontAwesome veya özel tasarlanmış SVG/PNG grafik simgelerinin araç çubuğuna entegre edilmesi.
*   **Etkin Options Bar:** Fırça boyutu, opaklığı ve sertliğini slider olarak üst taraftaki bar üzerinde canlı kontrol edebilme.
*   **Gelişmiş Renk Seçici:** Sağ panele entegre edilmiş bir renk paleti (Color Palette) ve HSB renk çemberi penceresinin eklenmesi.

### Orta Vadeli Hedefler (Faz 9-10)
*   **Dinamik Katman Önizlemeleri (Thumbnails):** CPU/GPU üzerinde katmanın piksellerini ölçeklendirerek katman listesinde küçük resim olarak çizdirme.
*   **Sekmeli Tuval Yapısı:** `std::vector<core::Document>` desteğiyle ImGui sekmeleri (Tabs) kullanarak çoklu dosya açma mimarisini kurma.
