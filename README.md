# STM32F4 Buffered UART Driver

STM32F4 mikrodenetleyicilerinde UART haberleşmesini **Interrupt** ve **Circular Buffer** kullanarak gerçekleştiren bir UART sürücüsüdür.

Driver; UART üzerinden veri alma/gönderme, string gönderme, `printf` benzeri formatlı veri gönderme ve gelen verileri satır halinde okuma işlemlerini destekler.

---

# 🚀 Hızlı Başlangıç

Driver'ı kendi STM32CubeIDE projenize dahil etmek için aşağıdaki adımları uygulayın.

## 1. Driver dosyalarını projeye ekleyin

Aşağıdaki dosyaları projenize dahil edin:

```text
UART_ex.c
UART_ex.h
Circuler_Buffer.c
Circuler_Buffer.h
```

Ardından driver'ı kullanacağınız dosyaya:

```c
#include "UART_ex.h"
```

ekleyin.

---

## 2. STM32CubeMX UART ayarlarını yapın

Kullanacağınız UART peripheral'ını aktif edin.

Örneğin USART2:

```text
Baud Rate   → 115200
Word Length → 8 Bits
Parity      → None
Stop Bits   → 1
Mode        → TX/RX
```

---

## 3. UART Interrupt'ını aktif edin

STM32CubeMX içerisinde:

```text
NVIC
└── USART2 global interrupt → Enabled
```

şeklinde kullandığınız UART'ın interrupt'ını aktif edin.

> Başka bir UART kullanıyorsanız ilgili UART'ın global interrupt'ını aktif edin.

---

## 4. RX ve TX Circular Buffer oluşturun

UART için bir driver nesnesi ve RX/TX için iki ayrı Circular Buffer oluşturun:

```c
UART_Ex_t uart2;

Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;
```

---

## 5. UART Driver'ı başlatın

STM32CubeMX tarafından oluşturulan UART handle'ını driver'a gönderin:

```c
UARTx_Initialization(
    &uart2,
    &huart2,
    &UART_cb_In,
    &UART_cb_out
);
```

Burada:

* `uart2` → UART driver yapısı
* `huart2` → STM32 HAL UART handle'ı
* `UART_cb_In` → RX buffer
* `UART_cb_out` → TX buffer

olarak kullanılır.

---

## 6. UART'ı kullanın

### Karakter gönderme

```c
UARTx_Write_Char(&uart2, 'A');
```

### String gönderme

```c
UARTx_Put_String(&uart2, "Merhaba STM32\r\n");
```

### Formatlı veri gönderme

```c
UARTx_Printf(
    &uart2,
    "ADC Value: %d\r\n",
    adcValue
);
```

### Gelen mesajı satır olarak okuma

```c
char lineBuffer[128];

if(UARTx_ReadLine(
        &uart2,
        lineBuffer,
        sizeof(lineBuffer)))
{
    // Yeni mesaj geldi.
}
```

> `UARTx_ReadLine()` fonksiyonu mesaj sonunu `\r\n` ile belirler. Terminal programında satır sonu olarak `CRLF` kullanılması önerilir.

---

# 📚 Driver Nasıl Çalışıyor?

Bu driver'ın temelinde iki yapı bulunuyor:

```text
UART Interrupt
       +
Circular Buffer
```

Amaç, UART üzerinden gelen ve giden verileri doğrudan uygulama içerisinde yönetmek yerine buffer'lar üzerinden yönetmektir.

Genel yapı:

```text
                  STM32
                    │
           ┌────────┴────────┐
           │                 │
          RX                TX
           │                 │
           ▼                 ▼
     RX Circular       TX Circular
        Buffer             Buffer
           │                 │
           ▼                 ▼
       Uygulama            UART
```

---

# 🔄 Circular Buffer Nedir?

Circular Buffer, bellekteki sabit boyutlu bir alanın sürekli olarak tekrar kullanılmasını sağlayan bir buffer yapısıdır.

Bu driver'da her Circular Buffer içerisinde iki temel indeks bulunur:

```c
uint16_t head;
uint16_t tail;
```

### `head`

Yeni verinin yazılacağı konumu gösterir.

### `tail`

Okunacak verinin bulunduğu konumu gösterir.

Veri eklendikçe `head`, veri okundukça `tail` ilerler.

Buffer'ın sonuna gelindiğinde indeks tekrar buffer'ın başına döner.

Bu nedenle yapı "Circular Buffer" olarak adlandırılır.

---

# 📥 RX İşlemi Nasıl Çalışıyor?

UART üzerinden yeni bir karakter geldiğinde UART'ın **RXNE interrupt'ı** tetiklenir.

Interrupt içerisinde gelen karakter UART register'ından alınır ve RX Circular Buffer'a eklenir.

Akış:

```text
UART'tan karakter gelir
        ↓
RXNE Interrupt
        ↓
Karakter UART'tan okunur
        ↓
RX Circular Buffer'a eklenir
        ↓
Uygulama daha sonra buffer'dan okur
```

Böylece uygulamanın sürekli olarak:

```c
"UART'tan veri geldi mi?"
```

diye kontrol etmesine gerek kalmaz.

Gelen veri buffer'da bekler.

---

# 📤 TX İşlemi Nasıl Çalışıyor?

Uygulama bir veri göndermek istediğinde veri doğrudan UART register'ına yazılmaz.

Önce TX Circular Buffer'a eklenir.

Örneğin:

```c
UARTx_Put_String(
    &uart2,
    "Merhaba\r\n"
);
```

çağrıldığında karakterler TX buffer'a eklenir.

Daha sonra UART'ın **TXE interrupt'ı** kullanılarak buffer içerisindeki karakterler sırayla UART'a gönderilir.

Akış:

```text
Uygulama
   ↓
TX Circular Buffer
   ↓
TXE Interrupt
   ↓
UART Data Register
   ↓
UART TX
```

TX buffer boşaldığında TX interrupt'ı devre dışı bırakılır.

Bu sayede UART gönderimi sırasında CPU'nun sürekli beklemesi gerekmez.

---

# 🧩 UART_Ex_t Yapısı

UART driver'ın temel bilgileri `UART_Ex_t` yapısında tutulur.

Bu yapı UART HAL handle'ını ve RX/TX buffer'larının adreslerini bir arada tutar.

Temel mantık:

```text
UART_Ex_t
│
├── UART Handle
│
├── RX Circular Buffer
│
└── TX Circular Buffer
```

Böylece farklı UART peripheral'ları için ayrı driver nesneleri oluşturulabilir.

Örneğin:

```c
UART_Ex_t uart1;
UART_Ex_t uart2;
```

---

# 🗃️ Circular Buffer Fonksiyonları

Circular Buffer driver'ı temel olarak veri ekleme ve veri okuma işlemlerini gerçekleştirir.

## Veri ekleme

```c
Circuler_Buffer_Enqueue(
    &buffer,
    data
);
```

Veriyi buffer'a ekler.

Buffer doluysa veri eklenemez.

---

## Veri okuma

```c
Circuler_Buffer_Dequeue(
    &buffer,
    &data
);
```

Buffer'daki sıradaki veriyi okur ve buffer'dan çıkarır.

Buffer boşsa veri okunamaz.

---

## Buffer boş mu?

```c
circuler_buffer_is_empty(&buffer);
```

Buffer'ın boş olup olmadığını kontrol eder.

---

## Buffer dolu mu?

```c
circuler_buffer_is_fully(&buffer);
```

Buffer'ın dolu olup olmadığını kontrol eder.

---

## Buffer'daki veri sayısı

```c
Circuler_Buffer_Count(&buffer);
```

Buffer içerisinde kaç adet veri bulunduğunu döndürür.

---

# 📝 UARTx_ReadLine() Nasıl Çalışıyor?

`UARTx_ReadLine()` fonksiyonunun amacı UART üzerinden gelen karakterleri biriktirerek tamamlanmış bir mesaj elde etmektir.

Örneğin bilgisayardan:

```text
LED_ON\r\n
```

geldiğinde karakterler sırayla RX buffer'a girer:

```text
L → E → D → _ → O → N → \r → \n
```

`UARTx_ReadLine()` bu karakterleri takip eder.

`\r\n` algılandığında mesajın tamamlandığını kabul eder:

```text
LED_ON
```

ve bunu kullanıcı tarafından verilen `lineBuffer` içerisine aktarır.

Bu yapı sayesinde UART üzerinden basit bir komut sistemi oluşturulabilir:

```text
LED_ON
LED_OFF
MOTOR_START
MOTOR_STOP
```

---

# 🖨️ UARTx_Printf() Nasıl Çalışıyor?

`UARTx_Printf()` fonksiyonu UART üzerinden formatlı veri göndermeyi sağlar.

Örneğin:

```c
UARTx_Printf(
    &uart2,
    "Temperature: %d C\r\n",
    temperature
);
```

çağrıldığında fonksiyon formatlı string'i oluşturur ve daha sonra UART TX buffer'a aktarır.

Böylece uygulama içerisinde debug mesajları veya değişken değerleri kolayca UART üzerinden gönderilebilir.

Örneğin:

```c
UARTx_Printf(
    &uart2,
    "X: %d | Y: %d | Z: %d\r\n",
    x,
    y,
    z
);
```

---

# 🧠 Neden Circular Buffer Kullanıldı?

Normal UART kullanımında CPU veri gönderirken veya alırken beklemek zorunda kalabilir.

Örneğin blocking bir UART gönderiminde:

```text
CPU
 │
 ├── UART gönderimini başlat
 │
 ├── Bekle
 │
 ├── Bekle
 │
 └── Veri gönderildi
```

Interrupt + Circular Buffer yapısında ise:

```text
CPU
 │
 ├── Veriyi TX Buffer'a koy
 │
 └── Diğer işlemlere devam et
             │
             ▼
       TX Interrupt
             │
             ▼
          UART
```

Bu nedenle özellikle sürekli UART trafiği bulunan uygulamalarda daha kullanışlı bir yapı elde edilir.

---

# 📦 Buffer Boyutu

Circular Buffer boyutu:

```c
#define circuler_buffer_size 512
```

olarak belirlenmiştir.

Bu değer uygulamanın ihtiyacına göre değiştirilebilir.

Örneğin daha küçük mesajlar kullanılacaksa daha küçük bir buffer tercih edilebilir.

Daha fazla UART verisinin bekletilmesi gerekiyorsa buffer boyutu artırılabilir.

---

# ⚠️ Dikkat Edilmesi Gerekenler

* RX ve TX için ayrı Circular Buffer kullanılmalıdır.
* UART global interrupt aktif olmalıdır.
* UART'ın TX/RX pinleri doğru yapılandırılmalıdır.
* `UARTx_ReadLine()` kullanılıyorsa terminalin satır sonu `CRLF (\r\n)` olarak ayarlanmalıdır.
* Buffer boyutu uygulamanın veri trafiğine uygun seçilmelidir.
* Çok yüksek UART trafiğinde buffer'ın dolup dolmadığı kontrol edilmelidir.

---

# 📂 Proje Yapısı

```text
STM32F4-Buffered-UART-Driver
│
├── UART_ex.c
├── UART_ex.h
├── Circuler_Buffer.c
├── Circuler_Buffer.h
└── README.md
```

---

# 🛠️ Kullanılan Teknolojiler

* STM32F4
* ARM Cortex-M4
* Embedded C
* STM32 HAL
* UART / USART
* Interrupt
* Circular Buffer
* STM32CubeMX
* STM32CubeIDE

---

# 🎯 Projenin Amacı

Bu proje STM32F4 üzerinde UART haberleşmesini **Interrupt ve Circular Buffer** kullanarak gerçekleştirmeyi ve farklı projelerde tekrar kullanılabilecek bir UART driver geliştirmeyi amaçlamaktadır.

Driver'ın temel amacı UART haberleşmesini uygulama kodundan ayırmak, veri alışverişini buffer'lar üzerinden yönetmek ve UART işlemlerinin CPU'yu mümkün olduğunca az meşgul etmesini sağlamaktır.
