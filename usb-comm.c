#include <stdio.h>
#include <libusb-1.0/libusb.h>

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

  /* for (int iface = 0; iface < 2; iface++) */
  /* { */
  /*   int r = libusb_claim_interface(handle, iface); */

  /*   if (r != LIBUSB_SUCCESS) */
  /*   { */
  /*     printf("%s:%d USB claim device failed with: %s\n", __FILE__, __LINE__, */
  /*            libusb_error_name(r)); */
  /*   } */
  /* } */

  char str[256];
  libusb_get_string_descriptor(handle, 0, 0x409, (unsigned char *)str, 256);

  printf("String[0]: %s\n", str);
  
  libusb_exit(NULL);
}    

