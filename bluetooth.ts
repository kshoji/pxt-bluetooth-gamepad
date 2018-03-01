namespace bluetooth {
    /**
     * Starts the Gamepad service over Bluetooth and registers it as the Gamepad transport.
     */
    //% blockId="bluetooth_start_gamepad"
    //% block="bluetooth start gamepad service"
    //% parts="bluetooth"
    export function startGamepadService() {
        bluetooth.setGamepadButton(GamepadButton.GAMEPAD_BUTTON_LEFT, ButtonState.BUTTON_UP)
    }

    /**
     * Sets a Gamepad button
     */
    //% blockId="bluetooth_gamepad_set_button_state"
    //% block="gamepad|set button %button|to %state"
    //% parts="bluetooth"
    //% shim=bluetooth::gamepadButton
    //% advanced=true
    export function setGamepadButton(button: GamepadButton, state: ButtonState) {
    }

    /**
     * Gets the button
     */
    //% weight=2 blockGap=8
    //% blockId="button" block="%button"
    //% advanced=true
    export function button(button: GamepadButton): GamepadButton {
        return button
    }

    /**
     * Gets the button state
     */
    //% weight=2 blockGap=8
    //% blockId="buttonState" block="%buttonState"
    //% advanced=true
    export function buttonState(buttonState: ButtonState): ButtonState {
        return buttonState
    }
}
