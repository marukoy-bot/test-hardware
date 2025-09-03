//parts of his code are taken from
//https://github.com/igrr/esp32-cam-demo
//by Ivan Grokhotkov
//released under Apache License 2.0

#include "I2SCamera.h"
#include "Log.h"
#include "rom/gpio.h"

int I2SCamera::blocksReceived = 0;
int I2SCamera::framesReceived = 0;
int I2SCamera::xres = 640;
int I2SCamera::yres = 480;
gpio_num_t I2SCamera::vSyncPin = GPIO_NUM_0;
intr_handle_t I2SCamera::i2sInterruptHandle = nullptr;
intr_handle_t I2SCamera::vSyncInterruptHandle = nullptr;
int I2SCamera::dmaBufferCount = 0;
int I2SCamera::dmaBufferActive = 0;
DMABuffer **I2SCamera::dmaBuffer = nullptr;
unsigned char* I2SCamera::frame = nullptr;
int I2SCamera::framePointer = 0;
int I2SCamera::frameBytes = 0;
volatile bool I2SCamera::stopSignal = false;

void IRAM_ATTR I2SCamera::i2sInterrupt(void* arg)
{
    I2S0.int_clr.val = I2S0.int_raw.val;
    blocksReceived++;
    
    if (!dmaBuffer || dmaBufferActive >= dmaBufferCount || !frame) {
        return; // Safety check
    }
    
    unsigned char* buf = dmaBuffer[dmaBufferActive]->buffer;
    dmaBufferActive = (dmaBufferActive + 1) % dmaBufferCount;
    
    if(framePointer < frameBytes)
    {
        for(int i = 0; i < xres * 4; i += 4)
        {
            if (framePointer + 1 < frameBytes) { // Bounds check
                frame[framePointer++] = buf[i + 2];
                frame[framePointer++] = buf[i];
            }
        }
    }
    
    if (blocksReceived >= yres)
    {
        framePointer = 0;
        blocksReceived = 0;
        framesReceived++;
        if(stopSignal)
        {
            i2sStop();
            stopSignal = false;
        }
    }
}

void IRAM_ATTR I2SCamera::vSyncInterrupt(void* arg)
{
    GPIO.status1_w1tc.val = GPIO.status1.val;
    GPIO.status_w1tc = GPIO.status;
    if(gpio_get_level(vSyncPin))
    {
        //frame done
    }
}

void I2SCamera::i2sStop()
{
    if (i2sInterruptHandle) {
        esp_intr_disable(i2sInterruptHandle);
    }
    if (vSyncInterruptHandle) {
        esp_intr_disable(vSyncInterruptHandle);
    }
    i2sConfReset();
    I2S0.conf.rx_start = 0;
}

void I2SCamera::i2sRun()
{
    DEBUG_PRINTLN("I2S Run");
    
    // Wait for vsync
    while (gpio_get_level(vSyncPin) == 0);
    while (gpio_get_level(vSyncPin) != 0);

    if (i2sInterruptHandle) {
        esp_intr_disable(i2sInterruptHandle);
    }
    
    i2sConfReset();
    blocksReceived = 0;
    dmaBufferActive = 0;
    framePointer = 0;
    
    if (dmaBuffer && dmaBuffer[0]) {
        DEBUG_PRINT("Sample count ");
        DEBUG_PRINTLN(dmaBuffer[0]->sampleCount());
        I2S0.rx_eof_num = dmaBuffer[0]->sampleCount();
        I2S0.in_link.addr = (uint32_t)&(dmaBuffer[0]->descriptor);
        I2S0.in_link.start = 1;
        I2S0.int_clr.val = I2S0.int_raw.val;
        I2S0.int_ena.val = 0;
        I2S0.int_ena.in_done = 1;
        
        if (i2sInterruptHandle) {
            esp_intr_enable(i2sInterruptHandle);
        }
        if (vSyncInterruptHandle) {
            esp_intr_enable(vSyncInterruptHandle);
        }
        I2S0.conf.rx_start = 1;
    }
}

bool I2SCamera::initVSync(int pin)
{
    DEBUG_PRINT("Initializing VSYNC... ");
    vSyncPin = (gpio_num_t)pin;
    
    esp_err_t err = gpio_set_intr_type(vSyncPin, GPIO_INTR_POSEDGE);
    if (err != ESP_OK) {
        DEBUG_PRINTLN("failed to set interrupt type!");
        return false;
    }
    
    err = gpio_intr_enable(vSyncPin);
    if (err != ESP_OK) {
        DEBUG_PRINTLN("failed to enable interrupt!");
        return false;
    }
    
    err = gpio_isr_register(vSyncInterrupt, (void*)"vSyncInterrupt", 
                           ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_IRAM, 
                           &vSyncInterruptHandle);
    if(err != ESP_OK) 
    {
        DEBUG_PRINTLN("failed!");
        return false;
    }
    DEBUG_PRINTLN("done.");
    return true;
}

void I2SCamera::deinitVSync()
{
    if (vSyncInterruptHandle) {
        esp_intr_disable(vSyncInterruptHandle);
        esp_intr_free(vSyncInterruptHandle);
        vSyncInterruptHandle = nullptr;
    }
}

bool I2SCamera::init(const int XRES, const int YRES, const int VSYNC, const int HREF, 
                     const int XCLK, const int PCLK, const int D0, const int D1, 
                     const int D2, const int D3, const int D4, const int D5, 
                     const int D6, const int D7)
{
    xres = XRES;
    yres = YRES;
    frameBytes = XRES * YRES * 2;
    
    // Free existing frame buffer if any
    if (frame) {
        free(frame);
        frame = nullptr;
    }
    
    frame = (unsigned char*)malloc(frameBytes);
    if(!frame)
    {
        DEBUG_PRINTLN("Not enough memory for frame buffer!");
        return false;
    }
    
    if (!i2sInit(VSYNC, HREF, PCLK, D0, D1, D2, D3, D4, D5, D6, D7)) {
        free(frame);
        frame = nullptr;
        return false;
    }
    
    dmaBufferInit(xres * 2 * 2);  //two bytes per dword packing, two bytes per pixel
    
    if (!initVSync(VSYNC)) {
        dmaBufferDeinit();
        free(frame);
        frame = nullptr;
        return false;
    }
    
    return true;
}

bool I2SCamera::i2sInit(const int VSYNC, const int HREF, const int PCLK, 
                        const int D0, const int D1, const int D2, const int D3, 
                        const int D4, const int D5, const int D6, const int D7)
{    
    int pins[] = {VSYNC, HREF, PCLK, D0, D1, D2, D3, D4, D5, D6, D7};    
    gpio_config_t conf = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    for (int i = 0; i < sizeof(pins) / sizeof(int); ++i) {
        conf.pin_bit_mask = 1ULL << pins[i];
        esp_err_t err = gpio_config(&conf);
        if (err != ESP_OK) {
            DEBUG_PRINTLN("GPIO config failed!");
            return false;
        }
    }

    // Route input GPIOs to I2S peripheral using GPIO matrix
    gpio_matrix_in(D0,    I2S0I_DATA_IN0_IDX, false);
    gpio_matrix_in(D1,    I2S0I_DATA_IN1_IDX, false);
    gpio_matrix_in(D2,    I2S0I_DATA_IN2_IDX, false);
    gpio_matrix_in(D3,    I2S0I_DATA_IN3_IDX, false);
    gpio_matrix_in(D4,    I2S0I_DATA_IN4_IDX, false);
    gpio_matrix_in(D5,    I2S0I_DATA_IN5_IDX, false);
    gpio_matrix_in(D6,    I2S0I_DATA_IN6_IDX, false);
    gpio_matrix_in(D7,    I2S0I_DATA_IN7_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN8_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN9_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN10_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN11_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN12_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN13_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN14_IDX, false);
    gpio_matrix_in(0x30,  I2S0I_DATA_IN15_IDX, false);
        
    gpio_matrix_in(VSYNC, I2S0I_V_SYNC_IDX, true);
    gpio_matrix_in(0x38,  I2S0I_H_SYNC_IDX, false);  //0x30 sends 0, 0x38 sends 1
    gpio_matrix_in(HREF,  I2S0I_H_ENABLE_IDX, false);
    gpio_matrix_in(PCLK,  I2S0I_WS_IN_IDX, false);

    // Enable and configure I2S peripheral
    periph_module_enable(PERIPH_I2S0_MODULE);
    
    // Reset I2S configuration
    i2sConfReset();
    
    // Enable slave mode (sampling clock is external)
    I2S0.conf.rx_slave_mod = 1;
    // Enable parallel mode
    I2S0.conf2.lcd_en = 1;
    // Use HSYNC/VSYNC/HREF to control sampling
    I2S0.conf2.camera_en = 1;
    // Configure clock divider
    I2S0.clkm_conf.clkm_div_a = 1;
    I2S0.clkm_conf.clkm_div_b = 0;
    I2S0.clkm_conf.clkm_div_num = 2;
    // FIFO will sink data to DMA
    I2S0.fifo_conf.dscr_en = 1;
    // FIFO configuration - two bytes per dword packing
    I2S0.fifo_conf.rx_fifo_mod = SM_0A0B_0C0D;
    I2S0.fifo_conf.rx_fifo_mod_force_en = 1;
    I2S0.conf_chan.rx_chan_mod = 1;
    // Clear flags which are used in I2S serial mode
    I2S0.sample_rate_conf.rx_bits_mod = 0;
    I2S0.conf.rx_right_first = 0;
    I2S0.conf.rx_msb_right = 0;
    I2S0.conf.rx_msb_shift = 0;
    I2S0.conf.rx_mono = 0;
    I2S0.conf.rx_short_sync = 0;
    I2S0.timing.val = 0;

    // Allocate I2S interrupt, keep it disabled
    esp_err_t err = esp_intr_alloc(ETS_I2S0_INTR_SOURCE, 
                                   ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM, 
                                   i2sInterrupt, NULL, &i2sInterruptHandle);
    if (err != ESP_OK) {
        DEBUG_PRINTLN("Failed to allocate I2S interrupt!");
        return false;
    }
    
    return true;
}

void I2SCamera::dmaBufferInit(int bytes)
{
    // Clean up existing buffers
    dmaBufferDeinit();
    
    dmaBufferCount = 2;
    dmaBuffer = (DMABuffer**) malloc(sizeof(DMABuffer*) * dmaBufferCount);
    if (!dmaBuffer) {
        DEBUG_PRINTLN("Failed to allocate DMA buffer array!");
        dmaBufferCount = 0;
        return;
    }
    
    for(int i = 0; i < dmaBufferCount; i++)
    {
        dmaBuffer[i] = new DMABuffer(bytes);
        if (!dmaBuffer[i]) {
            DEBUG_PRINTLN("Failed to create DMA buffer!");
            // Clean up partial allocation
            for (int j = 0; j < i; j++) {
                delete dmaBuffer[j];
            }
            free(dmaBuffer);
            dmaBuffer = nullptr;
            dmaBufferCount = 0;
            return;
        }
        if(i > 0)
            dmaBuffer[i-1]->next(dmaBuffer[i]);
    }
    if (dmaBufferCount > 0)
        dmaBuffer[dmaBufferCount - 1]->next(dmaBuffer[0]);
}

void I2SCamera::dmaBufferDeinit()
{
    if (!dmaBuffer) return;
    
    for(int i = 0; i < dmaBufferCount; i++)
    {
        if (dmaBuffer[i])
            delete dmaBuffer[i];
    }
    free(dmaBuffer);
    dmaBuffer = nullptr;
    dmaBufferCount = 0;
}