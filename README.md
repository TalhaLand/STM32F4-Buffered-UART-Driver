# STM32F4 Buffered UART Driver

STM32F4 mikrodenetleyicilerinde UART haberleşmesini **Interrupt** ve **Circular Buffer** kullanarak gerçekleştiren bir UART sürücüsüdür.

Driver; UART üzerinden veri alma/gönderme, string gönderme, `printf` benzeri formatlı veri gönderme ve gelen verileri satır halinde okuma işlemlerini destekler.

---

# 🚀 Hızlı Başlangıç

Driver'ı kendi STM32CubeIDE projenize dahil etmek için aşağıdaki adımları uygulayın.

### 1. Driver dosyalarını projenize ekleyin

Aşağıdaki dosyaları projenize dahil edin:

```text
UART_ex.c
UART_ex.h
Circuler_Buffer.c
Circuler_Buffer.h
```

Ardından:

```c
#include "UART_ex.h"
```

satırını driver'ı kullanacağınız `.c` dosyasına ekleyin.

### 2. UART'ı STM32CubeMX üzerinden yapılandırın

Kullanacağınız UART peripheral'ını aktif edin.

Örneğin:

```text
Baud Rate   → 115200
Word Length → 8 Bits
Parity      → None
Stop Bits   → 1
Mode        → TX/RX
```

### 3. UART Interrupt'ını aktif edin

NVIC içerisinden kullandığınız UART'ın global interrupt'ını aktif edin.

Örneğin USART2 kullanıyorsanız:

```text
USART2 global interrupt → Enabled
```

### 4. Circular Buffer oluşturun

RX ve TX için iki ayrı buffer oluşturun:

```c
UART_Ex_t uart2;

Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;
```

### 5. UART Driver'ı başlatın

STM32CubeMX tarafından oluşturulan UART handle'ını kullanarak driver'ı başlatın:

```c
UARTx_Initialization(
    &uart2,
    &huart2,
    &UART_cb_In,
    &UART_cb_out
);
```

### 6. Artık UART'ı kullanabilirsiniz

Karakter göndermek:

```c
UARTx_Write_Char(&uart2, 'A');
```

String göndermek:

```c
UARTx_Put_String(&uart2, "Merhaba STM32\r\n");
```

Formatlı veri göndermek:

```c
UARTx_Printf(
    &uart2,
    "ADC Value: %d\r\n",
    adcValue
);
```

UART üzerinden gelen satırı okumak:

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

Bu kadar. 🎯 Driver'ın temel kullanımına başlamak için yukarıdaki adımlar yeterlidir.

---

# 📌 Çalışma Mantığı

Driver'ın temelinde **Circular Buffer** ve **UART Interrupt** yapısı bulunmaktadır.

UART üzerinden veri geldiğinde:

```text
UART
 ↓
RX Interrupt
 ↓
RX Circular Buffer
 ↓
Uygulama
```

Veri gönderileceği zaman:

```text
Uygulama
 ↓
TX Circular Buffer
 ↓
TX Interrupt
 ↓
UART
```

Bu yapı sayesinde CPU'nun UART veri aktarımını sürekli olarak beklemesine gerek kalmaz.

---

# 🔄 Circular Buffer

Driver'da RX ve TX işlemleri için ayrı Circular Buffer kullanılmaktadır.

```text
RX → RX Buffer
TX → TX Buffer
```

Buffer içerisinde `head` ve `tail` indeksleri kullanılarak verilerin eklenmesi ve okunması sağlanır.

Varsayılan buffer boyutu:

```c
#define circuler_buffer_size 512
```

olarak belirlenmiştir.

---

# 📥 RX İşlemi

UART üzerinden bir karakter geldiğinde RX interrupt'ı tetiklenir.

Gelen karakter RX Circular Buffer'a eklenir.

Uygulama tarafında ise bu veriler buffer üzerinden okunur.

Bu sayede UART'tan gelen veriler, uygulamanın o anda işlem yapıp yapmadığından bağımsız olarak buffer içerisinde tutulabilir.

---

# 📤 TX İşlemi

Uygulama tarafından gönderilmek istenen veriler önce TX Circular Buffer'a eklenir.

TX interrupt'ı aktif olduğunda buffer içerisindeki veriler sırayla UART donanımına aktarılır.

TX buffer boşaldığında TX interrupt'ı devre dışı bırakılır.

---

# 📝 Satır Okuma

`UARTx_ReadLine()` fonksiyonu UART üzerinden gelen karakterleri biriktirerek satır halinde okunmasını sağlar.

Satır sonu olarak:

```text
\r\n
```

kullanılır.

Örneğin bilgisayardan:

```text
LED_ON\r\n
```

gönderildiğinde uygulama tarafında:

```c
lineBuffer
```

içerisinden:

```text
LED_ON
```

mesajı alınabilir.

Bu yapı özellikle UART üzerinden komut gönderilen uygulamalarda kullanışlıdır.

---

# 🖨️ Printf Desteği

`UARTx_Printf()` fonksiyonu sayesinde UART üzerinden formatlı veri gönderilebilir.

Örneğin:

```c
UARTx_Printf(
    &uart2,
    "Temperature: %d C\r\n",
    temperature
);
```

Bu özellik debug mesajları ve sensör verilerinin UART üzerinden gönderilmesi için kullanılabilir.

---

# 📂 Dosya Yapısı

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

Bu proje STM32F4 üzerinde UART haberleşmesini **Interrupt ve Circular Buffer** kullanarak gerçekleştirmek ve farklı projelerde tekrar kullanılabilecek bir UART driver geliştirmek amacıyla hazırlanmıştır.
