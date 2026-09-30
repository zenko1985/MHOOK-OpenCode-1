#Requires AutoHotkey v2.0

~LButton:: {
    while GetKeyState("LButton", "P") {
        Click
        Sleep 50
    }
}