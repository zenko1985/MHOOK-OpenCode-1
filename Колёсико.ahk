#Requires AutoHotkey v2.0
SendMode("Input")
SetWorkingDir(A_ScriptDir)

; ===== настройки =====
global Doing := false
global Sensitivity := 1.0    ; множитель силы прокрутки
global PollInterval := 30    ; ms между проверками позиции мыши
global MinMove := 3          ; порог в пикселях для срабатывания
global WheelAmount := 120    ; стандартный дельта для одного "щелчка" колеса
global ShowToolTip := false

WheelSend(direction) {
    global WheelAmount, Sensitivity
    ; direction: 1 = вверх, -1 = вниз
    delta := Round(WheelAmount * Sensitivity * direction)
    ; MOUSEEVENTF_WHEEL = 0x0800
    DllCall("mouse_event", "UInt", 0x0800, "Int", 0, "Int", 0, "Int", delta, "UPtr", 0)
}

StartScrollLoop() {
    global Doing, PollInterval, MinMove, ShowToolTip

    Doing := true
    MouseGetPos(&x, &y)
    oldY := y

    if ShowToolTip
        ToolTip("Scroll: ON")

    while GetKeyState("LButton", "P") && GetKeyState("RButton", "P") {
        MouseGetPos(&x, &y)
        dy := y - oldY

        if Abs(dy) >= MinMove {
            notches := Floor(Abs(dy) / MinMove)
            if notches < 1
                notches := 1

            dir := (dy < 0) ? 1 : -1   ; вверх мышь -> вверх скролл

            Loop notches {
                WheelSend(dir)
                Sleep(8)
            }

            oldY := y
        }

        Sleep(PollInterval)
    }

    Doing := false

    if ShowToolTip
        ToolTip()
}

; Таймер следит за тем, что обе кнопки нажаты
SetTimer(CheckButtons, 40)

CheckButtons() {
    global Doing
    if !Doing && GetKeyState("LButton", "P") && GetKeyState("RButton", "P")
        StartScrollLoop()
}

; ===== тест / отладка =====
F9:: {
    global ShowToolTip
    ShowToolTip := !ShowToolTip
    ToolTip(ShowToolTip ? "ToolTip ON" : "ToolTip OFF")
    SetTimer(HideToolTip, -1000)
}

HideToolTip() {
    ToolTip()
}

F10::WheelSend(1)    ; тест: одно "вверх"
F11::WheelSend(-1)   ; тест: одно "вниз"
F12::MsgBox("AHK version: " A_AhkVersion)
