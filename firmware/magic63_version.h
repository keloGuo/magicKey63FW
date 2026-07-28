#ifndef MAGIC63_VERSION_H
#define MAGIC63_VERSION_H

/*
 * Firmware version string is injected by CMake as MAGIC63_FIRMWARE_VERSION.
 *
 * USB bcdDevice policy:
 * - Use packed BCD in the form 0xMMmm, where MM is the public major version
 *   and mm is the public minor version.
 * - Development builds before the first public release use 0x0100.
 */
#define MAGIC63_USB_BCD_DEVICE 0x0100u

/*
 * Persistent configuration schema version.
 *
 * When this value changes, the current firmware restores the main persistent
 * configuration and keymap to defaults instead of attempting migration.
 */
#define MAGIC63_CONFIG_VERSION 3u

#endif
