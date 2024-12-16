# Automotive Embedded
## Bài 1: Setup project đầu tiên trên KeilC
Quá trình lập trình cho vi điều khiển: Từ ngôn ngữ bậc cao (ngôn ngữ c) chuyển sang ngôn ngữ máy (hex) rồi nạp cho vi điều khiển.
### Cách setup project
- Cấp xung clock cho ngoại vi
  Giống như mình muốn sử dụng ngoại vi nào thì cần truyền năng lượng vào để ngoại vi đó hoạt động.
  ví dụ:
  Để cấp xung clock cho GPIO cần xác định bus truyền xung clock (APB2). Sau đó xác định địa chỉ đầu tiên của thanh ghi RCC (nhận trách nhiệm quản lý xung clock cấp cho các ngoại vi)
  rồi offset với địa chỉ bus APB2.
  ```
  #define RCC_APB2ENR *((unsigned int *)0x40021018) // RCC:0x40021000 + APB2:0x18
  RCC_APB2ENR |= (1 << 4); // Kich hoat xung clock cap cho GPIOC
  ```
- Cấu hình chân của ngoại vi
  Sau khi cấp xung clock cho ngoại vi hoạt động cần chỉ rõ chức năng của ngoại vi chẳng hạn như làm đầu vào/đầu ra thông thường hoặc làm chân chức năng đặc biệt của ngoại vi (UART, SPI, ADC, v.v.).
  ví dụ:
  Để xác định chức năng cho chân GPIOC_Pin_13 ta cần xác định địa chỉ của thanh ghi GPIOC sau đó chọn offset với địa chỉ thanh ghi CRL (từ chân 0 đến 7) hoặc thanh ghi CRH (từ chân 8 đến 15).
  ```
  #define GPIOC_CRH		*((unsigned int *)0x40011004) // GPIOC: 0x40011000 + CRH:0x04
  GPIOC->CRH |= (3 << 20);    // Set Output mode, max speed 50 MHz
	GPIOC->CRH &= ~(3 << 22);   // Set General purpose output push-pull
  ```
  - Sử dụng ngoại vi
    Sau khi hoàn thành 2 bước trên ta có thể đọc hoặc ghi dư liệu từ các ngoại vi đó thông qua các thanh ghi.
    ví dụ:
    Để xuất tín hiệu 0 hoặc 1 ra chân GPIOC_Pin_13 ta cần biết địa chỉ của GPIOC và offset với ODR.
    ```
    #define GPIOC_ODR		*((unsigned int *)0x4001100C) // GPIOC:0x40011000 + ODR:0x0C
    GPIOC_ODR |= (1 << 13); // xuất tín hiệu 1 ra chân PC13
    GPIOC_ODR &= ~(1 << 13); // xuất tín hiệu 0 ra chân PC13
    ```

    ## Bài 2: GPIO
    Dung thư viện SPL để đơn giản hóa các việc như cấp xung clock cho ngoại vi, cấu hình chân của ngoại vi và sử dụng ngoại vi mà không cần biết rõ về địa chỉ của từng thanh ghi.
    Ví dụ:
    Để cấp xung clock cho GPIOC ta có hàm:
    ```
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // Với 2 tham số truyền vào là ngoại vi cần cấp xung clock và trạng thái bật hoặc tắt xung clock.
    ```
    Để cấu hình chân ngoại vi:
    Ta cần khởi tạo 1 biến có kiểu dữ liệu struct `GPIO_InitTypeDef ` do thư viện SPL cung cấp để khởi tạo giá trị tương ứng cho các thành viên và gọi hàm `GPIO_Init()` để cập nhật các giá trị đưa vào.
    ```
    GPIO_InitTypeDef GPIO_InitStruct;  // khởi tạo biến kiểu GPIO_InitTypeDef
    
  	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13; // Cấu hình cho chân thứ 13
  	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  // Xuất tín hiệu đầu ra
  	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;`// Tốc độ của ngoại vi
  	
  	GPIO_Init(GPIOC, &GPIO_InitStruct); // Lưu các cài đặt vào thanh ghi
    ```
    Để sử dụng ngoại vi:
    Ta chỉ cần gọi các hàm đọc và ghi dữ liệu và truyền tham số tương ứng.
    ```
    uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    \\Đọc giá trị 1 chân trong GPIO được cấu hình là INPUT
    uint16_t GPIO_ReadInputData(GPIO_TypeDef* GPIOx);
    \\Đọc giá trị nguyên GPIO được cấu hình là INPUT
    uint8_t GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    \\Đọc giá trị 1 chân trong GPIO được cấu hình là OUTPUT
    uint16_t GPIO_ReadOutputData(GPIO_TypeDef* GPIOx);
    \\Đọc giá trị nguyên GPIO được cấu hình là OUTPUT
    void GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    \\Cho giá trị điện áp của 1 chân trong GPIO = 1
    void GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
    \\Cho giá trị điện áp của 1 chân trong GPIO = 0
    void GPIO_WriteBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, BitAction BitVal);
    \\Ghi giá trị "BitVal" vào 1 chân trong GPIO
    void GPIO_Write(GPIO_TypeDef* GPIOx, uint16_t PortVal);
    \\Ghi giá trị "PortVal" vào nguyên GPIO
    ```
Ví dụ tổng quát:
```
#include "stm32f10x.h"                  // Device header
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO

void RCC_Config(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA,ENABLE); // Cấp xung clock cho GPIOC và GPIOA
}

void delay(uint32_t time){    // Tạo hàm delay đơn giản
	int i;
	for(i = 0; i < time; i++){}
	}

void GPIO_Config(){
	GPIO_InitTypeDef gpio_init;
	// output
	gpio_init.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;  // Set chế độ cho các chân 13, 5, 6, 7, 8.
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;    // Xuất tín hiệu đầu ra
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;   // Tốc độ xử lý 
	
	GPIO_Init(GPIOC, &gpio_init);    // hàm cập nhật những thiết lập cho GPIOC
	// input
	gpio_init.GPIO_Pin = GPIO_Pin_0;    // Set chế độ cho chân 0
	gpio_init.GPIO_Mode = GPIO_Mode_IPU; // Nhận tín hiệu đầu vào
	GPIO_Init(GPIOA, &gpio_init);    // hàm cập nhật những thiết lập cho GPIOA
}

void chaseLed(uint8_t loop){    // Hàm nháy đuổi 
	int i,j;
	for(j = 0; j < loop; j++){
	uint16_t ledval = 0x0010;
	for(i = 0; i < 4; i++){
	ledval = ledval << 1;
	GPIO_Write(GPIOC, ledval);    // Ghi dữ liệu cho các chân của GPIOC
	delay(10000000);
	}
	}
}
int main() {
 	RCC_Config();
	GPIO_Config();
	
	while(1){
	//chaseLed(4);
	//break;
	//GPIO_SetBits(GPIOC, GPIO_Pin_13);
	//delay(1000000);
	//GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	//delay(1000000);
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET){
			while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0));
			if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == Bit_SET){
				GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);    // Set giá trị 0 cho GPIOC_Pin_13
			}else
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);  // Set giá trị 1 cho GPIOC_Pin_13
	}
}
}
```

## Bài 3: Interrupt - Timer
### Interrupt
Ngắt là khi có 1 sự kiện khẩn cấp xảy ra trong hoặc ngoài vi điều khiển yêu cầu chương trình chính phải dừng thực thi và nhảy sang sự kiện ngắt để giải quyết khi nào xong mới nhảy lại chương trình chính để tiếp tục thực thi. Mỗi loại ngắt sẽ có 1 hàm ngắt riêng và tự động chạy khi nhận dược tín hiệu ngắt gọi là ISR. Địa chỉ của các ISR gọi là vector ngắt.
Cơ chế hoạt động:
Khi Program Counter(PC) đang trỏ tới địa chỉ sắp thực thi thì có ngắt xảy ra, nó sẽ lưu địa chỉ đó vào Main Stack Pointer và trỏ tới vector ngắt sau khi thực hiện xong nó sẽ lấy địa chỉ từ MSP để thực thi tiếp chương trình chính.
Có 3 ngoại ngắt:
- Ngắt ngoài:
  Xảy ra khi có sự thay đổi điện áp từ các chân nhận tín hiệu ngắt.
  OW: kích hoạt ngắt liên tục khi chân ở mức thấp.
  HIGH: Kích hoạt liên tục khi chân ở mức cao.
  RISING: Kích hoạt khi trạng thái trên chân chuyển từ thấp lên cao.
  FALLING: Kích hoạt khi trạng thái trên chân chuyển từ cao xuống thấp.
- Ngắt Timer
  Kích hoạt ngắt sau một thời gian nhất định theo cớ chế overflow(đếm lên đến khi tới giá trị lớn hơn giá trị có thể lưu trữ) hoặc underflow(đếm xuống đến khi tới giá trị nhỏ hơn giá trị nhỏ nhất có thể lưu trữ)
- Ngắt truyền thông
  Xảy ra khi có sự truyền hoặc nhận dữ liệu giữa các MCU giúp động bộ việc truyền và nhận dữ liệu tránh bị miss dữ liệu.
  Độ ưu tiên ngắt
  Nếu nhiều ngắt cùng thực hiện đồng thời sẽ gây ra xung đột nên cần phải cấp độ ưu tiên cho từng ngắt. Mức độ ưu tiên từ 0 đến 15 với số càng nhỏ thì độ ưu tiên càng cao.
  ví dụ:
  chương trình chính đang thực hiện thì nhảy sang ngắt ngoài, khi ngắt ngoài đang thực hiện thì có thêm ngắt timer lúc này nếu độ ưu tiên của ngắt ngoài cao hơn thì sẽ chạy tiếp ngắt ngoài sau đó mới tới ngắt timer và cuối cùng là chương trình chính, nhưng nếu ngắt ngoài có độ ưu tiên thấp hơn thì sẽ dừng ngắt ngoài và chạy ngắt timer, sau khi ngắt timer kết thúc mới thực hiện tiếp ngắt ngoài và cuối cùng là chương trình chính.
  ### Timer
Timer dùng để đếm chu kỳ xung clock. Timer còn có thể hoạt động ở chế độ nhận xung clock từ các tín hiệu ngoài. Ngoài ra còn các chế độ khác như PWM, định thời …vv.
Thường MCU sẽ có tần số xử lý là 72Mhz nghĩa là 1 chu kỳ xung clock sẽ mất 1/72000000s nên thường timer dùng để tạo hàm delay với thời gian chính xác.
```
  uint16_t TIM_Prescaler;   // hàm set với bao nhiêu tần số thì timer sẽ đếm 1 lần

  uint16_t TIM_CounterMode;  // hàm set chế độ đếm cho timer: đếm lên, đếm xuống,..

  uint16_t TIM_Period;        // Hàm set số lần đếm của timer. tôi đa 16 bit

  uint16_t TIM_ClockDivision; // Hàm chia nhỏ xung clock trước khi timer sử dụng
```
```
TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // Bật clock cho TIM2

TIM_TimeBaseInitStruct.TIM_Prescaler = 7199;         // Chia clock 72 MHz xuống còn 10 kHz
TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; // Đếm lên
TIM_TimeBaseInitStruct.TIM_Period = 9999;           // Đếm 10,000 chu kỳ (1 giây)
TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1; // Không chia thêm clock
TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);    // Cấu hình Timer

TIM_Cmd(TIM2, ENABLE); // Bắt đầu Timer
```





    
