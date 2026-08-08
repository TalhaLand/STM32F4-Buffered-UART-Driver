# STM32F4-Buffered-UART-Driver
Non-blocking UART Driver with Circular Buffer &amp; Custom Printf Support for STM32
# STM32F4 Non-Blocking UART Driver with Circular Buffer

Bu proje, STM32 mikrodenetleyicilerinde **kesme tabanlı (interrupt-driven)** ve **bloklamayan (non-blocking)** UART haberleşmesi sağlamak amacıyla geliştirilmiştir. Arka planda çift dairesel tampon (Circular Buffer) mimarisi kullanır.

## 🛠️ Öne Çıkan Özellikler

- **Circular Buffer Mimarisi:** Giriş (RX) ve Çıkış (TX) için bağımsız FIFO veri tamponlama ($512$ byte default).
- **Bloklamayan Tasarım:** `HAL_Delay` veya bekleme döngüleri olmadan arka planda ISR (`TXE` / `RXNE`) üzerinden iletim.
- **Özel Printf Desteği:** `vsnprintf` ve değişken argümanlar (`va_list`) kullanarak özelleştirilmiş, hızlı `UARTx_Printf` fonksiyonu.
- **Satır Bazlı Okuma:** `\r\n` sonlandırıcı karakterlerini otomatik tespit eden `UARTx_ReadLine` parsing yapısı.

## 📁 Proje Mimari Dosyaları

- `Circuler_Buffer.h` / `Circuler_Buffer.c` : Dairesel tampon oluşturma, Enqueue/Dequeue işleyicileri.
- `UART_ex.h` / `UART_ex.c` : Printf yönlendirmesi, string ve karakter iletim fonksiyonları.
- `stm32f4xx_it.c` : `USART2_IRQHandler` içerisinde `DR` yazmaç düzeyinde kesme yönetimi.
- `main.c` : Sürücü ve tampon ilklendirmeleri, örnek kullanım.
