# Graphite Studio — GPU ve OpenGL Alt Sistemi

Graphite Studio, OpenGL 4.6 Core Profile standartlarına dayalı, tamamen **Direct State Access (DSA)** mimarisiyle tasarlanmış modern bir GPU işleme altyapısına sahiptir.

---

## 🚀 Direct State Access (DSA) Avantajları

Klasik OpenGL (sürüm 4.5 öncesi), kaynakları manipüle etmek için önce hedef bağlama noktasına (global binding target) bağlamayı (`glBindTexture`, `glBindFramebuffer` vb.) zorunlu kılıyordu. Bu durum şu problemlere yol açıyordu:
1.  **Global Durum Kirliliği (Global State Pollution):** Bir modülde bağlanan doku veya buffer, başka bir yerde beklenmeyen çizim hatalarına yol açabiliyordu.
2.  **ImGui Kararsızlıkları:** Dear ImGui kendi dahili OpenGL durumunu korumaya çalışır. Küresel durum kirliliği, ImGui render döngüsünü bozabiliyordu.
3.  **Performans Kayıpları:** Sürekli bind/unbind işlemleri GPU sürücüsüne ciddi yük bindiriyordu.

**DSA mimarisi ile:**
- Kaynaklar doğrudan kendi ID'leri (Named Objects) üzerinden güncellenir.
- Örneğin; `glBindTexture` + `glTexImage2D` yerine `glTextureStorage2D` + `glTextureSubImage2D` kullanılır.
- Uygulama içindeki doku birimlerine bağlama yaparken `glActiveTexture` + `glBindTexture` yerine `glBindTextureUnit` doğrudan kullanılır. Bu sayede aktif doku birimi seçicisi (active texture selector) değişmeden kalır.

---

## 🔒 RAII Kaynak Sarmalayıcıları (Resource Management)

OpenGL kaynaklarının yaşam döngüsünü C++ standartlarında güvenli hale getirmek için move-only (kopyalanamaz, taşınabilir) RAII sınıfları yazılmıştır:

### 1. `core::GLBuffer`
GPU üzerindeki vertex, index veya uniform veri belleklerini sarmalar.
- `glCreateBuffers` ile oluşturulur.
- `glNamedBufferStorage` ile immutable (değiştirilemez) veya dinamik GPU bellek alanı tahsis edilir.

### 2. `core::GLVertexArray`
Vertex attribute biçimlendirmelerini (VAO) yönetir.
- `glCreateVertexArrays` ile oluşturulur.
- `glVertexArrayVertexBuffer` ve `glVertexArrayAttribFormat` ile state-free attribute bağlantıları kurulur.

### 3. `core::GLTexture`
2D doku kaynaklarını sarmalar.
- `glCreateTextures` ile oluşturulur.
- `glTextureStorage2D` ile GPU belleği bir kez tahsis edilir (Immutable Storage).
- `glTextureSubImage2D` ile pikseller doğrudan dokuya yazılır.

### 4. `core::GLFramebuffer`
Çevrimdışı (offscreen) çizim hedeflerini sarmalar.
- `glCreateFramebuffers` ile oluşturulur.
- `glNamedFramebufferTexture` ile dokular FBO attachment'larına bağlanır.

---

## 🛡️ Rule of Zero (Sıfır Kuralı) Entegrasyonu

Tüm OpenGL sarmalayıcıları (`GLTexture`, `GLFramebuffer` vb.) kendi yıkıcı metotlarında kaynak temizliğini otomatik yapar. Bu sayede bu nesneleri üye olarak tutan üst sınıflarda (`core::Layer`, `core::Compositor`) manuel kaynak silme (`glDelete*`) kodlarına ihtiyaç kalmaz.
Sınıfların move constructor ve move assignment operatörleri derleyici tarafından otomatik üretilir (`= default`), bu da insan kaynaklı bellek ve kaynak sızıntısı (resource leaks) riskini sıfıra indirir.

---

## 🚀 glGetTextureSubImage ile Bağlantısız Okuma

Belgeyi diske kaydetme (`SaveCompositeToFile`) aşamasında FBO'yu `GL_FRAMEBUFFER` hedefine bağlayıp `glReadPixels` yapmak yerine, `glGetTextureSubImage` kullanılmıştır:
```cpp
glGetTextureSubImage(
    textureId, 
    0,          // Level 0
    0, 0, 0,    // offsets (x, y, z)
    m_Width, m_Height, 1, // dimensions (w, h, d)
    GL_RGBA, GL_UNSIGNED_BYTE, 
    pixels.size(), pixels.data()
);
```
Bu fonksiyon, OpenGL'in küresel durumunu hiç değiştirmeden, belirtilen dokudaki pikselleri doğrudan CPU belleğine kopyalar. En güvenli, en modern ve en hızlı doku okuma yöntemidir.
