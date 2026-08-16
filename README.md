# STM32F4 Buffered UART Driver

Kısa açıklama

## 1. Quick Start
### 1.1 Driver dosyalarını projeye ekleme
### 1.2 CubeMX / CubeIDE UART ayarları
### 1.3 main.c içerisine eklenmesi gerekenler
### 1.4 stm32f4xx_it.c içerisinde yapılması gerekenler
### 1.5 Driver'ı initialize etme

## 2. UART Kullanımı

### Veri gönderme
UARTx_Write_Char()
UARTx_Put_String()
UARTx_Printf()

### Veri alma
UARTx_ReadLine()

### Örnek
PC → STM32
STM32 → PC

## 3. Driver Architecture

UART
 ↓
Interrupt
 ↓
Circular Buffer
 ↓
UART Driver
 ↓
Application

## 4. Circular Buffer

### Neden Circular Buffer?
### head / tail
### Enqueue
### Dequeue
### Buffer full / empty
### Count

## 5. UART Interrupt Mekanizması

### RXNE
### TXE
### USART2_IRQHandler()
### Neden DR register'ı kullanıyoruz?

## 6. UART Driver

### UARTx_Initialization()
### UARTx_Write_Char()
### UARTx_Put_String()
### UARTx_Printf()
### UARTx_ReadLine()

Her fonksiyon:
- Ne yapıyor?
- Neden gerekli?
- Nasıl çalışıyor?
- Önemli noktalar

## 7. UARTx_ReadLine() Mantığı

CR
CR + LF
messageReady
index
maxLen

## 8. Neden Bu Tasarımı Seçtik?

Blocking UART yerine neden buffer?
Polling yerine neden interrupt?
Neden iki buffer?
Neden TXE interrupt?

## 9. Baştan Yazmak İstersem

Circular Buffer
→ UART RX
→ UART TX
→ Interrupt
→ Driver API

adım adım geliştirme sırası

## 10. Configuration

Buffer size
Baud rate
UART instance

## 11. Limitations / Known Issues

## 12. Project Structure

## 13. Example
