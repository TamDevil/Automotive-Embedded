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
## Bài 4: Communication Protocols
Truyền nhận dữ liệu giữa các MCU hay giữa MCU với các ngoại vi thực chất là sự truyền tín hiệu điện áp 3,3V hoặc 5v tương ứng với giá trị 1 và 0v tương ứng với giá trị 0 trong hệ nhị phân. Dùng các dãy 0 và 1 định nghĩa các dữ liệu phức tạp. Nhưng khi gửi 2 lần với cùng 1 giá trị thì các ngoại vi sẽ không phân biệt được đó là dữ liệu mới hay dữ liệu cũ chưa được gửi hoàn tất khiến sai lệch dữ liệu nên các chuẩn giao tiếp được ra đời.

### SPI (Serial Peripheral Interface)
- Chuẩn giao tiếp nối tiếp
Chỉ gửi hoặc nhận 1 bit trong một thời gian nhất định khác với giao tiếp song song có thể gửi hoặc nhận nhiều bit trong cùng một thời gian nhưng lại tốn phần cứng do cần dùng quá nhiều dây dữ liệu.
- Hoạt động ở chế độ song công (Có thể truyền - nhận cùng thời điểm).
  Bổ sung: đơn công là chỉ có 1 chức năng truyền hoặc nhận, bán song công chỉ có thể truyền hoặc nhận trong 1 thời điểm.
- Một Master có thể giao tiếp với nhiều Slave
- Đồng bộ giúp Master và Slave tương tác khi nào truyền hoặc nhận dữ liệu.
- Có 4 dây giao tiếp:
+ SCK (Serial Clock): Thiết bị Master tạo xung tín hiệu SCK và cung cấp cho Slave.
+ MISO (Master Input Slave Output): Tín hiệu tạo bởi thiết bị Slave và nhận bởi thiết bị Master.
+ MOSI (Master Output Slave Input): Tín hiệu tạo bởi thiết bị Master và nhận bởi thiết bị Slave. 
+ SS (Đôi khi CS- Slave Select/Chip Select): Chọn thiết bị Slave cụ thể để giao tiếp. Để chọn Slave giao tiếp thiết bị Master chủ động kéo đường SS tương ứng xuống mức 0 (Low). 
Nguyên lý hoạt động:
  Để bắt đầu quá trình Master đưa chân CS xuống 0v để chọn Slave muốn truyền/nhận dữ liệu sau đó kích hoạt xung cho chân SCK, với mỗi xung chân MISO sẽ truyền 1 bit từ Slave cho Master và chân MOSI sẽ truyền 1 bit từ Master cho Slave 

Có 4 chế độ SPI được tạo bởi tổ hợp CPOL và CPHA
- CPOL = 0 nghĩa là chế độ nghỉ của chân SCK ở mức low (0v)
- CPOL = 1 nghĩa là chế độ nghỉ của chân SCK ở mức High (3,3v hoặc 5v)
- CPHA = 0 nghĩa là sẽ thực hiện nhận tín hiệu ở thời điểm bắt đầu 1 xung và gửi tín hiệu ở thời điểm kết thúc 1 xung.
- CPHA = 1 nghĩa là sẽ thực hiện gửi tín hiệu ở thời điểm bắt đầu 1 xung và nhận tín hiệu ở thời điểm kết thúc 1 xung.

- Chế độ 0: CPOL = 0; CPHA = 0
Nhận dữ liệu khi SCK từ low lên high và truyền dữ liệu khi SCK từ high xuống low.
- Chế độ 1: CPOL = 0; CPHA = 1
Nhận dữ liệu khi SCK từ high xuống low và truyền dữ liệu khi SCK từ low lên high.
- Chế độ 2: CPOL = 1; CPHA = 0
Nhận dữ liệu khi SCK từ high xuống low và truyền dữ liệu khi SCK từ low lên high.
- Chế độ 3: CPOL = 1; CPHA = 1
Nhận dữ liệu khi SCK từ low lên high và truyền dữ liệu khi SCK từ high xuống low.

### I2C (Inter-Integrated Circuit)
Cũng gần giống SPI nhưng chỉ có 2 dây 1 dây để đồng bộ tín hiệu nhận/truyền dữ liệu như SCK là chân SCL (Serial Clock) chân còn lại là SDA(Serial Data) chỉ truyền hoặc nhận dữ liệu tại 1 thời điểm (chế độ bán song công). I2C khi không làm việc 2 chân sẽ rơi vào trạng thái Open-Drain (lơ lửng) không có 1 điện áp cụ thể nên sẽ mắc 2 dây vào nguồn 5v hoặc 3,3v thông qua 2 điện trở để tránh tình trạng này.

Nguyên lý hoạt động:
Để truyền/nhận dữ liệu thì chân SDA từ mức 1 xuống mức 0 trước chân SCL để hoàn tất điều kiện bắt đầu. Tiếp theo, vì không có chân SC như SPI để chỉ rõ Slave mục tiêu nên Master sẽ truyền 7 bit địa chỉ của Slave cần tìm và 1 bit biểu diễn trạng thái truyền hoặc nhận dữ liệu cho tất cả Slave. Slave trùng địa chỉ sẽ truyền bit ACK kéo SDA xuống 0v để xác nhận và truyền/nhận dữ liệu như bình thường. Điều kiện kết thúc là đưa chân SCL lên mức 1 trước chân SDA.

### UART
Không có cơ chế đồng bộ như I2C hay SPI, chỉ có 2 thiết bị cùng cấp tương tác với nhau và có dây truyền dữ liệu TX(transmit) và nhận dữ liệu RX(Receive). vì không có cơ chế đồng bộ nên phải tạo 1 bộ timer delay để quản lý thời điểm truyền/nhận dữ liệu và giữa các MCU có tần số xử lý khác nhau nên phải tạo ra 1 thời gian delay tiêu chuẩn dựa vào baudrate. Baudrate là số bits có thể truyền trong thời gian 1 giây và biến đổi nó để có được thời gian truyền/nhận 1 bit, từ đó tạo delay dựa trên thời gian đó để quản lý thời điểm truyền/nhận.

Nguyên lý hoạt động:
Để bắt đầu giao tiếp cần cho Tx của 1 thiết bị xuống 0 và delay 1 khoảng bằng với thời gian truyền 1 bit. Sau đó sẽ truyền/nhận dữ liệu mỗi bit rồi delay tiếp tục như vậy cho đến hết dữ liệu. Tiếp theo sẽ có cơ chế xác nhận dữ liệu thông qua việc truyền thêm 1 bit theo quy luật chẵn, lẽ. Dữ liệu có 3 lần số 1 thì sẽ thêm bit 1 hoặc bit 0 để trở thành quy luật chãn hoặc lẻ theo quy định từ trước. Nếu quy định là quy luật chẵn nhưng khi thêm bit Parity lại ra lẻ thì bị lỗi dữ liệu. Nhược điểm của parity là nếu số bit bị lỗi là chẵn thì sẽ không thể nhận biết được. Kết thúc quá trình giao tiếp bằng cách đưa Tx từ 0 lên 1 và delay 1 khoảng thời gian bằng thời gian gửi được 1 bit.
