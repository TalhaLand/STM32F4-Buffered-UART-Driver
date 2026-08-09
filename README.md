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

Ardından driver'ı kullanacağınız `.c` dosyasına:

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

> Başka bir UART kullanıyorsanız aynı ayarları kullandığınız UART peripheral'ına uygulayın.

---

## 3. UART Interrupt'ını aktif edin

STM32CubeMX içerisinde:

```text
NVIC
└── USART2 global interrupt → Enabled
```

şeklinde kullandığınız UART'ın global interrupt'ını aktif edin.

---

## 4. RX ve TX Circular Buffer oluşturun

UART driver için bir `UART_Ex_t` değişkeni ve RX/TX için iki ayrı Circular Buffer oluşturun:

```c
UART_Ex_t uart2;

Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;
```

RX ve TX için ayrı buffer kullanılması önemlidir.

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
* `UART_cb_In` → RX Circular Buffer
* `UART_cb_out` → TX Circular Buffer

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

`UARTx_ReadLine()` fonksiyonu mesaj sonunu `\r\n` ile belirler.

---

# 📚 Driver Nasıl Çalışıyor?

Driver'ın temelinde iki yapı bulunmaktadır:

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

RX tarafında veri UART'tan gelir ve interrupt aracılığıyla RX buffer'a aktarılır.

TX tarafında ise uygulamanın göndermek istediği veri önce TX buffer'a eklenir ve interrupt aracılığıyla UART'a gönderilir.

---

# 🔄 Circular Buffer

Circular Buffer, sabit boyutlu bir bellek alanının sürekli olarak tekrar kullanılmasını sağlayan bir buffer yapısıdır.

Bu driver'da Circular Buffer içerisinde iki temel indeks bulunmaktadır:

```c
uint16_t head;
uint16_t tail;
```

### `head`

Yeni verinin yazılacağı konumu gösterir.

### `tail`

Okunacak verinin bulunduğu konumu gösterir.

Örneğin:

```text
        Buffer
┌────┬────┬────┬────┬────┐
│ A  │ B  │ C  │    │    │
└────┴────┴────┴────┴────┘
       ↑         ↑
      tail      head
```

Veri eklendikçe `head`, veri okundukça `tail` ilerler.

Buffer'ın sonuna ulaşıldığında indeks tekrar başlangıca döner.

Bu nedenle yapı **Circular Buffer** olarak adlandırılır.

---

# 📥 RX İşlemi

UART üzerinden yeni bir karakter geldiğinde UART'ın **RXNE (Receive Data Register Not Empty)** durumu oluşur.

Bu durumda UART interrupt handler çalışır.

Temel akış:

```text
UART'tan karakter gelir
        ↓
RXNE Interrupt
        ↓
UART Data Register
        ↓
RX Buffer
        ↓
Uygulama
```

Interrupt içerisinde yapılması gereken işlem mümkün olduğunca kısa tutulur.

---

# ⚡ RXNE Interrupt

UART interrupt handler içerisinde RXNE durumu kontrol edilir:

```c
if(__HAL_UART_GET_FLAG(
        uart2.huart,
        UART_FLAG_RXNE))
{
    ...
}
```

Burada kontrol edilen şey:

> UART'ın Data Register'ında okunmayı bekleyen yeni bir karakter var mı?

sorusudur.

---

## Gelen karakteri okuma

RXNE oluştuğunda UART'ın Data Register'ından karakter okunur:

```c
uint8_t ch;

ch = (uint8_t)(
    uart2.huart->Instance->DR & 0x00FF
);
```

Burada:

```text
DR → Data Register
```

UART üzerinden gelen verinin bulunduğu register'dır.

Örneğin UART üzerinden `A` karakteri geldiğinde:

```text
DR → 0x41
```

olur.

`& 0x00FF` işlemi ile yalnızca 8 bitlik veri alınır ve `ch` değişkenine aktarılır.

---

## Karakteri RX Buffer'a ekleme

UART'tan okunan karakter doğrudan uygulamaya gönderilmez.

Önce RX Circular Buffer'a eklenir:

```c
Circuler_Buffer_Enqueue(
    uart2.cbIn,
    ch
);
```

Böylece interrupt sırasında alınan karakter buffer içerisinde saklanır.

Veri akışı:

```text
UART
 ↓
RXNE Interrupt
 ↓
DR Register
 ↓
ch
 ↓
RX Circular Buffer
 ↓
UARTx_ReadLine()
 ↓
Uygulama
```

Buradaki önemli nokta:

**Interrupt yalnızca veriyi hızlıca alıp buffer'a koyar.**

Mesajın işlenmesi veya uzun süren işlemler interrupt içerisinde yapılmaz.

---

# 📤 TX İşlemi

Uygulama bir veri göndermek istediğinde veri doğrudan UART register'ına yazılmaz.

Önce TX Circular Buffer'a eklenir.

Örneğin:

```c
UARTx_Put_String(
    &uart2,
    "Merhaba STM32\r\n"
);
```

çağrıldığında karakterler TX buffer'a eklenir.

Daha sonra TX interrupt'ı kullanılarak bu karakterler UART'a aktarılır.

Genel akış:

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

---

# ⚡ TXE Interrupt

TX tarafında **TXE (Transmit Data Register Empty)** interrupt'ı kullanılır.

TXE, UART'ın Data Register'ının yeni bir veri almaya hazır olduğunu belirtir.

Interrupt içerisinde TXE durumu kontrol edilir:

```c
if(__HAL_UART_GET_FLAG(
        uart2.huart,
        UART_FLAG_TXE))
{
    ...
}
```

Burada kontrol edilen şey:

> UART'ın Data Register'ı yeni bir karakter almaya hazır mı?

sorusudur.

---

## TX Buffer'ı kontrol etme

Öncelikle TX Circular Buffer'ın boş olup olmadığı kontrol edilir:

```c
if(!circuler_buffer_is_empty(
        uart2.cbOut))
{
    ...
}
```

Buffer boş değilse gönderilecek veri vardır.

---

## Buffer'dan karakter alma

TX buffer içerisinde veri varsa sıradaki karakter alınır:

```c
uint8_t ch;

if(Circuler_Buffer_Dequeue(
        uart2.cbOut,
        &ch))
{
    ...
}
```

`Circuler_Buffer_Dequeue()`:

1. Buffer'daki sıradaki karakteri alır.
2. `tail` indeksini ilerletir.
3. Okunan karakteri `ch` değişkenine aktarır.

---

## UART'a karakter gönderme

Buffer'dan alınan karakter UART'ın Data Register'ına yazılır:

```c
uart2.huart->Instance->DR = ch;
```

Böylece karakter UART üzerinden gönderilmeye başlanır.

---

# 🛑 TXE Interrupt Neden Kapatılıyor?

TX Circular Buffer boşaldığında gönderilecek başka veri kalmamıştır.

Bu durumda TXE interrupt'ın açık kalmasına gerek yoktur.

Eğer TXE interrupt sürekli açık bırakılırsa UART boş olduğu halde sürekli interrupt oluşturabilir.

Bu nedenle:

```text
TX Buffer'da veri var
        ↓
TXE Interrupt → Açık

TX Buffer boş
        ↓
TXE Interrupt → Kapalı
```

şeklinde çalışılır.

Yeni bir veri TX buffer'a eklendiğinde TXE interrupt tekrar aktif edilir.

Bu yapı gereksiz interrupt oluşmasını önler.

---

# 🧠 Interrupt İçerisinde Neden Uzun İşlem Yapılmıyor?

Interrupt fonksiyonları mümkün olduğunca kısa tutulmalıdır.

RX interrupt içerisinde:

```text
✅ UART'tan veriyi oku
✅ RX buffer'a ekle
✅ Interrupt'tan çık
```

TX interrupt içerisinde:

```text
✅ TX buffer'dan karakter al
✅ UART'a yaz
✅ Buffer boşsa TXE'yi kapat
```

yapılması yeterlidir.

Interrupt içerisinde:

```text
❌ printf
❌ HAL_Delay
❌ Uzun hesaplamalar
❌ String işleme
❌ Uzun süren fonksiyonlar
```

gibi işlemler yapılmamalıdır.

Çünkü interrupt çalışırken CPU normal program akışını bırakıp interrupt handler'ı çalıştırır.

Interrupt çok uzun sürerse diğer işlemler gecikebilir ve yüksek UART trafiğinde veri kaçırma riski oluşabilir.

---

# 🧩 UART_Ex_t Yapısı

UART driver'ın temel bilgileri `UART_Ex_t` yapısında tutulmaktadır.

Mantıksal olarak:

```text
UART_Ex_t
│
├── UART HAL Handle
│
├── RX Circular Buffer
│
└── TX Circular Buffer
```

Bu sayede UART peripheral'ı ile ilgili bilgiler tek bir yapı içerisinde tutulur.

Farklı UART peripheral'ları için farklı driver nesneleri oluşturulabilir:

```c
UART_Ex_t uart1;
UART_Ex_t uart2;
```

---

# 🗃️ Circular Buffer Fonksiyonları

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

# 📝 UARTx_ReadLine()

`UARTx_ReadLine()` fonksiyonunun amacı UART üzerinden gelen karakterleri biriktirerek tamamlanmış bir mesaj elde etmektir.

Örneğin bilgisayardan:

```text
LED_ON\r\n
```

geldiğinde karakterler RX buffer'a sırayla girer:

```text
L → E → D → _ → O → N → \r → \n
```

`UARTx_ReadLine()` bu karakterleri takip eder.

`\r\n` algılandığında mesajın tamamlandığını kabul eder ve mesajı kullanıcı tarafından verilen buffer'a aktarır.

Sonuç:

```text
lineBuffer
    ↓
"LED_ON"
```

Bu yapı özellikle UART üzerinden komut gönderilen uygulamalarda kullanılabilir:

```text
LED_ON
LED_OFF
MOTOR_START
MOTOR_STOP
```

---

# 🖨️ UARTx_Printf()

`UARTx_Printf()` fonksiyonu UART üzerinden formatlı veri göndermeyi sağlar.

Örneğin:

```c
UARTx_Printf(
    &uart2,
    "Temperature: %d C\r\n",
    temperature
);
```

Fonksiyon verilen formatı oluşturur ve oluşan string'i TX Circular Buffer'a aktarır.

Bu sayede debug mesajları ve değişken değerleri UART üzerinden kolayca gönderilebilir.

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

# 🔗 Driver ve Interrupt Arasındaki İlişki

Bu projede `UART_ex.c`, `Circuler_Buffer.c` ve `stm32f4xx_it.c` birlikte çalışır.

```text
                 UART Donanımı
                       │
              ┌────────┴────────┐
              │                 │
             RXNE              TXE
              │                 │
              ▼                 ▼
       Interrupt Handler  Interrupt Handler
              │                 │
              ▼                 ▼
          RX Buffer          TX Buffer
              │                 │
              ▼                 ▼
       UARTx_ReadLine()    UARTx_Printf()
       UARTx_Read...()     UARTx_Put_String()
```

### `UART_ex.c`

Uygulamanın kullanacağı UART fonksiyonlarını içerir.

Örneğin:

```text
UARTx_Initialization()
UARTx_Write_Char()
UARTx_Put_String()
UARTx_Printf()
UARTx_ReadLine()
```

### `Circuler_Buffer.c`

Circular Buffer'ın veri ekleme, veri çıkarma ve durum kontrol işlemlerini gerçekleştirir.

### `stm32f4xx_it.c`

UART interrupt'larını işler.

RXNE geldiğinde:

```text
UART → RX Buffer
```

TXE geldiğinde:

```text
TX Buffer → UART
```

işlemini gerçekleştirir.

---

# 🧠 Neden HAL Fonksiyonları Yerine Register Kullanıldı?

Bu projede interrupt içerisinde bazı UART işlemleri doğrudan UART register'ları üzerinden gerçekleştirilmiştir.

Örneğin:

```c
uart2.huart->Instance->DR = ch;
```

ve:

```c
ch = (uint8_t)(
    uart2.huart->Instance->DR & 0x00FF
);
```

Buradaki:

```text
DR → Data Register
```

UART'ın veri register'ıdır.

Bu yaklaşımın amacı UART peripheral'ının register seviyesindeki çalışma mantığını öğrenmek ve interrupt içerisinde mümkün olduğunca doğrudan ve hızlı bir veri aktarımı gerçekleştirmektir.

Dolayısıyla bu proje sadece:

```text
UART + HAL
```

kullanımını değil;

```text
UART
 +
Register
 +
Interrupt
 +
Circular Buffer
 +
Driver Abstraction
```

yapısını öğrenmek amacıyla geliştirilmiştir.

---

# 📦 Buffer Boyutu

Circular Buffer boyutu:

```c
#define circuler_buffer_size 512
```

olarak belirlenmiştir.

Bu değer projenin ihtiyacına göre değiştirilebilir.

Daha küçük mesajlar ve düşük veri trafiği için daha küçük bir buffer kullanılabilir.

Daha yoğun UART trafiğinde ise daha büyük bir buffer tercih edilebilir.

---

# ⚠️ Dikkat Edilmesi Gerekenler

* RX ve TX için ayrı Circular Buffer kullanılmalıdır.
* UART global interrupt aktif olmalıdır.
* UART TX/RX pinleri doğru yapılandırılmalıdır.
* `UARTx_ReadLine()` kullanılıyorsa terminalin satır sonu `CRLF (\r\n)` olarak ayarlanmalıdır.
* Buffer boyutu uygulamanın veri trafiğine uygun seçilmelidir.
* Yüksek UART trafiğinde buffer'ın dolup dolmadığı kontrol edilmelidir.
* Interrupt içerisinde uzun süren işlemler yapılmamalıdır.

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

Aynı zamanda proje; **UART register'ları, interrupt mekanizması, Circular Buffer ve driver abstraction** konularını pratik olarak öğrenmek amacıyla geliştirilmiştir.
