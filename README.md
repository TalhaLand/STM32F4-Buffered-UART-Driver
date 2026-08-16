# STM32F4 Buffered UART Driver

STM32F4 mikrodenetleyiciler için geliştirilmiş, **Interrupt-driven UART** haberleşme sürücüsüdür.

Bu driver, UART haberleşmesinde gelen ve giden verileri **Circular Buffer** yapıları üzerinden yönetir. UART RX ve TX işlemleri interrupt kullanılarak gerçekleştirildiği için uygulama kodunun UART haberleşmesini sürekli polling ederek takip etmesine gerek kalmaz.

Driver ayrıca:

* Interrupt-driven RX
* Interrupt-driven TX
* RX Circular Buffer
* TX Circular Buffer
* String transmission
* Formatted transmission (`printf` benzeri kullanım)
* CR/LF sonlandırılmış mesaj okuma

özelliklerini sağlar.

---

# 1. Quick Start

Bu bölüm yalnızca driver'ı kullanmak isteyen bir kişinin hızlıca sistemi çalıştırabilmesi için hazırlanmıştır.

Driver'ın nasıl çalıştığını öğrenmek istemiyorsanız, bu bölümdeki adımları takip etmeniz yeterlidir.

---

## 1.1. Driver Dosyalarını Projeye Eklemek

Driver için aşağıdaki dosyalara ihtiyaç vardır:

```text
UART_ex.c
UART_ex.h
Circuler_Buffer.c
Circuler_Buffer.h
```

Bu dosyaları STM32CubeIDE projenize ekleyin.

Örneğin:

```text
Core/
├── Inc/
│   ├── UART_ex.h
│   └── Circuler_Buffer.h
│
└── Src/
    ├── UART_ex.c
    └── Circuler_Buffer.c
```

Dosyaları projeye ekledikten sonra:

```c
#include "UART_ex.h"
```

satırı ile driver `main.c` içerisinde kullanılabilir.

---

# 2. STM32CubeMX / CubeIDE UART Ayarları

Öncelikle kullanmak istediğiniz UART çevre birimini CubeMX üzerinden yapılandırın.

Örneğin USART2:

```text
USART2
 ├── TX
 └── RX
```

UART modu:

```text
Mode: TX and RX
```

Örnek UART ayarları:

```text
Baud Rate    : 115200
Word Length  : 8 Bits
Parity       : None
Stop Bits    : 1
Mode         : TX/RX
Hardware Flow Control : None
Oversampling : 16
```

Bu ayarlar sonucunda CubeMX aşağıdakine benzer bir handle oluşturacaktır:

```c
UART_HandleTypeDef huart2;
```

ve UART başlatma fonksiyonu:

```c
static void MX_USART2_UART_Init(void);
```

oluşturulacaktır.

> **Not:** Driver HAL UART handle'ı üzerinden çalışmaktadır. Bu nedenle CubeMX tarafından oluşturulan `UART_HandleTypeDef` kullanılmaktadır.

---

# 3. main.c İçerisinde Driver'ı Aktif Etmek

Öncelikle `UART_ex.h` dosyasını include edin:

```c
#include "UART_ex.h"
```

Daha sonra UART driver instance'ı ve RX/TX buffer'ları oluşturun:

```c
UART_Ex_t uart2;

Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;
```

Burada:

* `uart2` → UART driver instance'ıdır.
* `UART_cb_In` → UART üzerinden gelen verileri tutar.
* `UART_cb_out` → UART üzerinden gönderilecek verileri tutar.

---

## 3.1. Driver Initialization

CubeMX tarafından oluşturulan UART initialization fonksiyonlarından sonra driver başlatılmalıdır.

Örneğin:

```c
MX_GPIO_Init();
MX_USART2_UART_Init();

UARTx_Initialization(
    &uart2,
    &huart2,
    &UART_cb_In,
    &UART_cb_out
);
```

Burada:

```c
&uart2
```

driver instance'ını,

```c
&huart2
```

CubeMX tarafından oluşturulan HAL UART handle'ını,

```c
&UART_cb_In
```

RX buffer'ını,

```c
&UART_cb_out
```

TX buffer'ını belirtir.

Initialization sırasında driver:

1. UART handle'ını driver'a bağlar.
2. RX Circular Buffer'ı initialize eder.
3. TX Circular Buffer'ı initialize eder.
4. RXNE interrupt'ını aktif eder.
5. TXE interrupt'ını aktif eder.

---

# 4. stm32f4xx_it.c İçerisinde Yapılması Gerekenler

Driver'ın interrupt tabanlı çalışması için UART interrupt handler'ının doğru yapılandırılması gerekir.

Öncelikle:

```c
#include "UART_ex.h"
```

eklenmelidir.

Daha sonra `main.c` içerisinde oluşturduğumuz UART driver instance'ına erişebilmek için:

```c
extern UART_Ex_t uart2;
```

tanımlanmalıdır.

CubeMX tarafından oluşturulan:

```c
extern UART_HandleTypeDef huart2;
```

de kullanılmaya devam edilir.

---

## 4.1. USART2_IRQHandler()

USART2 kullanılıyorsa:

```c
void USART2_IRQHandler(void)
{
    ...
}
```

fonksiyonu içerisinde RX ve TX işlemleri gerçekleştirilir.

RX tarafında:

```c
if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE))
{
    uint8_t ch =
        (uint8_t)(uart2.huart->Instance->DR & (uint8_t)0x00FF);

    Circuler_Buffer_Enqueue(uart2.cbIn, ch);
}
```

TX tarafında ise:

```c
if(__HAL_UART_GET_FLAG(uart2.huart, UART_FLAG_TXE))
{
    if(!(circuler_buffer_is_empty(uart2.cbOut)))
    {
        uint8_t ch;

        if(Circuler_Buffer_Dequeue(uart2.cbOut, &ch))
        {
            uart2.huart->Instance->DR = ch;
        }
    }
    else
    {
        __HAL_UART_DISABLE_IT(uart2.huart, UART_IT_TXE);
    }
}
```

kullanılır.

Sonrasında HAL interrupt handler'ı da çağrılır:

```c
HAL_UART_IRQHandler(&huart2);
```

> **Önemli:** Driver'ın interrupt tabanlı çalışması için kullanılan UART'ın ilgili global interrupt'ının NVIC tarafında etkin olması gerekir.

---

# 5. İlk UART Mesajınızı Göndermek

Driver initialize edildikten sonra UART üzerinden mesaj göndermek oldukça basittir.

## Tek karakter

```c
UARTx_Write_Char(&uart2, 'A');
```

---

## String göndermek

```c
UARTx_Put_String(&uart2, "Hello World\r\n");
```

---

## printf benzeri kullanım

Driver'ın en kullanışlı fonksiyonlarından biri:

```c
UARTx_Printf()
```

fonksiyonudur.

Örneğin:

```c
UARTx_Printf(&uart2, "Hello World\r\n");
```

Değişken göndermek için:

```c
int temperature = 25;

UARTx_Printf(
    &uart2,
    "Temperature: %d C\r\n",
    temperature
);
```

Float:

```c
float voltage = 3.3f;

UARTx_Printf(
    &uart2,
    "Voltage: %.2f V\r\n",
    voltage
);
```

---

# 6. UART Üzerinden Mesaj Almak

Driver gelen karakterleri RX Circular Buffer içerisinde saklar.

Uygulama tarafında mesaj okumak için:

```c
UARTx_ReadLine()
```

fonksiyonu kullanılır.

Örneğin:

```c
char rxBuffer[128];

if(UARTx_ReadLine(&uart2, rxBuffer, sizeof(rxBuffer)))
{
    UARTx_Printf(
        &uart2,
        "Received: %s\r\n",
        rxBuffer
    );
}
```

Bilgisayardan:

```text
Hello STM32\r\n
```

gönderildiğinde:

```text
Received: Hello STM32
```

şeklinde işlenebilir.

Driver burada mesajın sonunu:

```text
\r\n
```

kombinasyonu ile belirler.

---

# 7. Tam Kullanım Örneği

`main.c` içerisinde temel kullanım şu şekilde olabilir:

```c
#include "main.h"
#include "UART_ex.h"

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

    UARTx_Printf(
        &uart2,
        "UART Driver Started!\r\n"
    );

    char rxBuffer[128];

    while(1)
    {
        if(UARTx_ReadLine(
            &uart2,
            rxBuffer,
            sizeof(rxBuffer)
        ))
        {
            UARTx_Printf(
                &uart2,
                "Received: %s\r\n",
                rxBuffer
            );
        }
    }
}
```

Bu yapı ile:

```text
PC
 │
 │ UART
 ▼
STM32
 │
 ├── RX Interrupt
 │       ↓
 │   RX Circular Buffer
 │       ↓
 │   UARTx_ReadLine()
 │       ↓
 │   Application
 │
 └── TX Circular Buffer
         ↓
     TX Interrupt
         ↓
        UART
         ↓
         PC
```

şeklinde bir haberleşme sistemi oluşur.

---

# 8. Driver Mimarisi

Driver'ın temel mimarisi:

```text
                  APPLICATION
                       │
                       │
             UARTx_Printf()
             UARTx_Put_String()
             UARTx_ReadLine()
                       │
                       ▼
                ┌─────────────┐
                │ UART Driver │
                └─────────────┘
                  │         │
                  ▼         ▼
             RX Buffer   TX Buffer
                  │         │
                  ▼         ▼
             RX Interrupt  TX Interrupt
                  │         │
                  └────┬────┘
                       ▼
                    UART
```

Buradaki temel fikir UART'ın doğrudan application tarafından yönetilmemesidir.

Application:

```c
UARTx_Printf(...);
```

der.

Driver:

```text
veriyi TX buffer'a koyar
        ↓
TX interrupt çalışır
        ↓
buffer'dan karakter alınır
        ↓
UART DR register'ına yazılır
        ↓
UART üzerinden gönderilir
```

RX tarafında ise süreç tersidir:

```text
UART üzerinden karakter gelir
        ↓
RXNE flag oluşur
        ↓
USART interrupt çalışır
        ↓
DR register okunur
        ↓
RX buffer'a karakter eklenir
        ↓
Application UARTx_ReadLine() çağırır
```

---

# 9. Neden Circular Buffer Kullanıyoruz?

UART'tan gelen veri her zaman application'ın hazır olduğu anda gelmeyebilir.

Örneğin:

```text
UART → A
UART → B
UART → C
UART → D
```

karakterleri geldiğinde CPU başka bir işlemle meşgul olabilir.

Eğer gelen karakteri doğrudan application'a vermeye çalışırsak veri kaybetme ihtimali oluşur.

Bu nedenle gelen veriyi önce buffer'a koyuyoruz:

```text
UART
 ↓
Interrupt
 ↓
RX Circular Buffer
 ↓
Application
```

Application daha sonra uygun olduğunda buffer'daki veriyi okuyabilir.

Bu yaklaşım UART ile application arasına bir **asenkron veri kuyruğu** koymuş olur.

---

# 10. Circular Buffer Mantığı

Circular Buffer içerisinde iki temel index vardır:

```c
uint16_t head;
uint16_t tail;
```

Bunların görevleri:

```text
head → yeni verinin yazılacağı yer

tail → okunacak verinin bulunduğu yer
```

Örneğin buffer:

```text
[0][1][2][3][4][5][6][7]
 ↑
head
 ↑
tail
```

Başlangıçta:

```c
head = 0;
tail = 0;
```

olur.

Bu durumda buffer boştur.

---

# 11. Buffer Empty

Kod:

```c
bool circuler_buffer_is_empty(
    Circuler_Buffer_t *circulerBuffer
)
{
    return
        (circulerBuffer->head ==
         circulerBuffer->tail)
        ? true
        : false;
}
```

Temel mantık:

```text
head == tail
```

ise buffer boştur.

Örneğin:

```text
head = 5
tail = 5
```

→ buffer empty.

---

# 12. Buffer'a Veri Eklemek - Enqueue

Fonksiyon:

```c
bool Circuler_Buffer_Enqueue(
    Circuler_Buffer_t *circulerBuffer,
    uint8_t data
)
```

görevi buffer'a bir byte eklemektir.

Önce buffer'ın dolu olup olmadığı kontrol edilir:

```c
if(circuler_buffer_is_fully(circulerBuffer))
{
    return false;
}
```

Dolu değilse:

```c
circulerBuffer->buffer[
    circulerBuffer->head
] = data;
```

ile veri `head` konumuna yazılır.

Sonrasında:

```c
circulerBuffer->head =
    (circulerBuffer->head + 1)
    % circuler_buffer_size;
```

yapılır.

Buradaki `%` işlemi circular davranışın temelidir.

Örneğin buffer'ın boyutu 512 ise:

```text
510
511
0
1
2
...
```

şeklinde tekrar başa dönülür.

---

# 13. Buffer'dan Veri Almak - Dequeue

Fonksiyon:

```c
Circuler_Buffer_Dequeue()
```

buffer'ın `tail` noktasındaki veriyi çıkarır.

Öncelikle buffer boş mu kontrol edilir:

```c
if(circuler_buffer_is_empty(circulerBuffer))
{
    return false;
}
```

Veri varsa:

```c
*data =
    circulerBuffer->buffer[
        circulerBuffer->tail
    ];
```

ile veri alınır.

Sonrasında:

```c
circulerBuffer->tail =
    (circulerBuffer->tail + 1)
    % circuler_buffer_size;
```

ile `tail` ilerletilir.

---

# 14. Buffer Count

Buffer'da kaç byte veri bulunduğunu öğrenmek için:

```c
Circuler_Buffer_Count()
```

kullanılır.

Eğer:

```text
head >= tail
```

ise:

```c
head - tail
```

kadar veri vardır.

Eğer `head`, `tail` değerinin gerisine dönmüşse:

```c
circuler_buffer_size - tail + head
```

hesaplanır.

Bu, circular buffer'ın fiziksel olarak sona ulaşıp tekrar başa dönmesini hesaba katar.

---

# 15. Buffer Boyutu

Buffer boyutu:

```c
#define circuler_buffer_size 512
```

olarak belirlenmiştir.

Bu değeri değiştirmek mümkündür.

Örneğin:

```c
#define circuler_buffer_size 128
```

veya:

```c
#define circuler_buffer_size 1024
```

yapılabilir.

Ancak buffer büyüdükçe RAM kullanımı da artar.

Bir `Circuler_Buffer_t` içerisinde:

```c
uint8_t buffer[circuler_buffer_size];
```

bulunduğu için her buffer yaklaşık olarak buffer boyutu kadar RAM kullanır.

RX ve TX için iki ayrı buffer olduğundan:

```text
512 byte RX
+
512 byte TX
=
1024 byte
```

sadece veri buffer'ları için kullanılır.

---

# 16. UART Driver Structure

UART driver'ın temel yapısı `UART_Ex_t` yapısıdır.

Bu yapı UART ile ilgili kaynakları bir araya getirir.

Mantıksal olarak:

```text
UART_Ex_t
│
├── UART Handle
│
├── RX Circular Buffer
│
└── TX Circular Buffer
```

şeklinde düşünülebilir.

Bu sayede driver fonksiyonlarına her seferinde ayrı ayrı UART handle ve buffer göndermek yerine:

```c
UARTx_Printf(&uart2, ...);
```

şeklinde tek bir driver instance'ı üzerinden işlem yapılabilir.

---

# 17. UARTx_Initialization()

Fonksiyon:

```c
void UARTx_Initialization(
    UART_Ex_t *uart,
    UART_HandleTypeDef *huart,
    Circuler_Buffer_t *cbIn,
    Circuler_Buffer_t *cbOut
)
```

driver'ın başlangıç fonksiyonudur.

İlk olarak UART handle bağlanır:

```c
uart->huart = huart;
```

RX buffer:

```c
uart->cbIn = cbIn;
```

TX buffer:

```c
uart->cbOut = cbOut;
```

ile bağlanır.

Daha sonra:

```c
Circuler_Buffer_Init(uart->cbIn);
Circuler_Buffer_Init(uart->cbOut);
```

ile iki buffer initialize edilir.

Son olarak:

```c
__HAL_UART_ENABLE_IT(
    uart->huart,
    UART_IT_RXNE
);
```

ile RX interrupt,

```c
__HAL_UART_ENABLE_IT(
    uart->huart,
    UART_IT_TXE
);
```

ile TX interrupt aktif edilir.

---

# 18. RXNE Interrupt

RXNE:

```text
Receive Data Register Not Empty
```

anlamına gelir.

UART yeni bir byte aldığında RXNE flag'i set edilir.

Bu durumda:

```c
USART2_IRQHandler()
```

çalışır.

Kod:

```c
if(__HAL_UART_GET_FLAG(
    &huart2,
    UART_FLAG_RXNE))
{
    ...
}
```

ile RXNE kontrol edilir.

---

# 19. UART DR Register

RX sırasında gelen byte:

```c
uart2.huart->Instance->DR
```

register'ından okunur.

Örneğin:

```c
uint8_t ch =
    (uint8_t)(
        uart2.huart->Instance->DR &
        (uint8_t)0x00FF
    );
```

Buradaki amaç UART'ın data register'ındaki alınan byte'ı elde etmektir.

Daha sonra:

```c
Circuler_Buffer_Enqueue(
    uart2.cbIn,
    ch
);
```

ile RX buffer'a eklenir.

Dolayısıyla:

```text
UART DR
  ↓
uint8_t ch
  ↓
RX Circular Buffer
```

oluşur.

---

# 20. TXE Interrupt

TXE:

```text
Transmit Data Register Empty
```

anlamına gelir.

UART yeni bir byte göndermeye hazır olduğunda TXE flag'i set edilir.

Driver TX buffer'da veri olup olmadığını kontrol eder:

```c
if(!(circuler_buffer_is_empty(
    uart2.cbOut)))
```

Buffer'da veri varsa:

```c
Circuler_Buffer_Dequeue(
    uart2.cbOut,
    &ch
);
```

ile bir byte alınır.

Sonra:

```c
uart2.huart->Instance->DR = ch;
```

ile UART data register'ına yazılır.

---

# 21. TXE Interrupt Neden Kapatılıyor?

TX buffer tamamen boşaldığında TX interrupt'ın sürekli çalışmasına gerek yoktur.

Bu nedenle:

```c
__HAL_UART_DISABLE_IT(
    uart2.huart,
    UART_IT_TXE
);
```

ile TXE interrupt kapatılır.

Yeni veri gönderilmek istendiğinde:

```c
UARTx_Write_Char()
```

TX buffer'a veri ekler ve gerekirse TXE interrupt tekrar aktif edilir.

Bu sayede CPU yalnızca gerçekten gönderilecek veri olduğunda TX interrupt ile ilgilenir.

---

# 22. UARTx_Write_Char()

Fonksiyon:

```c
void UARTx_Write_Char(
    UART_Ex_t *uart,
    char ch
)
```

tek bir karakter göndermek için kullanılır.

İlk olarak karakter TX buffer'a eklenir:

```c
Circuler_Buffer_Enqueue(
    uart->cbOut,
    ch
);
```

Eğer TXE interrupt aktif değilse driver buffer'daki ilk karakteri alıp doğrudan UART data register'ına yazar.

Sonrasında:

```c
__HAL_UART_ENABLE_IT(
    uart->huart,
    UART_IT_TXE
);
```

ile TXE interrupt aktif edilir.

Bundan sonraki karakterler interrupt tarafından gönderilir.

---

# 23. UARTx_Put_String()

Fonksiyon:

```c
void UARTx_Put_String(
    UART_Ex_t *uart,
    char *str
)
```

bir string'i karakter karakter gönderir.

Temel mantığı:

```c
while(*str)
{
    UARTx_Write_Char(uart, *str);

    str++;
}
```

şeklindedir.

Örneğin:

```c
UARTx_Put_String(
    &uart2,
    "Hello\r\n"
);
```

çağrısı:

```text
H
e
l
l
o
\r
\n
```

karakterlerinin sırayla TX buffer'a eklenmesini sağlar.

---

# 24. UARTx_Printf()

`UARTx_Printf()` fonksiyonu UART'ı `printf()` benzeri kullanabilmek için oluşturulmuştur.

Örneğin:

```c
UARTx_Printf(
    &uart2,
    "ADC Value: %d\r\n",
    adcValue
);
```

şeklinde kullanılabilir.

Fonksiyon içerisinde:

```c
va_list
va_start
vsnprintf
va_end
```

kullanılarak formatlı string oluşturulur.

Önce:

```c
char tx_Buffer[256];
```

oluşturulur.

Daha sonra:

```c
vsnprintf(
    tx_Buffer,
    sizeof(tx_Buffer),
    format,
    args
);
```

ile formatlanmış mesaj buffer'a yazılır.

Son olarak:

```c
UARTx_Put_String(
    uart,
    tx_Buffer
);
```

ile UART'a gönderilir.

---

# 25. UARTx_Printf() İçin Önemli Not

Fonksiyon içerisinde:

```c
char tx_Buffer[256];
```

bulunduğu için tek bir `UARTx_Printf()` çağrısında oluşturulan formatlı mesajın uzunluğu pratik olarak bu buffer ile sınırlıdır.

Daha uzun mesajlar için buffer boyutu veya fonksiyon tasarımı değiştirilmelidir.

Ayrıca `printf` formatlama işlemleri özellikle `%f` gibi floating-point formatlarında mikrodenetleyici üzerinde ekstra kod boyutu ve işlem yükü oluşturabilir.

---

# 26. UARTx_ReadLine()

`UARTx_ReadLine()` driver'ın RX tarafındaki application seviyesindeki fonksiyonudur.

Amaç:

```text
RX Circular Buffer
        ↓
karakterleri oku
        ↓
\r\n ara
        ↓
mesaj tamamlandı
        ↓
lineBuffer
```

şeklinde çalışmaktır.

Fonksiyon:

```c
bool UARTx_ReadLine(
    UART_Ex_t *uart,
    char *lineBuffer,
    uint16_t maxLen
)
```

şeklindedir.

---

# 27. CR ve LF Nedir?

UART terminal programlarında bir satır sonlandırılırken genellikle:

```text
\r\n
```

gönderilir.

Burada:

```text
\r = Carriage Return
\n = Line Feed
```

anlamına gelir.

Driver mesaj sonunu:

```text
CR + LF
```

kombinasyonu olarak kabul eder.

Örneğin:

```text
HELLO\r\n
```

geldiğinde:

```text
HELLO
```

mesaj olarak kabul edilir.

---

# 28. UARTx_ReadLine() İçerisindeki State Değişkenleri

Fonksiyon içerisinde:

```c
static uint16_t index = 0;
static bool messageReady = false;
static bool lastCr = false;
```

değişkenleri kullanılır.

### index

Şu an mesajın kaçıncı karakterinin yazıldığını takip eder.

```text
H → index = 1
e → index = 2
l → index = 3
...
```

### messageReady

Tam bir mesajın bulunduğunu belirtir.

```text
messageReady = true
```

olduğunda fonksiyon bir sonraki çağrısında mesajı application'a teslim eder.

### lastCr

Bir önceki karakterin:

```text
\r
```

olup olmadığını takip eder.

Bunun amacı:

```text
\r\n
```

kombinasyonunu algılamaktır.

---

# 29. UARTx_ReadLine() Mesaj Algılama Mantığı

Örneğin gelen veri:

```text
H
e
l
l
o
\r
\n
```

olsun.

Karakterler RX buffer'dan tek tek çıkarılır.

Normal karakterlerde:

```c
lineBuffer[index++] = ch;
```

yapılır.

`\r` geldiğinde:

```c
lastCr = true;
```

olur.

Sonraki karakter:

```c
\n
```

ise:

```c
if(lastCr && ch == '\n')
{
    messageReady = true;
    lastCr = false;
    break;
}
```

çalışır.

Böylece mesajın tamamlandığı anlaşılır.

---

# 30. Neden `static` Kullanıldı?

`UARTx_ReadLine()` bir döngü içerisinde sürekli çağrılabilir.

Mesaj tek seferde gelmeyebilir.

Örneğin:

```text
Loop 1:
HEL

Loop 2:
LO\r

Loop 3:
\n
```

gibi parça parça gelebilir.

Eğer `index`, `lastCr` ve `messageReady` normal local değişken olsaydı fonksiyondan çıkıldığında değerleri kaybolurdu.

`static` kullanılması sayesinde değerler fonksiyon çağrıları arasında korunur.

Bu nedenle:

```c
static uint16_t index;
static bool messageReady;
static bool lastCr;
```

kullanılmıştır.

---

# 31. maxLen Neden Kullanılıyor?

Fonksiyon çağrılırken:

```c
char rxBuffer[128];

UARTx_ReadLine(
    &uart2,
    rxBuffer,
    sizeof(rxBuffer)
);
```

şeklinde buffer boyutu belirtilir.

Fonksiyon:

```c
if(index < (maxLen - 1))
```

kontrolü ile array sınırlarının dışına çıkmamaya çalışır.

Sonlandırma karakteri:

```c
'\0'
```

için de bir byte ayrılır.

Bu nedenle:

```text
maxLen
```

değerinin tamamı mesaj karakterleri için kullanılmaz.

Son byte string terminator için ayrılır.

---

# 32. Null Terminator

C dilinde string'ler:

```c
'\0'
```

karakteri ile sonlandırılır.

Bu nedenle:

```c
lineBuffer[index] = '\0';
```

yapılır.

Örneğin:

```text
H e l l o \0
```

şeklinde bellekte tutulur.

Böylece:

```c
printf("%s", lineBuffer);
```

gibi string fonksiyonları kullanılabilir.

---

# 33. Driver'ın Veri Akışı

## RX

```text
External Device
      │
      ▼
   UART RX
      │
      ▼
    RXNE
      │
      ▼
USART2_IRQHandler()
      │
      ▼
    DR Read
      │
      ▼
RX Circular Buffer
      │
      ▼
UARTx_ReadLine()
      │
      ▼
 Application
```

---

## TX

```text
Application
     │
     ▼
UARTx_Printf()
     │
     ▼
UARTx_Put_String()
     │
     ▼
UARTx_Write_Char()
     │
     ▼
TX Circular Buffer
     │
     ▼
TXE Interrupt
     │
     ▼
USART2_IRQHandler()
     │
     ▼
DR Register
     │
     ▼
UART TX
```

---

# 34. Neden Polling Yerine Interrupt?

Polling yaklaşımında application sürekli UART'ın durumunu kontrol eder.

Örneğin:

```c
while(1)
{
    if(UART_DataAvailable())
    {
        ...
    }
}
```

Bu yaklaşım basit olsa da CPU'nun UART'ı sürekli kontrol etmesini gerektirebilir.

Interrupt yaklaşımında ise:

```text
UART'tan veri geldi
       ↓
Hardware interrupt oluşturdu
       ↓
CPU interrupt handler'a gitti
       ↓
Veri buffer'a koyuldu
       ↓
Application normal çalışmasına devam etti
```

şeklinde çalışılır.

Bu nedenle interrupt tabanlı yapı embedded sistemlerde daha uygun bir mimari sağlar.

---

# 35. Neden RX ve TX İçin İki Ayrı Buffer Var?

RX ve TX birbirinden bağımsız iki veri akışıdır.

```text
RX:
UART → STM32
```

```text
TX:
STM32 → UART
```

Bu nedenle iki farklı buffer kullanılır:

```c
Circuler_Buffer_t UART_cb_In;
Circuler_Buffer_t UART_cb_out;
```

RX buffer:

```text
UART → RX interrupt → cbIn
```

TX buffer:

```text
Application → cbOut → TX interrupt → UART
```

şeklinde çalışır.

---

# 36. Driver Dosyalarının Görevleri

## Circuler_Buffer.h

Circular Buffer veri yapısını ve fonksiyon prototiplerini içerir.

```text
Circuler_Buffer_t
Circuler_Buffer_Init()
Circuler_Buffer_Enqueue()
Circuler_Buffer_Dequeue()
Circuler_Buffer_Count()
```

---

## Circuler_Buffer.c

Circular Buffer algoritmasının implementation kısmıdır.

Burada:

* Empty kontrolü
* Full kontrolü
* Enqueue
* Dequeue
* Count
* Initialization

gerçekleştirilir.

---

## UART_ex.h

UART driver'ın dışarıya sunduğu API'yi içerir.

Application'ın temel olarak bilmesi gereken dosya budur.

---

## UART_ex.c

UART driver'ın implementation kısmıdır.

Burada:

* Initialization
* Character transmission
* String transmission
* Formatted transmission
* Line reception

gerçekleştirilir.

---

## stm32f4xx_it.c

UART hardware interrupt işlemlerinin gerçekleştirildiği yerdir.

Burada:

```text
RXNE
TXE
```

kontrolleri yapılır.

---

# 37. API Özeti

| Fonksiyon                    | Görevi                                      |
| ---------------------------- | ------------------------------------------- |
| `UARTx_Initialization()`     | UART driver'ı başlatır                      |
| `UARTx_Write_Char()`         | Tek karakter gönderir                       |
| `UARTx_Put_String()`         | String gönderir                             |
| `UARTx_Printf()`             | Formatlı veri gönderir                      |
| `UARTx_ReadLine()`           | CR/LF sonlandırılmış mesaj okur             |
| `Circuler_Buffer_Init()`     | Buffer'ı initialize eder                    |
| `Circuler_Buffer_Enqueue()`  | Buffer'a byte ekler                         |
| `Circuler_Buffer_Dequeue()`  | Buffer'dan byte çıkarır                     |
| `Circuler_Buffer_Count()`    | Buffer'daki veri miktarını döndürür         |
| `circuler_buffer_is_empty()` | Buffer'ın boş olup olmadığını kontrol eder  |
| `circuler_buffer_is_fully()` | Buffer'ın dolu olup olmadığını kontrol eder |

---

# 38. Driver'ı Baştan Yazmak İstersem

Bu projeyi tekrar sıfırdan yazmak gerektiğinde aşağıdaki sırayı takip etmek mantıklıdır.

## Step 1 - Circular Buffer

Önce:

```c
typedef struct
{
    uint8_t buffer[SIZE];

    uint16_t head;
    uint16_t tail;

} Circuler_Buffer_t;
```

oluştur.

Ardından:

```text
Init
 ↓
Empty
 ↓
Full
 ↓
Enqueue
 ↓
Dequeue
 ↓
Count
```

fonksiyonlarını yaz.

---

## Step 2 - UART Driver Structure

UART ile buffer'ları aynı yapı içerisinde tut:

```text
UART_Ex_t
 ├── huart
 ├── cbIn
 └── cbOut
```

---

## Step 3 - Initialization

UART handle ve buffer'ları driver'a bağla.

Ardından:

```text
RXNE interrupt
TXE interrupt
```

aktif et.

---

## Step 4 - RX Interrupt

UART'tan byte geldiğinde:

```text
RXNE
 ↓
DR oku
 ↓
RX Buffer'a Enqueue
```

yap.

---

## Step 5 - TX Interrupt

TXE oluştuğunda:

```text
TX Buffer boş mu?
 │
 ├── Hayır → Dequeue → DR
 │
 └── Evet → TXE interrupt disable
```

yap.

---

## Step 6 - Application API

Sonrasında application'ın doğrudan register ile uğraşmaması için:

```c
UARTx_Write_Char()
UARTx_Put_String()
UARTx_Printf()
UARTx_ReadLine()
```

gibi üst seviye fonksiyonlar oluştur.

---

# 39. Neden Driver Katmanı Oluşturduk?

Driver olmadan application doğrudan:

```c
huart2.Instance->DR
```

gibi register'lara erişmek zorunda kalabilir.

Bu durumda application kodu UART hardware'ına fazla bağımlı hale gelir.

Driver kullanıldığında:

```c
UARTx_Printf(
    &uart2,
    "ADC: %d\r\n",
    adcValue
);
```

yeterlidir.

Application:

```text
UART register
interrupt
buffer
TXE
RXNE
```

gibi detaylarla doğrudan ilgilenmez.

Bu ayrım:

```text
Application
     ↓
Driver
     ↓
Hardware
```

şeklinde bir abstraction oluşturur.

---

# 40. Tasarım Kararları

Bu driver'ın temel tasarım kararları şunlardır:

### Interrupt-driven

UART işlemlerini polling yerine interrupt üzerinden gerçekleştirmek için.

### Circular Buffer

UART ile application arasındaki veri akışını ayırmak için.

### RX + TX ayrı buffer

Gelen ve giden verileri birbirinden bağımsız yönetmek için.

### Driver abstraction

Application kodunu UART register detaylarından ayırmak için.

### `printf` benzeri API

Debug ve formatted message gönderimini kolaylaştırmak için.

### Line-based RX

UART terminal veya komut tabanlı sistemlerde mesajları:

```text
COMMAND\r\n
```

formatında kolayca işlemek için.

---

# 41. Örnek Komut Sistemi

Bu driver kullanılarak basit bir UART command interface oluşturulabilir.

Örneğin PC'den:

```text
LED_ON\r\n
```

gönderilebilir.

STM32:

```c
char rxBuffer[64];

if(UARTx_ReadLine(
    &uart2,
    rxBuffer,
    sizeof(rxBuffer)
))
{
    if(strcmp(rxBuffer, "LED_ON") == 0)
    {
        HAL_GPIO_WritePin(
            GPIOA,
            GPIO_PIN_5,
            GPIO_PIN_SET
        );

        UARTx_Printf(
            &uart2,
            "LED ON\r\n"
        );
    }
}
```

şeklinde komutu işleyebilir.

Bu yapı ileride:

```text
STM32 CLI
Debug Console
Configuration Menu
Sensor Interface
Motor Control Commands
Bootloader Interface
```

gibi sistemlerin temelini oluşturabilir.

---

# 42. Önemli Teknik Notlar

## Buffer Overflow

Circular Buffer doluysa:

```c
Circuler_Buffer_Enqueue()
```

`false` döndürür.

Bu durumda gelen veya gönderilmek istenen byte'ın işlenmesi application tarafından ayrıca ele alınmalıdır.

---

## Buffer Size

Daha büyük buffer daha fazla RAM kullanır.

Daha küçük buffer ise yoğun UART trafiğinde overflow riskini artırabilir.

Buffer boyutu uygulamanın veri hızına göre seçilmelidir.

---

## `UARTx_ReadLine()` Durumu

Mevcut implementation CR/LF tabanlı mesajları hedeflemektedir:

```text
MESSAGE\r\n
```

Dolayısıyla farklı protokoller için:

```text
\0
;
,
packet length
STX/ETX
CRC
```

gibi başka mesaj sonlandırma veya framing yöntemleri eklenebilir.

---

# 43. Mevcut Driver'ın Geliştirilebilecek Kısımları

Bu proje temel bir interrupt-driven UART driver olarak tasarlanmıştır.

İleride aşağıdaki özellikler eklenebilir:

* DMA RX
* DMA TX
* RX idle-line detection
* Timeout mekanizması
* Ring buffer overflow counter
* UART error handling
* Overrun error handling
* Framing error handling
* Parity error handling
* Multiple UART instance support
* Thread-safe kullanım
* RTOS desteği
* Command parser
* Binary packet protocol
* CRC kontrolü
* Callback sistemi

Özellikle DMA ile birlikte kullanıldığında yüksek baud rate ve yoğun veri akışlarında CPU yükü azaltılabilir.

---

# 44. Proje Yapısı

Önerilen proje yapısı:

```text
STM32F4-Buffered-UART-Driver/
│
├── Core/
│   ├── Inc/
│   │   ├── UART_ex.h
│   │   └── Circuler_Buffer.h
│   │
│   └── Src/
│       ├── UART_ex.c
│       └── Circuler_Buffer.c
│
├── README.md
│
└── LICENSE
```

---

# 45. Özet

Bu driver'ın temel çalışma mantığı:

```text
                    STM32
                     │
        ┌────────────┴────────────┐
        │                         │
       RX                         TX
        │                         │
        ▼                         ▼
      RXNE                       TXE
        │                         │
        ▼                         ▼
    DR Register              TX Buffer
        │                         │
        ▼                         │
   RX Circular Buffer             │
        │                         │
        ▼                         │
 UARTx_ReadLine()                │
        │                         │
        ▼                         ▼
      Application ←────── UARTx_Printf()
```

Daha basit şekilde:

```text
RX:

UART → Interrupt → RX Buffer → Application


TX:

Application → TX Buffer → Interrupt → UART
```

Bu mimarinin temel amacı UART hardware işlemlerini application kodundan ayırmak ve UART haberleşmesini buffer destekli, interrupt-driven bir yapıya dönüştürmektir.

---

# 46. Hatırlamak İçin Kısa Versiyon

Bu projeyi aylar sonra tekrar açtığında hatırlaman gereken ana fikir:

```text
1. UART veri aldığında RXNE oluşur.

2. RXNE interrupt çalışır.

3. DR register'dan byte okunur.

4. Byte RX Circular Buffer'a Enqueue edilir.

5. Application UARTx_ReadLine() ile buffer'ı okur.

--------------------------------------------------

6. Application veri göndermek istediğinde
   UARTx_Write_Char() çağrılır.

7. Veri TX Circular Buffer'a Enqueue edilir.

8. TXE interrupt aktif edilir.

9. TXE oluştuğunda buffer'dan Dequeue yapılır.

10. Byte DR register'ına yazılır.

11. Buffer boşalınca TXE interrupt kapatılır.
```

Yani driver'ın bütün mantığı aslında şu iki cümlede özetlenebilir:

> **RX tarafında UART'tan gelen veriyi interrupt ile buffer'a koyuyoruz.**

> **TX tarafında application'ın gönderdiği veriyi buffer'dan interrupt ile UART'a gönderiyoruz.**

Circular Buffer ise bu iki taraf arasındaki veri akışını geçici olarak tutan yapıdır.
