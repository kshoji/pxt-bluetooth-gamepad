// This sample is based on sparkfun's gamer:bit, its buttons are `active low`.

// Pull up & set events
pins.setPull(DigitalPin.P0, PinPullMode.PullUp)
pins.setPull(DigitalPin.P1, PinPullMode.PullUp)
pins.setPull(DigitalPin.P2, PinPullMode.PullUp)
pins.setPull(DigitalPin.P8, PinPullMode.PullUp)
pins.setPull(DigitalPin.P12, PinPullMode.PullUp)
pins.setPull(DigitalPin.P16, PinPullMode.PullUp)
pins.setPull(DigitalPin.P5, PinPullMode.PullUp)
pins.setPull(DigitalPin.P11, PinPullMode.PullUp)

pins.setEvents(DigitalPin.P0, PinEventType.Edge)
pins.setEvents(DigitalPin.P1, PinEventType.Edge)
pins.setEvents(DigitalPin.P2, PinEventType.Edge)
pins.setEvents(DigitalPin.P8, PinEventType.Edge)
pins.setEvents(DigitalPin.P12, PinEventType.Edge)
pins.setEvents(DigitalPin.P16, PinEventType.Edge)
pins.setEvents(DigitalPin.P5, PinEventType.Edge)
pins.setEvents(DigitalPin.P11, PinEventType.Edge)

// Up
control.onEvent(DAL.MICROBIT_ID_IO_P0, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_UP, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P0, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_UP, ButtonState.BUTTON_DOWN)
})

// Left
control.onEvent(DAL.MICROBIT_ID_IO_P1, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_LEFT, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P1, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_LEFT, ButtonState.BUTTON_DOWN)
})

// Right
control.onEvent(DAL.MICROBIT_ID_IO_P2, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_RIGHT, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P2, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_RIGHT, ButtonState.BUTTON_DOWN)
})

// Down
control.onEvent(DAL.MICROBIT_ID_IO_P8, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_DOWN, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P8, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_DOWN, ButtonState.BUTTON_DOWN)
})

// A
control.onEvent(DAL.MICROBIT_ID_IO_P16, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_A, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P16, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_A, ButtonState.BUTTON_DOWN)
})

// B
control.onEvent(DAL.MICROBIT_ID_IO_P12, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_B, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P12, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_B, ButtonState.BUTTON_DOWN)
})

// select
control.onEvent(DAL.MICROBIT_ID_IO_P5, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_SELECT, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P5, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_SELECT, ButtonState.BUTTON_DOWN)
})

// start
control.onEvent(DAL.MICROBIT_ID_IO_P11, DAL.MICROBIT_PIN_EVT_RISE, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_START, ButtonState.BUTTON_UP)
})
control.onEvent(DAL.MICROBIT_ID_IO_P11, DAL.MICROBIT_PIN_EVT_FALL, () => {
    bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_START, ButtonState.BUTTON_DOWN)
})

// Bluetooth connection indicator
bluetooth.onBluetoothConnected(() => {
    basic.showString("C")
})

bluetooth.onBluetoothDisconnected(() => {
    basic.showString("D")
})

bluetooth.startGamepadService()
basic.showString("S")
