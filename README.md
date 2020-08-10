# image-generator-server-client
![Alt-текст](https://github.com/ueberaccelerate/image-generator-server-client/blob/master/doc/image-generator.png "server")
![Alt-текст](https://github.com/ueberaccelerate/image-generator-server-client/blob/master/doc/qt-reciever.png "client")
## Description
Небольшого программного комплекса, состоящего из нескольких,
взаимодействующих друг с другом приложений. 
Одно из приложений генерирует информацию и выдает её с использованием TCP/IP стека. 
Клиентское приложение принимает информацию, отображает её на экране вместе со статистикой, параллельно записывая на диск. 
## Building
Build by making a build directory (i.e. build/), run cmake in that dir, and then use make to build the desired target.

Example:
```bash
> mkdir build && cd build
> cmake .. -DCMAKE_BUILD_TYPE=[Debug | Release]
> cmake --build .
> ./src/image-generator/image-generator [--config path_to_config]
> ./src/qt-viewreciever/qt-viewreciever
```
## Requirement
- boost v1.73.0
- qt5
