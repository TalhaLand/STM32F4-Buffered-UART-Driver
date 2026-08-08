# STM32F4 Buffered UART Driver

**Interrupt Tabanlı UART Haberleşmesi ve Circular Buffer Mimarisi**

## 📌 Proje Hakkında

Bu proje, STM32F4 mikrodenetleyiciler üzerinde **Embedded C** kullanılarak geliştirilen modüler bir UART haberleşme sürücüsüdür.

Projenin temel amacı, UART haberleşmesini **interrupt tabanlı veri alımı** ve **Circular Buffer** mimarisi kullanarak daha düzenli ve işlemciyi gereksiz yere meşgul etmeyen bir yapıya dönüştürmektir.

UART üzerinden alınan veriler doğrudan uygulama içerisinde işlenmek yerine Circular Buffer içerisine aktarılır. Uygulama katmanı ise ihtiyaç duyduğu zaman bu verileri okuyarak işler.

---

## ✨ Özellikler

* Interrupt tabanlı UART veri alımı
* Circular Buffer implementasyonu
* Non-blocking veri işleme yaklaşımı
* Modüler sürücü mimarisi
* UART ve buffer yönetiminin ayrıştırılması
* Head / Tail tabanlı buffer yönetimi
* Buffer overflow kontrolü
* STM32F4 / ARM Cortex-M4 uyumlu yapı

---

## 🏗️ Yazılım Mimarisi

Projenin temel veri akışı aşağıdaki gibidir:

```text
        UART Peripheral
               │
               ▼
        UART RX Interrupt
               │
               ▼
        Alınan Veri
               │
               ▼
       ┌─────────────────┐
       │ Circular Buffer │
       │                 │
       │  Head → Yazma   │
       │  Tail → Okuma   │
       └─────────────────┘
               │
               ▼
        Application Layer
```

UART interrupt'ı üzerinden alınan veri Circular Buffer içerisine aktarılır.

Application Layer ise buffer içerisindeki veriyi ihtiyaç duyduğu zaman okuyarak işler.

---

## 🔄 Circular Buffer

Circular Buffer, UART üzerinden gelen verilerin belirli bir bellek alanı içerisinde sürekli olarak tutulmasını sağlar.

Buffer içerisinde temel olarak iki indeks kullanılır:

* **Head:** Yeni verinin yazılacağı konumu gösterir.
* **Tail:** Okunacak verinin konumunu gösterir.

Buffer'ın sonuna ulaşıldığında indeks tekrar başlangıç konumuna döner.

```text
        ┌─────────────────────┐
        │                     │
        ▼                     │
   ┌────┬────┬────┬────┬────┐ │
   │    │    │    │    │    │ │
   └────┴────┴────┴────┴────┘ │
        ▲                     │
        └─────────────────────┘
```

Bu yapı sayesinde aynı bellek alanı tekrar tekrar kullanılabilir.

---

## ⚙️ UART Veri Alımı

UART veri alımı interrupt mekanizması üzerinden gerçekleştirilir.

Temel işlem akışı:

```text
UART RX
   │
   ▼
Interrupt
   │
   ▼
Alınan Veriyi Oku
   │
   ▼
Circular Buffer'a Yaz
```

Bu yaklaşım sayesinde ana programın UART verisi beklemek için sürekli polling yapmasına gerek kalmaz.

---

## 💻 Kullanım

Uygulama katmanı buffer içerisindeki veriyi ihtiyaç duyduğu zaman okuyabilir.

Örneğin:

```c
uint8_t data;

if (CircularBuffer_Read(&buffer, &data))
{
    // Alınan veriyi işle
}
```

Kullanılan fonksiyon isimleri ve API yapısı projenin mevcut implementasyonuna göre değişebilir.

---

## 📁 Proje Yapısı

```text
STM32F4-Buffered-UART-Driver/
│
├── Core/
│   ├── Inc/
│   └── Src/
│
├── Drivers/
│   ├── UART/
│   └── CircularBuffer/
│
├── STM32CubeIDE/
│
└── README.md
```

---

## 🎯 Projenin Amacı

Bu proje ile aşağıdaki konularda pratik yapılmıştır:

* Embedded C
* STM32 UART peripheral kullanımı
* Interrupt tabanlı programlama
* Circular Buffer veri yapısı
* Non-blocking haberleşme
* Modüler Embedded Software Architecture
* Sürücü geliştirme

---

## 🚀 Gelecek Geliştirmeler

* DMA tabanlı UART veri alımı
* Command Parser
* Paket/frame parser
* Timeout yönetimi
* Birden fazla UART instance desteği
* Daha gelişmiş hata yönetimi
* Circular Buffer için unit test yapısı

---

## 🛠️ Kullanılan Teknolojiler

* STM32F4
* ARM Cortex-M4
* Embedded C
* UART / USART
* Interrupt
* Circular Buffer
* STM32CubeIDE

---

## 📜 Lisans

Bu proje eğitim ve kişisel geliştirme amacıyla hazırlanmıştır.
