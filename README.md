# STM32F4 Buffered UART Driver

STM32F4 mikrodenetleyicilerinde UART haberleşmesini **Circular Buffer** ve **interrupt** kullanarak gerçekleştiren bir UART sürücüsüdür.

Bu driver ile UART üzerinden gelen ve gönderilen veriler buffer'lar kullanılarak yönetilir. Böylece CPU'nun UART üzerinden veri gönderirken veya alırken sürekli olarak beklemesi yerine, veri aktarımı interrupt'lar üzerinden gerçekleştirilir.

Driver ayrıca `printf` benzeri formatlı veri gönderimini ve UART üzerinden gelen mesajların satır halinde okunmasını desteklemektedir.

---

# 📌 Özellikler

* UART RX işlemlerinde interrupt kullanımı
* UART TX işlemlerinde interrupt kullanımı
* RX ve TX için ayrı Circular Buffer kullanımı
* Karakter gönderme
* String gönderme
* `printf` benzeri formatlı veri gönderme
* UART üzerinden gelen verileri satır halinde okuma
* Buffer'ın dolu ve boş durumunu kontrol etme
* Buffer içerisindeki veri sayısını öğrenme
* STM32 HAL ile uyumlu çalışma

---

# 🧠 Çalışma Mantığı

Driver içerisinde UART için iki farklı Circular Buffer kullanılmaktadır:

```text
              UART
               │
       ┌───────┴───────┐
       │               │
      RX              TX
       │               │
       ▼               ▼
  RX Buffer        TX Buffer
       │               │
       ▼               ▼
  Uygulama          UART Donanımı
```

### RX tarafı

UART üzerinden bir karakter geldiğinde `RXNE` interrupt'ı oluşur.

Gelen karakter UART register'ından okunarak RX Circular Buffer'a eklenir.

```text
UART RX
   ↓
RXNE Interrupt
   ↓
UART Data Register
   ↓
RX Circular Buffer
   ↓
Uygulama
```

### TX tarafı

Uygulama bir karakter veya string göndermek istediğinde veri doğrudan UART'a gönderilmez.

Önce TX Circular Buffer'a eklenir.

Daha sonra `TXE` interrupt'ı kullanılarak buffer içerisindeki veriler UART donanımına aktarılır.

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

Bu yapı sayesinde uygulama UART donanımını beklemek zorunda kalmadan çalışmaya devam edebilir.

---

# 📁 Projeye Dahil Etme

Driver'ı kendi STM32CubeIDE projenize dahil etmek için aşağıdaki dosyaları projenize ekleyin:

```text
UART_ex.c
UART_ex.h

Circuler_Buffer.c
Circuler_Buffer.h
```

Örneğin proje yapısı:

```text
MyProject
│
├── Core
│   ├── Inc
│   └── Src
│
├── Drivers
│   └── UART
│       ├── UART_ex.c
│       ├── UART_ex.h
│       ├── Circuler_Buffer.c
│       └── Circuler_Buffer.h
│
└── ...
```

Driver'ı kullanacağınız dosyaya:

```c
#include "UART_ex.h"
```

ekleyin.

---

# ⚙️ STM32CubeMX Ayarları

Öncelikle STM32CubeMX üzerinden kullanacağınız UART peripheral'ını aktif edin.

Örnek olarak USART2 kullanılabilir.

UART ayarları:

```text
Baud Rate   → 115200
Word Length → 8 Bits
Parity      → None
Stop Bits   → 1
Mode        → TX/RX
```

Örneğin:

```c
huart2.Init.BaudRate = 115200;
huart2.Init.WordLength = UART_WORDLENGTH_8B;
huart2.Init.StopBits = UART_STOPBITS_1;
huart2.Init.Parity = UART_PARITY_NONE;
huart2.Init.Mode = UART_MODE_TX_RX;
```

UART interrupt'ının NVIC üzerinden aktif olduğundan emin olun.

Örneğin USART2 kullanılıyorsa:

```text
USART2 global interrupt → Enabled
```

---

# 🔄 Circular Buffer

Driver'ın temelini Circular Buffer oluşturmaktadır.

Circular Buffer içerisinde iki temel indeks bulunur:

```c
uint16_t head;
uint16_t tail;
```

`head` verinin yazılacağı konumu, `tail` ise okunacağı konumu gösterir.

Buffer boyutu varsayılan olarak:

```c
#define circuler_buffer_size 512
```

olarak belirlenmiştir.

Buffer yapısı:

```c
typedef struct
{
    uint8_t buffer[circuler_buffer_size];

    uint16_t head;
    uint16_t tail;

} Circuler_Buffer_t;
```

---

# 📥 Veriyi Buffer'a Ekleme

Buffer'a veri eklemek için:

```c
Circuler_Buffer_Enqueue(&buffer, data);
```

fonksiyonu kullanılır.

Fonksiyon başarılı olursa `true`, buffer doluysa `false` döndürür.

Örneğin:

```c
uint8_t data = 'A';

if(Circuler_Buffer_Enqueue(&buffer, data))
{
    // Veri başarıyla eklendi.
}
```

---

# 📤 Buffer'dan Veri Okuma

Buffer içerisindeki veriyi okumak için:

```c
Circuler_Buffer_Dequeue(&buffer, &data);
```

kullanılır.

Örneğin:

```c
uint8_t data;

if(Circuler_Buffer_Dequeue(&buffer, &data))
{
    // Veri başarıyla okundu.
}
```

Buffer boşsa fonksiyon `false` döndürür.

---

# 🔎 Buffer Durumunu Kontrol Etme

Buffer'ın boş olup olmadığını kontrol etmek için:

```c
circuler_buffer_is_empty(&buffer);
```

Buffer'ın dolu olup olmadığını kontrol etmek için:

```c
circuler_buffer_is_fully(&buffer);
```

kullanılabilir.

Buffer içerisinde kaç adet veri olduğunu öğrenmek için:

```c
Circuler_Buffer_Count(&buffer);
```

fonksiyonu kullanılabilir.

---

# 🚀 UART Driver'ı Başlatma

UART driver için öncelikle `UART_Ex_t` yapısından bir değişken oluşturulmalıdır.

Ayrıca RX ve TX için iki ayrı Circular Buffer oluşturulmalıdır.

```c
UART_Ex_t uart2;

Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;
```

Daha sonra driver şu şekilde başlatılır:

```c
UARTx_Initialization(
    &uart2,
    &huart2,
    &UART_cb_In,
    &UART_cb_out
);
```

Burada:

```text
uart2       → UART driver yapısı
huart2      → STM32 HAL UART handle'ı
UART_cb_In  → RX buffer
UART_cb_out → TX buffer
```

olarak kullanılmaktadır.

Initialization fonksiyonu içerisinde RXNE ve TXE interrupt'ları aktif edilir.

---

# ✏️ Karakter Gönderme

UART üzerinden tek bir karakter göndermek için:

```c
UARTx_Write_Char(&uart2, 'A');
```

kullanılabilir.

Karakter önce TX Circular Buffer'a eklenir.

Daha sonra TXE interrupt'ı ile UART donanımına aktarılır.

---

# 📝 String Gönderme

String göndermek için:

```c
UARTx_Put_String(&uart2, "Merhaba Dünya!\r\n");
```

kullanılabilir.

Fonksiyon string içerisindeki karakterleri tek tek TX buffer'a ekler.

Örneğin:

```c
UARTx_Put_String(&uart2, "STM32 UART Driver\r\n");
```

---

# 🖨️ Printf Benzeri Kullanım

Driver'ın kullanışlı özelliklerinden biri `printf` benzeri formatlı veri gönderebilmesidir.

Bunun için:

```c
UARTx_Printf(&uart2, "ADC Value: %d\r\n", adcValue);
```

kullanılabilir.

Örneğin:

```c
int temperature = 25;

UARTx_Printf(
    &uart2,
    "Temperature: %d C\r\n",
    temperature
);
```

Birden fazla değişken de gönderilebilir:

```c
UARTx_Printf(
    &uart2,
    "X: %d | Y: %d | Z: %d\r\n",
    x,
    y,
    z
);
```

Bu fonksiyon arka planda `vsnprintf()` kullanarak verilen formatı oluşturur ve ardından `UARTx_Put_String()` fonksiyonu ile TX buffer'a aktarır.

---

# 📖 UART Üzerinden Satır Okuma

UART üzerinden gelen mesajları satır halinde okumak için:

```c
UARTx_ReadLine(
    &uart2,
    lineBuffer,
    sizeof(lineBuffer)
);
```

kullanılabilir.

Örneğin:

```c
char lineBuffer[128];

if(UARTx_ReadLine(
        &uart2,
        lineBuffer,
        sizeof(lineBuffer)))
{
    // Yeni bir mesaj geldi.
}
```

Driver `\r\n` karakterlerini satır sonu olarak kullanmaktadır.

Örneğin bilgisayardan:

```text
Hello STM32\r\n
```

gönderildiğinde:

```c
lineBuffer
```

içerisinde:

```text
Hello STM32
```

mesajı elde edilir.

Bu yapı özellikle UART üzerinden komut gönderilen uygulamalarda kullanılabilir.

Örneğin:

```text
LED_ON\r\n
LED_OFF\r\n
MOTOR_START\r\n
MOTOR_STOP\r\n
```

gibi komutlar alınabilir.

---

# ⚡ Interrupt Yapısı

UART interrupt handler içerisinde iki temel durum kontrol edilmektedir.

## RXNE

UART üzerinden yeni bir veri geldiğinde `RXNE` interrupt'ı oluşur.

Gelen karakter okunarak RX buffer'a eklenir:

```c
uint8_t ch;

ch = (uint8_t)(
    uart2.huart->Instance->DR & 0x00FF
);

Circuler_Buffer_Enqueue(
    uart2.cbIn,
    ch
);
```

## TXE

UART veri register'ı yeni veri göndermeye hazır olduğunda `TXE` interrupt'ı oluşur.

TX buffer boş değilse buffer içerisindeki bir sonraki karakter UART'a gönderilir:

```c
if(!circuler_buffer_is_empty(uart2.cbOut))
{
    uint8_t ch;

    if(Circuler_Buffer_Dequeue(
        uart2.cbOut,
        &ch))
    {
        uart2.huart->Instance->DR = ch;
    }
}
```

TX buffer boş olduğunda ise TXE interrupt'ı devre dışı bırakılır.

---

# 💻 Tam Kullanım Örneği

Aşağıdaki örnek USART2 kullanarak driver'ın temel kullanımını göstermektedir.

```c
#include "main.h"
#include "UART_ex.h"

UART_HandleTypeDef huart2;

UART_Ex_t uart2;

Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();

    UARTx_Initialization(
        &uart2,
        &huart2,
        &UART_cb_In,
        &UART_cb_out
    );

    while(1)
    {
        UARTx_Printf(
            &uart2,
            "STM32 UART Driver\r\n"
        );

        HAL_Delay(2000);
    }
}
```

Bu örnekte her 2 saniyede bir:

```text
STM32 UART Driver
```

mesajı UART üzerinden gönderilir.

---

# 📥 Gelen Mesajı Okuma Örneği

UART üzerinden gelen mesajları okumak için:

```c
char lineBuffer[128];

while(1)
{
    if(UARTx_ReadLine(
            &uart2,
            lineBuffer,
            sizeof(lineBuffer)))
    {
        UARTx_Printf(
            &uart2,
            "Gelen mesaj: %s\r\n",
            lineBuffer
        );
    }
}
```

Örneğin bilgisayardan:

```text
Merhaba STM32
```

gönderildiğinde STM32:

```text
Gelen mesaj: Merhaba STM32
```

şeklinde cevap verebilir.

---

# 📊 Buffer Yapısının Özeti

Bu driver'da UART haberleşmesi için iki Circular Buffer kullanılmaktadır:

```text
                     STM32
                       │
              ┌────────┴────────┐
              │                 │
             RX                TX
              │                 │
              ▼                 ▼
        ┌───────────┐     ┌───────────┐
        │ RX Buffer │     │ TX Buffer │
        └─────┬─────┘     └─────┬─────┘
              │                 │
              ▼                 ▼
          Uygulama            UART
```

RX tarafında:

```text
UART → RX Interrupt → RX Buffer → Uygulama
```

TX tarafında:

```text
Uygulama → TX Buffer → TX Interrupt → UART
```

şeklinde bir veri akışı vardır.

---

# ⚠️ Dikkat Edilmesi Gerekenler

### 1. Buffer boyutu

Circular Buffer boyutu:

```c
#define circuler_buffer_size 512
```

olarak belirlenmiştir.

Uygulamanızda daha büyük veya daha küçük mesajlar kullanılacaksa bu değer ihtiyaca göre değiştirilebilir.

### 2. UART Interrupt

UART RX ve TX işlemlerinin düzgün çalışması için ilgili UART interrupt'ının aktif olması gerekir.

Örneğin USART2 kullanılıyorsa:

```text
USART2 global interrupt → Enabled
```

olmalıdır.

### 3. RX ve TX buffer'ları

RX ve TX için ayrı Circular Buffer kullanılması gerekir.

```c
Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;
```

Bu iki buffer'ın aynı anda RX ve TX için kullanılmaması gerekir.

### 4. Satır sonu

`UARTx_ReadLine()` fonksiyonu mesaj sonunu:

```text
\r\n
```

karakter dizisi ile belirlemektedir.

Bu nedenle terminal programının satır sonu ayarının `CRLF` olması önerilir.

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

Bu proje STM32F4 üzerinde UART haberleşmesini interrupt ve Circular Buffer kullanarak gerçekleştirmeyi öğrenmek ve farklı projelerde tekrar kullanılabilecek bir UART driver geliştirmek amacıyla hazırlanmıştır.

Driver'ın temel amacı UART üzerinden veri gönderme ve alma işlemlerini uygulama kodundan mümkün olduğunca ayırarak daha düzenli ve tekrar kullanılabilir bir yapı oluşturmaktır.

Bu yapı özellikle komut tabanlı UART uygulamaları, debug mesajları, sensör verileri ve mikrodenetleyici ile bilgisayar arasındaki seri haberleşme uygulamalarında kullanılabilir.
