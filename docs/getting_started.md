# Graphite Studio — Başlangıç Rehberi (Getting Started)

Bu doküman, Graphite Studio projesini derlemek, çalıştırmak ve projeye katkıda bulunmak isteyen geliştiriciler için temel rehberdir.

---

## 🛠️ Derleme ve Çalıştırma

### Gereksinimler

- **İşletim Sistemi:** Windows 10/11, macOS veya popüler Linux dağıtımları.
- **Modern C++ Derleyicisi:** C++20 standardını tam destekleyen bir derleyici:
  - MSVC (Visual Studio 2022 / 2026 veya üzeri)
  - GCC 11+
  - Clang 12+
- **CMake:** Sürüm 3.20 veya üzeri.
- **Ekran Kartı & Sürücüler:** OpenGL 4.6 Core Profile destekleyen grafik donanımı.

---

## 💻 Geliştirici Ortamı Kurulumu (Windows)

### 1. Depoyu Klonlama ve Dizin Değiştirme
```bash
git clone https://github.com/user/GraphiteStudio.git
cd GraphiteStudio
```

### 2. Proje Yapılandırması (Configure)
CMake, sisteminizde kurulu olan derleyici araç setini otomatik tespit edecektir. Windows üzerinde Visual Studio 2026 Build Tools kullanmak için:
```bash
cmake -G "Visual Studio 18 2026" -A x64 -B build
```

### 3. Derleme (Build)
Projeyi en yüksek optimizasyon seviyesinde derlemek için Release konfigürasyonunu seçin:
```bash
cmake --build build --config Release
```
Bu komut sonucunda çalıştırılabilir dosya `build/Release/GraphiteStudio.exe` dizininde oluşturulacaktır.

### 4. Uygulamayı Çalıştırma
```bash
./build/Release/GraphiteStudio.exe
```

---

## 🤝 Katkı Sağlama Rehberi (Contribution)

Graphite Studio'ya katkıda bulunurken lütfen aşağıdaki kodlama standartlarına uyunuz:

1.  **Modern C++ Pratikleri:**
    - Ham işaretçiler (`new`/`delete`) yerine akıllı işaretçiler (`std::unique_ptr`, `std::shared_ptr`) veya RAII kaynak yönetimi kullanın.
    - Sorumlulukların Ayrılması İlkesine (SRP) uyun. Yeni bir işlev eklerken bunu doğrudan `CanvasView` veya `Application` içine yazmak yerine ilgili sınıflara bölün.
    - Mümkünse Sıfır Kuralı (Rule of Zero) uygulayarak kaynak yönetimi sarmalayıcılarından yararlanın.
2.  **OpenGL / Grafik Kodları:**
    - Asla klasik binding API'lerini (`glBindTexture`, `glGenTextures`) kullanmayın.
    - Tüm grafik güncellemelerini Direct State Access (DSA) fonksiyonları (`glCreateTextures`, `glTextureStorage2D`, `glBindTextureUnit`) ile yapın.
3.  **Temizlik:**
    - Kodlarınızı göndermeden önce mutlaka derleme testi yapın ve kullanılmayan değişkenleri/kütüphaneleri temizleyin.
