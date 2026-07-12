# Graphite Studio — İkon Yükleme ve Arayüz Güncelleme Rehberi

Proje dizininde `resources/icons/` altında tüm araçlar, göz simgesi, kilit simgesi ve renk düğmeleri için placeholder SVG dosyaları oluşturulmuştur. 

Bu rehber, bu SVG dosyalarını (veya PNG versiyonlarını) OpenGL dokuları olarak yükleyip ImGui üzerinde simgeler halinde nasıl gösterebileceğinizi adım adım açıklar.

---

## 🛠️ Yöntem 1: PNG / Doku Olarak Yükleme (En Basit ve Standart Yöntem)

İkonları SVG yerine PNG formatına dönüştürüp (örn. 24x24 piksel boyutu) uygulamada `core::GLTexture` sınıfı yardımıyla yükleyebilirsiniz.

### 1. Doku Yükleyici Yardımcı Fonksiyonu Yazma
`src/gui/Toolbar.cpp` veya yeni bir yardımcı sınıf içine, PNG dosyasını diske okuyup bir `core::GLTexture` nesnesi döndüren bir fonksiyon ekleyin:

```cpp
#include "thirdparty/stb_image.h" // stb_image zaten projede bulunmaktadır

std::unique_ptr<core::GLTexture> LoadIconTexture(const std::string& filename) {
    std::string path = "resources/icons/" + filename;
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load icon: " << path << std::endl;
        return nullptr;
    }

    auto tex = std::make_unique<core::GLTexture>();
    tex->Create(GL_TEXTURE_2D);
    tex->AllocateStorage2D(1, GL_RGBA8, w, h);
    
    // Unpack alignment'ı 1 yapıyoruz
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    tex->UploadSubImage2D(0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    tex->SetParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    tex->SetParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}
```

### 2. İkonları Toolbar'da Tutma ve Çizdirme
Dikey araç çubuğu (`Toolbar` sınıfı) başlatılırken ikon dokularını yükleyin:

```cpp
// Toolbar.h içinde üye değişkenler
std::vector<std::unique_ptr<core::GLTexture>> m_IconTextures;

// Toolbar.cpp içinde yükleme (Init aşamasında):
for (int i = 0; i < s_ToolCount; ++i) {
    m_IconTextures.push_back(LoadIconTexture(s_Tools[i].filename)); // filename: "move.png" vb.
}
```

Düğmeleri çizerken `ImGui::Button` yerine `ImGui::ImageButton` kullanın:

```cpp
// ImGui'ye dokunun ID'sini geçerek çizim yapma
GLuint texId = m_IconTextures[i] ? m_IconTextures[i]->GetId() : 0;
if (ImGui::ImageButton(reinterpret_cast<void*>(static_cast<intptr_t>(texId)), ImVec2(buttonSize, buttonSize))) {
    m_SelectedTool = i;
}
```

---

## 🎨 Yöntem 2: NanoSVG ile Çalışma Esnasında Rasterize Etme (SVG Çözümü)

Uygulamanın SVG dosyalarını doğrudan okumasını istiyorsanız, son derece hafif tek dosyalık header kütüphanesi olan **NanoSVG**'yi kullanabilirsiniz:

1.  [NanoSVG GitHub](https://github.com/memononen/nanosvg) adresinden `nanosvg.h` ve `nanosvgrast.h` dosyalarını `thirdparty/` dizinine yerleştirin.
2.  SVG dosyasını bellek üzerinde PNG'ye dönüştürüp OpenGL dokusuna yükleyin:

```cpp
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

std::unique_ptr<core::GLTexture> LoadSvgTexture(const std::string& filename, int targetWidth, int targetHeight) {
    std::string path = "resources/icons/" + filename;
    NSVGimage* svgImage = nsvgParseFromFile(path.c_str(), "px", 96.0f);
    if (!svgImage) return nullptr;

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    std::vector<unsigned char> imgData(targetWidth * targetHeight * 4);
    
    // SVG çizimini belleğe rasterize etme
    nsvgRasterize(rast, svgImage, 0, 0, 1, imgData.data(), targetWidth, targetHeight, targetWidth * 4);

    auto tex = std::make_unique<core::GLTexture>();
    tex->Create(GL_TEXTURE_2D);
    tex->AllocateStorage2D(1, GL_RGBA8, targetWidth, targetHeight);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    tex->UploadSubImage2D(0, 0, 0, targetWidth, targetHeight, GL_RGBA, GL_UNSIGNED_BYTE, imgData.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(svgImage);
    return tex;
}
```

---

## 🔒 imgui.ini Sıfırlama Uyarısı

> [!WARNING]
> ImGui, pencerelerin konum ve dock durumlarını önbellek olarak `imgui.ini` dosyasında saklar.
> Proje kök dizinindeki `imgui.ini` dosyasını sildiğinizde, **Color** panelinin Properties ve History ile aynı sağ üst sekmeye (`dock_right_top`) yerleştiği varsayılan temiz düzen otomatik olarak aktifleşecektir.

---

## 🤖 AI SVG Creator için Profesyonel İkon Promptları

Aşağıdaki prompt kütüphanesini, SVG/Vector üreten AI modellerinde (örneğin Midjourney, DALL-E 3 Vector Mode, Recraft.ai veya özel SVG jeneratörleri) kullanarak mükemmel uyumlu ikon setleri oluşturabilirsiniz. 

### Ortak Stil Parametreleri (Prompt sonuna eklenebilir):
`flat vector icon, minimalist outline design, professional software UI, monoline vector, stroke width 1.5, solid fill, black and white / grayscale, transparent background, svg format, asset, 24x24 px grid --no shadows gradients colors`

### İkon Bazlı Prompt Kütüphanesi:

| Dosya Adı | İkon Tanımı | Profesyonel AI Promptu |
|---|---|---|
| `move.svg` | Taşıma Aracı (Dört Yönlü Ok) | `Flat vector icon of a four-way move cross cursor, minimalist outline design, black outline, transparent background, SVG vector icon, professional UI, 24x24 px` |
| `marquee.svg` | Seçim Çerçevesi (Kesikli Kare) | `Flat vector icon of a dashed square marquee selection tool, minimalist outline, transparent background, SVG vector, 24x24 px` |
| `lasso.svg` | Kement Seçimi (İp Döngüsü) | `Flat vector icon of a lasso selection tool, looping rope vector shape, minimalist outline, transparent background, SVG vector, 24x24 px` |
| `magic_wand.svg` | Sihirli Değnek (Yıldızlı Çubuk) | `Flat vector icon of a magic wand tool with small sparkles at the tip, minimalist vector outline, transparent background, SVG vector, 24x24 px` |
| `crop.svg` | Kırpma Aracı (Köşeli Kare) | `Flat vector icon of a photo cropping tool icon, overlapping frame corners, minimalist vector outline, transparent background, SVG vector, 24x24 px` |
| `eyedropper.svg` | Damlalık (Renk Seçici) | `Flat vector icon of a chemical dropper pipette eyedropper tool, angled outline, transparent background, SVG vector, 24x24 px` |
| `healing_brush.svg` | Düzeltme Fırçası (Yara Bandı) | `Flat vector icon of a cross bandage patch adhesive bandage tool, minimalist vector outline, transparent background, SVG vector, 24x24 px` |
| `brush.svg` | Fırça Aracı (Boya Fırçası) | `Flat vector icon of an artist paint brush with hair tip, angled vector outline, transparent background, SVG vector, 24x24 px` |
| `clone_stamp.svg` | Klonlama Damgası | `Flat vector icon of a manual ink stamp tool, stamp seal vector outline, minimalist, transparent background, SVG vector, 24x24 px` |
| `history_brush.svg` | Geçmiş Fırçası (Geri Dönüş Oku) | `Flat vector icon of a circular undo arrow combined with a brush tip, vector outline, transparent background, SVG vector, 24x24 px` |
| `eraser.svg` | Silgi Aracı | `Flat vector icon of a rectangular rubber eraser tool, angled vector block outline, transparent background, SVG vector, 24x24 px` |
| `gradient.svg` | Degrade Geçiş Kutusu | `Flat vector icon of a rectangular gradient transition filled box, linear half halftone pattern, transparent background, SVG vector, 24x24 px` |
| `dodge.svg` | Renk Açma (İğne Daire) | `Flat vector icon of a photography dodge tool wand, circle outline on a stick, transparent background, SVG vector, 24x24 px` |
| `pen.svg` | Kalem Aracı (Dolmakalem Ucu) | `Flat vector icon of a calligraphy fountain pen nib tool, sharp vector outline, transparent background, SVG vector, 24x24 px` |
| `text.svg` | Yazı Aracı (Büyük T Harfi) | `Flat vector icon of a serif capital letter T text tool, clean typography vector outline, transparent background, SVG vector, 24x24 px` |
| `path_selection.svg` | Yol Seçimi (İçi Dolu Ok) | `Flat vector icon of a solid dark arrow pointer cursor, path selection tool, transparent background, SVG vector, 24x24 px` |
| `shape.svg` | Şekil Aracı (Düz Kare) | `Flat vector icon of a simple geometry square box shape tool, vector outline, transparent background, SVG vector, 24x24 px` |
| `hand.svg` | El Aracı (Kaydırma) | `Flat vector icon of an open human hand palm, hand tool cursor outline, transparent background, SVG vector, 24x24 px` |
| `zoom.svg` | Büyüteç (Yakınlaştırma) | `Flat vector icon of a magnifying glass zoom tool lens, angled vector outline, transparent background, SVG vector, 24x24 px` |
| `eye_open.svg` | Açık Göz (Görünür Katman) | `Flat vector icon of an open human eye, visibility on status, minimalist vector outline, transparent background, SVG vector, 24x24 px` |
| `eye_closed.svg` | Kapalı Göz (Gizli Katman) | `Flat vector icon of a closed human eye with a slash line, invisible status, transparent background, SVG vector, 24x24 px` |
| `lock.svg` | Kilitli (Locked) | `Flat vector icon of a closed secure padlock, locked layer status, vector outline, transparent background, SVG vector, 24x24 px` |
| `unlock.svg` | Kilitsiz (Unlocked) | `Flat vector icon of an open padlock, unlocked layer status, vector outline, transparent background, SVG vector, 24x24 px` |
| `swap_color.svg` | Renk Takası (İki Yönlü Ok) | `Flat vector icon of a double curved swap arrow, swap foreground background colors, vector outline, transparent background, SVG vector, 12x12 px` |
| `reset_color.svg` | Renk Sıfırlama (Çift Kare) | `Flat vector icon of two tiny overlapping black and white squares, reset default colors, vector outline, transparent background, SVG vector, 12x12 px` |

