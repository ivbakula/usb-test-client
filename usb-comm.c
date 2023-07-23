#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <string.h>
#include <math.h>
#include <assert.h>

typedef struct
{
  uint32_t command;
  uint8_t data[60];
} USB_Packet;

typedef struct
{
  uint32_t command;
  uint32_t pll_registers[6];
} USB_PllSetFrequency;

#define SPI_WRITE_COMMAND 0x1
#define PLL_SET_FREQUENCY 0x2

#define PFD_FREQ 10.0e6

typedef union
{
  struct
  {
    uint32_t control_bits : 3;
    uint32_t frac_value : 12;
    uint32_t int_value : 16;
    uint32_t reserved : 1;
  };
  uint32_t data;
} Register0;

typedef union
{
  struct
  {
    uint32_t control_bits : 3;
    uint32_t modulus : 12;
    uint32_t phase : 12;
    uint32_t prescaler : 1;
    uint32_t reserved : 4;
  };
  uint32_t data;
} Register1;

typedef union
{
  struct
  {
    uint32_t control_bits : 3;
    uint32_t reset_ctr : 1;
    uint32_t tristate_cp : 1;
    uint32_t pd : 1;
    uint32_t pd_polarity : 1;
    uint32_t ldp : 1;
    uint32_t ldf : 1;
    uint32_t chargepump : 4;
    uint32_t doublebuff : 1;
    uint32_t r_counter : 10;
    uint32_t r_div_2 : 1;
    uint32_t ref_doubler : 1;
    uint32_t muxout : 3;
    uint32_t lownoise : 2;
    uint32_t reserved : 1;
  };
  uint32_t data;
} Register2;

typedef union
{
  struct
  {
    uint32_t control_bits : 3;
    uint32_t clock_div : 12;
    uint32_t clock_div_mode : 2;
    uint32_t reserved : 1;
    uint32_t csr : 1;
    uint32_t reserved_2 : 2;
    uint32_t reserver_3 : 11;
  };
  uint32_t data;
} Register3;

typedef union
{
  struct
  {
    uint32_t control_bits : 3;
    uint32_t power : 2;
    uint32_t power_en : 1;
    uint32_t aux_power : 2;
    uint32_t aux_en : 1;
    uint32_t aux_sel : 1;
    uint32_t mtld : 1;
    uint32_t vco_pwr_dwn : 1;
    uint32_t band_sel : 8;
    uint32_t div_sel : 3;
    uint32_t fb_sel : 1;
    uint32_t reserved : 8;
  };
  uint32_t data;
} Register4;

typedef union
{
  struct
  {
    uint32_t control_bits : 3;
    uint32_t reserved : 16;
    uint32_t reserved_2 : 2;
    uint32_t reserved_3 : 1;
    uint32_t ld_pin_mode : 2;
    uint32_t reserved_4 : 8;
  };
  uint32_t data;
} Register5;

USB_PllSetFrequency  set_frequency(double frequency)
{
  int bs_clk_div = 200;
  int phase = 1;
  int rf_div = (int)ceil(log(2200.0e6 / frequency) / log(2));

  double vco_freq = frequency * pow(2.0, (double)rf_div);
  double divider = vco_freq / PFD_FREQ;
  int iint = (int)(floor(divider));
  int prescaler = 0;
  
  if (vco_freq > 3000.0e6)
    prescaler = 1;

  double remainder = divider - (double)iint;
  int mod = 2;
  int frac = mod - 1;
  double eps = (fabs(remainder * (double)mod - (double)frac));

  while (mod < 4095 && eps > 1.0e-10) {
    mod += 1;
    frac = (int)round(mod * remainder);
    eps = (fabsl(remainder * (double)mod - (double)frac));
  }

  int ldf = (frac > 0) ? 0 : 1;

  USB_PllSetFrequency p = {0};
  p.command = PLL_SET_FREQUENCY;

  Register0 r0 = {.data = 0};
  r0 = (Register0){
      .control_bits = 0,
      .frac_value = frac,
      .int_value = iint,
  };
  
  printf("Check R0\n");
  assert(r0.control_bits == 0);
  assert(r0.frac_value == frac);
  assert(r0.int_value == iint);
  printf("R0 OK\n");


  Register1 r1 = {.data = 0};
  r1 = (Register1) {
      .control_bits = 1,
      .modulus = mod,
      .phase = phase,
      .prescaler = prescaler,
  };
  
  printf("Check R1\n");
  assert(r1.control_bits == 1);
  assert(r1.modulus == mod);
  assert(r1.phase == phase);
  assert(r1.prescaler == prescaler);
  printf("R1 OK\n");

  Register2 r2 = {.data = 0};
  r2 = (Register2){
      .control_bits = 2,
      .pd_polarity = 1,
      .chargepump = 7,
      .muxout = 0,
      .ldp = 1,
      .r_counter = 1,
      .ldf = ldf,
  };
  printf("Check R2\n");
  assert(r2.control_bits == 2);
  assert(r2.pd_polarity == 1);
  assert(r2.chargepump == 7);
  assert(r2.muxout == 0);
  assert(r2.ldp == 1);
  assert(r2.r_counter == 1);
  assert(r2.ldf == ldf);
  printf("R2 OK\n");

  Register3 r3 = {.data = 0};
  r3 = (Register3) {
      .control_bits = 3,
      .clock_div = 200,
      .clock_div_mode = 0,
  };
  printf("Check R3\n");
  assert(r3.control_bits == 3);
  assert(r3.clock_div == 200);
  assert(r3.clock_div_mode == 0);
  printf("R3 OK\n");

  Register4 r4 = {.data = 0};
  r4 = (Register4) {
      .control_bits = 4,
      .power_en = 1,
      .fb_sel = 1,
      .power = 0,
      .band_sel = bs_clk_div,
      .div_sel = rf_div,
  };

  printf("Check R4\n");
  assert(r4.control_bits == 4);
  assert(r4.power_en == 1);
  assert(r4.fb_sel == 1);
  assert(r4.power == 0);
  assert(r4.band_sel == bs_clk_div);
  assert(r4.div_sel == rf_div);
  printf("R4 OK\n");
  
  Register5 r5 = { .data = 0};
  r5 = (Register5){
      .control_bits = 5,
      .ld_pin_mode = 1,
  };
  printf("Check R5\n");
  assert(r5.control_bits == 5);
  assert(r5.ld_pin_mode == 1);
  printf("R5 OK\n");

  USB_PllSetFrequency packet = {
      .command = PLL_SET_FREQUENCY,
      .pll_registers = {r0.data, r1.data, r2.data, r3.data, r4.data, r5.data},
  };

  return packet;
}

int main()
{
  int ret = libusb_init(NULL);
  if (ret)
  {
    printf("%s:%d USB error: %s\n", __FILE__, __LINE__, libusb_error_name(ret));
    return -1;
  }

  libusb_device_handle *handle;
  handle = libusb_open_device_with_vid_pid(NULL, 0x6976, 0x616e);

  if (handle == NULL)
  {
    printf("%s:%d USB open device failed\n", __FILE__, __LINE__);
    return -1;
  }

  libusb_device *dev = libusb_get_device(handle);
  uint8_t bus = libusb_get_bus_number(dev);

  for (int iface = 0; iface < 2; iface++)
  {
    int r = libusb_claim_interface(handle, iface);

    if (r != LIBUSB_SUCCESS)
    {
      printf("%s:%d USB claim device failed with: %s\n", __FILE__, __LINE__,
             libusb_error_name(r));
    }
  }

  char str[256];
  libusb_get_string_descriptor(handle, 0, 0x409, (unsigned char *)str, 256);


  USB_Packet packet = {.command = SPI_WRITE_COMMAND,
                       .data = "Hello world!\r\n"};
  
  int len = 0;
  /* libusb_interrupt_transfer(handle, 0x01, (unsigned char *)&packet, */
  /*                           sizeof(packet), &len, 1000); */


  USB_PllSetFrequency config = set_frequency(90.0e6);
  len = 0;
  /* USB_PllSetFrequency adf435x_config_10MHz = { */
  /*   .command = PLL_SET_FREQUENCY, */
  /*   //    .pll_registers = {0x550010, 0x8008029, 0x2004E42, 0x4B3, 0x8A01FC, */
  /*   //    0x580005} */
  /*   .pll_registers = { 0x6C0000, 0x8008011, 0x2004E42, 0x4B3, 0x9A01FC, 0x580005}, */
  /* }; */

  libusb_interrupt_transfer(handle, 0x01,
                            (unsigned char *)&config,
                            sizeof(config), &len, 1000);
  
  /* char buffer[64]; */
  /* len = 0; */
  /* libusb_interrupt_transfer(handle, 0x81, buffer, 64, &len, 1000); */
  /* printf("len: %d %s\n", len, buffer); */


  libusb_exit(NULL);
}    

