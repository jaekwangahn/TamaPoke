# GeekMagic SmallTV PRO touch firmware

Target board: the SmallTV PRO with a classic ESP32, 8 MB flash, a 240×240 ST7789V LCD, and the GPIO32 touch pad on top of the enclosure. See the [board pin map](https://raw.githubusercontent.com/giovi321/smalltv-mod/main/src/board_esp32_pro.h).

## Play with the top touch pad

Leave the pad untouched during startup so the firmware can measure its resting value.

- Tap once to highlight the next selectable tile. Wait for the yellow outline, then double-tap to activate that tile.
- Hold for about a second and release to open or close the gesture menu. Tap once to move between its options and double-tap to run the highlighted gesture. From the pet screen, **LEFT** opens Activities, **RIGHT** opens the Pokédex, **UP** opens the pet profile, **DOWN** opens settings, and **PET** opens the collect/release dialog.
- A tap wakes the dimmed screen. Select the egg, fishing prompt or mining rock and double-tap to interact.

The firmware includes the Pokédex thumbnails and uses them for pets without an animated sprite. Local game saves use NVS. The farm, album, and backups use internal LittleFS. Wi-Fi can synchronize the clock after a power cycle. This board build has no speaker, motion sensor, or external clock integration.

## Build

Install ESP32 Arduino core 3.3.10 and the libraries listed in the [main README](../README.md#build-and-flash). From the repository root run:

```sh
./tools/build_smalltv_pro.sh
```

The app image is `build/smalltv-pro/TamaPoke.ino.bin`. The initial UART installation image, which also contains the bootloader and partition table, is `build/smalltv-pro/TamaPoke.ino.merged.bin`.

## Install through an existing OTA updater

Run `./tools/build_smalltv_pro.sh --ota` and select `build/smalltv-pro-ota/TamaPoke.ino.bin` in the device's web updater. This build targets a 2,228,224-byte OTA slot. It opens compatible LittleFS storage without formatting it and can reuse the ESP32's saved Wi-Fi station configuration. If the device stays offline after the update, choose **DOWN** from the gesture menu and open Wi-Fi setup.

With TamaPoke's updater, open **Settings → OTA UPDATE** on the device. On the same Wi-Fi network, open [tamapoke.local/update](http://tamapoke.local/update), enter the six-digit PIN displayed on the device, and choose the app BIN. The PIN is valid for three minutes and is consumed when an upload begins. The updater validates the image in the inactive app partition before switching the boot slot. NVS, LittleFS, and the partition table are preserved.

For an installed TamaPoke build without an updater, connect via UART and read the device's partition table and boot selection first. For a data-preserving upgrade, write only the app BIN to the confirmed application slot. Keep the existing partition table, boot selection, NVS, and LittleFS. The merged image below is for initial installation that replaces those regions.

## First flash over UART

Use a 3.3 V logic USB-to-UART adapter on the internal header. Connect ground, adapter TX to RXD0, and adapter RX to TXD0. Hold GPIO0 at ground during power-up to enter the ESP32 downloader. Follow the [SmallTV PRO UART pad map](https://github.com/ViToni/esphome-geekmagic-smalltv#prepare-the-smalltv-pro-for-esphome) for your board.

The merged image replaces the full 8 MB flash, including the original firmware and saved settings. Read a backup before the first flash if you want to restore the factory image later.

```sh
python3 -m esptool --chip esp32 --port /dev/cu.usbserial-XXXX read-flash 0x0 0x800000 smalltv-stock.bin
python3 -m esptool --chip esp32 --port /dev/cu.usbserial-XXXX write-flash 0x0 build/smalltv-pro/TamaPoke.ino.merged.bin
```

Disconnect GPIO0 from ground and power-cycle after writing. For subsequent Arduino uploads on the same partition layout, use the build's separate bootloader, partition, and app files so the saved NVS and LittleFS partitions are preserved.
