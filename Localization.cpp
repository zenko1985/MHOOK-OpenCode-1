#include "Localization.h"
int MHLanguage = MH_LANG_RU;
static const TCHAR* strings_RU[LOC_STRING_COUNT] = {
	L"MHook V2",                                     // 0 LOC_APP_TITLE
	L"Из мыши в клавиатуру: настройка",              // 1 LOC_DIALOG1_TITLE
	L"Из мыши в клавиатуру: дополнительная настройка",// 2 LOC_DIALOG2_TITLE
	L"ЗАПУСК",                                       // 3 LOC_BTN_START
	L"ВЫХОД",                                        // 4 LOC_BTN_EXIT
	L"Сохранить...",                                 // 5 LOC_BTN_SAVE
	L"Загрузить...",                                 // 6 LOC_BTN_LOAD
	L"WASD",                                         // 7
	L"Окна-клавиши...",                              // 8 LOC_BTN_MAGIC_WINDOWS
	L"По окну",                                      // 9 LOC_BTN_LOAD_BY_WINDOW
	L"Чувствительность в пикс",                     // 10
	L"Вверх",                                        // 11
	L"Влево",                                        // 12
	L"Вправо",                                       // 13
	L"Вниз",                                         // 14
	L"Сколько направлений",                          // 15
	L"Сколько времени считать клавишу нажатой",      // 16
	L"Быстрое движение - это больше",                // 17
	L"После удержания мыши в левом нижнем углу экрана в течение", // 18
	L"секунд,",                                      // 19
	L"ЛЕВАЯ работает как 1:",                        // 20
	L"Зона влево-вправо",                            // 21
	L"Зона вверх-вниз",                              // 22
	L"Оставить режим 3 на одной из осей",            // 23
	L"Крутить колесо - чувствительность",            // 24
	L"2:",                                           // 25
	// Radio buttons - modes
	L"Режим 1: Движения мыши вызывают нажатия клавиш, нажатие ПКМ отпускает все клавиши", // 26
	L"Режим 2: правая кнопка мыши = нажатие клавиш", // 27
	L"Режим 3: клавиша нажата, пока мышь движется",  // 28
	L"Режим 4 (автогонки):",                         // 29
	L"Режим 5 (8 умений)",                           // 30
	L"Режим 6: Колесико мыши = ЛКМ+ПКМ",            // 31
	L"Режим 7: только окна",                         // 32
	// Checkboxes
	L"Быстрое движение мышью нажимает клавишу",     // 33
	L"из конца в конец по оси горизонтально в два движения", // 34
	L"из конца в конец в два движения (для аркад)",  // 35
	L"можно менять направление (горизонтально/вертикально) на ходу", // 36
	L"ПРАВАЯ нажимает 1:",                           // 37
	L"автоклик левой при отпускании правой",         // 38
	L"ЛКМ Кликер Угол",                              // 39
	L"AHK ЛКМ кликер",                               // 40
	L"3 сек ЛКМ=Win",                                // 41
	L"3 сек ЛКМ=Esc",                                // 42
	L"AHK Колесико",                                 // 43
	L"Видимый курсор",                               // 44
	L"Крутить колесо - чувствительность",            // 45
	L"Двойной щелчок правой = пауза",               // 46
	L"и при отпускании",                             // 47
	L"и при отпускании",                             // 48
	L"вбок и вниз = просто вниз",                    // 49
	L"игнорировать быстрое движение мыши",           // 50
	L"вбок и вниз - сразу отпускать",                // 51
	L"Движения мыши выбирают клавишу для нажатия ПКМ.", // 52
	// Groupboxes
	L"Кнопки мыши",                                  // 53
	L"Удержание правой кнопки мыши и движения жмут кнопки", // 54
	// Dialog 2
	L"Надпись в окне",                               // 55
	L"Сенсор",                                       // 56
	L"Цвет",                                         // 57
	L"Координаты",                                   // 58
	L"Размер",                                       // 59
	L"Клавиша",                                      // 60
	L"Режим",                                        // 61
	L"Айтрекер:",                                    // 62
	L"ОК",                                           // 63
	L"Запустить",                                    // 64
	L"Остановить",                                   // 65
	L"Обозначать кружком направление взгляда",       // 66
	// Combos
	L"мышь",                                         // 67
	L"айтрекер",                                     // 68
	L"зелёный",                                      // 69
	L"жёлтый",                                       // 70
	L"красный",                                      // 71
	L"синий",                                        // 72
	L"кнопка",                                       // 73
	L"переключатель",                                // 74
	L"без группы",                                   // 75
	// Autoclick speeds
	L"20 мс",                                        // 76
	L"50 мс",                                        // 77
	L"100 мс",                                       // 78
	L"500 мс",                                       // 79
	// Timeouts
	L"50 мс",                                        // 80
	L"75 мс",                                        // 81
	L"100 мс",                                       // 82
	L"125 мс",                                       // 83
	L"150 мс",                                       // 84
	L"200 мс",                                       // 85
	L"250 мс",                                       // 86
	L"0,5 секунды",                                  // 87
	L"1 секунда",                                    // 88
	// Mode3
	L"не надо",                                      // 89
	L"на оси вправо-влево",                          // 90
	L"на оси вверх-вниз",                            // 91
	// Circle scales
	L"не использовать",                              // 92
	L"50 пикселов",                                  // 93
	L"100 пикселов",                                 // 94
	// Scancodes (translatable)
	L"<ничего>",                                     // 95
	L"вверх",                                        // 96
	L"вправо",                                       // 97
	L"вниз",                                         // 98
	L"влево",                                        // 99
	L"пробел",                                       // 100
	L"Левый Shift",                                  // 101
	L"Левый Ctrl",                                   // 102
	L"Левый Alt",                                    // 103
	L"Левый Win",                                    // 104
	L"Правый Shift",                                 // 105
	L"Правый Ctrl",                                  // 106
	L"Правый Alt",                                   // 107
	L"Правый WIN",                                   // 108
	L"(F8 - запрещена) ",                            // 109
	L"(Delete - запрещена)",                          // 110
	L"ЛКМ",                                          // 111
	L"ПКМ",                                          // 112
	L"ЛКМ+F12",                                      // 113
	L"Мышь влево",                                   // 114
	L"Мышь вправо",                                  // 115
	L"Скролл туда",                                  // 116
	L"Скролл сюда",                                  // 117
	L"Средняя кнопка",                               // 118
	L"Колёсико вверх",                               // 119
	L"Колёсико вниз",                                // 120
	L"(Num . - запрещена)",                          // 121
	// Errors
	L"Не найдено",                                   // 122
	L"Не найдено: %s",                               // 123
	L"Не могу открыть файл: '",                      // 124
	L"Не могу создать файл: '",                      // 125
	L"Сообщение об ошибке не найдено",               // 126
	L"Нет системной ошибки",                         // 127
	L"MH: сообщение об ошибке",                      // 128
	L"Непорядок!",                                   // 129
	L"неизвестно",                                   // 130
	L"Ошибка Tobii Gaze SDK: %d (%s)",              // 131
	L"MHook:Gaze SDK: сообщение об ошибке",          // 132
	// File dialog
	L"файлы MHOOK\0*.MHOOK\0\0",                    // 133
	L"Открыть файл конфигурации MHOOK",              // 134
	L"Сохранить файл конфигурации MHOOK",            // 135
	L"MHOOK",                                        // 136
	// Window titles
	L"Из мыши в клавиатуру: настройка",              // 137
	L"Из мыши в клавиатуру: дополнительная настройка",// 138
	L"Нажмите на окно игры...",                      // 139
	// Lang buttons
	L"RUS",                                          // 140
	L"ENG",                                          // 141
};
static const TCHAR* strings_EN[LOC_STRING_COUNT] = {
	L"MHook V2",                                     // 0 LOC_APP_TITLE
	L"Mouse to Keyboard: settings",                  // 1 LOC_DIALOG1_TITLE
	L"Mouse to Keyboard: additional settings",       // 2 LOC_DIALOG2_TITLE
	L"START",                                        // 3 LOC_BTN_START
	L"EXIT",                                         // 4 LOC_BTN_EXIT
	L"Save...",                                      // 5 LOC_BTN_SAVE
	L"Load...",                                      // 6 LOC_BTN_LOAD
	L"WASD",                                         // 7
	L"Key-windows...",                               // 8 LOC_BTN_MAGIC_WINDOWS
	L"By window",                                    // 9 LOC_BTN_LOAD_BY_WINDOW
	L"Sensitivity in pixels",                        // 10
	L"Up",                                           // 11
	L"Left",                                         // 12
	L"Right",                                        // 13
	L"Down",                                         // 14
	L"Number of directions",                         // 15
	L"How long to hold key pressed",                 // 16
	L"Fast movement is more than",                   // 17
	L"After holding mouse in the lower-left corner for", // 18
	L"seconds,",                                     // 19
	L"LEFT works as 1:",                             // 20
	L"Dead zone left-right",                         // 21
	L"Dead zone up-down",                            // 22
	L"Keep mode 3 on one axis",                      // 23
	L"Scroll wheel - sensitivity",                   // 24
	L"2:",                                           // 25
	// Radio buttons
	L"Mode 1: Mouse movements press keys, RMB releases all", // 26
	L"Mode 2: right mouse button = key press",       // 27
	L"Mode 3: key held while mouse moves",           // 28
	L"Mode 4 (racing):",                             // 29
	L"Mode 5 (8 skills)",                            // 30
	L"Mode 6: Mouse wheel = LMB+RMB",                // 31
	L"Mode 7: windows only",                         // 32
	// Checkboxes
	L"Fast mouse movement presses key",              // 33
	L"end to end horizontally in two movements",     // 34
	L"end to end in two movements (for arcades)",    // 35
	L"can change direction on the go",               // 36
	L"RIGHT presses 1:",                             // 37
	L"autoclick left on right release",              // 38
	L"LMB Clicker Corner",                           // 39
	L"AHK LMB clicker",                              // 40
	L"3 sec LMB=Win",                                // 41
	L"3 sec LMB=Esc",                                // 42
	L"AHK Wheel",                                    // 43
	L"Visible cursor",                               // 44
	L"Scroll wheel - sensitivity",                   // 45
	L"Double right click = pause",                   // 46
	L"and on release",                               // 47
	L"and on release",                               // 48
	L"side and down = just down",                    // 49
	L"ignore fast mouse movement",                   // 50
	L"side and down - release immediately",          // 51
	L"Mouse movements choose key for RMB press.",    // 52
	// Groupboxes
	L"Mouse buttons",                                // 53
	L"Holding right mouse button and movements press keys", // 54
	// Dialog 2
	L"Window text",                                  // 55
	L"Sensor",                                       // 56
	L"Color",                                        // 57
	L"Coordinates",                                  // 58
	L"Size",                                         // 59
	L"Key",                                          // 60
	L"Mode",                                         // 61
	L"Eyetracker:",                                  // 62
	L"OK",                                           // 63
	L"Start",                                        // 64
	L"Stop",                                         // 65
	L"Mark gaze direction with circle",              // 66
	// Combos
	L"mouse",                                        // 67
	L"eyetracker",                                   // 68
	L"green",                                        // 69
	L"yellow",                                       // 70
	L"red",                                          // 71
	L"blue",                                         // 72
	L"button",                                       // 73
	L"switch",                                       // 74
	L"no group",                                     // 75
	// Autoclick speeds
	L"20 ms",                                        // 76
	L"50 ms",                                        // 77
	L"100 ms",                                       // 78
	L"500 ms",                                       // 79
	// Timeouts
	L"50 ms",                                        // 80
	L"75 ms",                                        // 81
	L"100 ms",                                       // 82
	L"125 ms",                                       // 83
	L"150 ms",                                       // 84
	L"200 ms",                                       // 85
	L"250 ms",                                       // 86
	L"0.5 seconds",                                  // 87
	L"1 second",                                     // 88
	// Mode3
	L"none",                                         // 89
	L"on left-right axis",                           // 90
	L"on up-down axis",                              // 91
	// Circle scales
	L"don't use",                                    // 92
	L"50 pixels",                                    // 93
	L"100 pixels",                                   // 94
	// Scancodes
	L"<nothing>",                                    // 95
	L"up",                                           // 96
	L"right",                                        // 97
	L"down",                                         // 98
	L"left",                                         // 99
	L"space",                                        // 100
	L"Left Shift",                                   // 101
	L"Left Ctrl",                                    // 102
	L"Left Alt",                                     // 103
	L"Left Win",                                     // 104
	L"Right Shift",                                  // 105
	L"Right Ctrl",                                   // 106
	L"Right Alt",                                    // 107
	L"Right WIN",                                    // 108
	L"(F8 - forbidden) ",                            // 109
	L"(Delete - forbidden)",                         // 110
	L"LMB",                                          // 111
	L"RMB",                                          // 112
	L"LMB+F12",                                      // 113
	L"Mouse left",                                   // 114
	L"Mouse right",                                  // 115
	L"Scroll there",                                 // 116
	L"Scroll here",                                  // 117
	L"Middle button",                                // 118
	L"Wheel up",                                     // 119
	L"Wheel down",                                   // 120
	L"(Num . - forbidden)",                          // 121
	// Errors
	L"Not found",                                    // 122
	L"Not found: %s",                                // 123
	L"Cannot open file: '",                          // 124
	L"Cannot create file: '",                        // 125
	L"Error message not found",                      // 126
	L"No system error",                              // 127
	L"MH: error message",                            // 128
	L"Error!",                                       // 129
	L"unknown",                                      // 130
	L"Tobii Gaze SDK error: %d (%s)",                // 131
	L"MHook:Gaze SDK: error message",                // 132
	// File dialog
	L"MHOOK files\0*.MHOOK\0\0",                     // 133
	L"Open MHOOK configuration file",                // 134
	L"Save MHOOK configuration file",                // 135
	L"MHOOK",                                        // 136
	// Window titles
	L"Mouse to Keyboard: settings",                  // 137
	L"Mouse to Keyboard: additional settings",       // 138
	L"Click on the game window...",                  // 139
	// Lang buttons
	L"RUS",                                          // 140
	L"ENG",                                          // 141
};
const TCHAR* L(int id) {
	if (id < 0 || id >= LOC_STRING_COUNT) return L"";
	if (MHLanguage == MH_LANG_EN) return strings_EN[id];
	return strings_RU[id];
}
void Localization_Init() {
	// Language is loaded from config, nothing to init here
}