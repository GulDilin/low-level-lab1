# low-level-lab1

Project for STM32 ARM education stand. It is example of garland lights modes 

## Russian

### Task
Разработать и реализовать драйверы управления светодиодными индикаторами
и обработки нажатий кнопки стенда SDK-1.1М (индикаторы и кнопка расположены на боковой панели стенда).
Написать программу с использованием разработанных драйверов в соответствии с вариантом задания.

### Variant

Реализовать простой имитатор гирлянды с переключением режимов.
Должно быть реализовано не менее четырех последовательностей переключения светодиодов, обязательно с разной частотой мигания.
По нажатию кнопки происходит переключение на следующий режим. Если режим последний в списке, нажатие кнопки должно переключать на первый режим.
При повторном выборе режима анимация на светодиодах должна запускаться с того места, на котором была прервана переключением на следующий режим.

### Program description

Для решения задачи создания имитации гирлянды, которая при повторном выборе режима анимации должна восстанавливаться с того же момента,
на котором остановилась мы применили несколько структур. 

Во первых мы выделили структуру состояния режима – это конкретных набор параметров включения светодиодов и время, на которое этот режим должен быть включен.
 
Во вторых была выделена структура режим – это указатель на массив из состояний и его длина
 
В процессе работы одного режима состояния переключаются по кругу. В процессе работы программы в бесконечном цикле опрашивается кнопка. В каждой итерации цикла сохраняется состояние текущего режима в массиве сохраненных состояний режима. При нажатии кнопки проиходит переключение на следующий режим по кругу (после последнего идет самый первый). В процессе этого переключения восстанавливается время старта режима для правильного переключения его в следующее состояние. Далее применяется состояние активного режима (активация светодиодов). 

