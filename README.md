# bluetooth-gamepad

**Work in progress**

This PXT package allows the micro:bit to act as a Gamepad peripheral.

## Usage

Place a ``||bluetooth start gamepad service||`` block in your program to enable Bluetooth LE Gamepad.
With this block, the `micro:bit` starts advertise BLE packets as a Gamepad peripheral.

```blocks
bluetooth.startGamepadService();
```

For example, hold left gamepad button :

```blocks
bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_LEFT, ButtonState.BUTTON_DOWN);
```

## About test script (test.ts)

The micro:bit's memory(RAM) size is too small to run the test script.
To build & run the test script(test.ts), edit `pxt.json` file, and activate `"yotta" : "config"` part of the configuration.

This hack makes `BLE GATT table` smaller, and removes unnecessary BLE services(DFU, Event, and Device Information Service).

## Supported Platforms

Currently, tested with `micro:bit` and `Android` host only.
Mac OS X can connect with `micro:bit`, but it can't receive Gamepad messages.

## Supported targets

* for PXT/microbit

(The metadata above is needed for package search.)

## License

MIT

icon.png : copyright By Darkone - Own work, CC BY-SA 2.5, https://commons.wikimedia.org/w/index.php?curid=235633

```package
bluetooth
bluetooth-gamepad=github:kshoji/pxt-bluetooth-gamepad
```
