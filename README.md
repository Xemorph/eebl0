<p align="center"><a href="https://elmorlabs.com/"><img width=50% src="./assets/elmorlabs_logo.png"></a></p>

<h4 align="center">A simple yet easy to use C-library to flash specific MCUs</h4>

<p align="center"> </p>

<div align="center">
  <a href="https://github.com/Xemorph/eebl0/blob/main/README.md#1-support">
    <img height="22px" src="assets/os-shield.jpg">
  </a>
</div>

<p align="center"></p>

# Table of contents
<!-- UPDATE with: doctoc --notitle README.md -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->


- [1. Prologue](#1-prologue)
- [2. Support](#2-support)
- [3. Usage](#3-usage)
  - [3.1 Interface](#31-interface)
  - [3.2 Example](#32-example)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## 1. Prologue
The basic idea of this C library is based on various sources. The program code was written by myself and did not follow any special scheme. Nevertheless I do not lay claim to my own program code and make it available to everyone free of charge.
The actual idea came from a KB article from the company Microchip, I link the article below. Furthermore the technical logic came from ElmorLabs. From him I got the right to use his logo only for non-commercial use.

I offer no warranty, guarantee and no support. A cyclic release of the library is excluded for private reasons, I simply don't have the time and the resources.

Link to the mentioned [KB article](https://microchip.my.site.com/s/article/Low-footprint-SERCOM-UART-bootloader-for-SAM-D10-SAM-D11-devices)

## 2. Support
The library has been created for the most platforms but uses internally the library RS232 which needs to be compiled for your platform.

## 3. Usage
### 3.1 Interface
```c
[...]

struct EEBL0;

/* OBJECT/specific functionality */
struct EEBL0 * EEBL0_create(void);
void EEBL0_free(struct EEBL0 ** self);

void EEBL0_setPort(struct EEBL0 * self, const char * const port);
const char * const EEBL0_getPort(struct EEBL0 * self);

const char * const EEBL0_welcome(void);
const char * const EEBL0_getStatus(struct EEBL0 * self);

/* CONNECTION-specific functionality */
bool EEBL0_connect(struct EEBL0 * self);
void EEBL0_disconnect(struct EEBL0 * self);
/* FIRMWARE-specific functionality */
bool EEBL0_setAndLoadFirmware(struct EEBL0 * self, const char * const fw_location);
bool EEBL0_readFirmwareFromDevice(struct EEBL0 * self);
bool EEBL0_eraseFirmware(struct EEBL0 * self);
bool EEBL0_verifyFirmware(struct EEBL0 * self);
bool EEBL0_flashFirmware(struct EEBL0 * self);

[...]
```
### 3.2 Example
```c
#include <eebl0/eebl0.h>

int main(void) {
  struct EEBL0 * eebl0 = EEBL0_create();
  EEBL0_setPort(eebl0, "COM4");
  bool status = EEBL0_connect(eebl0);

  printf("Welcome %s:\n--------------\n - Port(%s)\n - Status(%s)\n",
      EEBL0_welcome(), EEBL0_getPort(eebl0), EEBL0_getStatus(eebl0));

  printf("\nErasing... %s\n", (EEBL0_erase(eebl0) ? "OK" : "FAIL"));

  EEBL0_free(&eebl0);

  printf("\nPointer of 'eebl0' is %p\n", eebl0);

  return(0);
}
```
