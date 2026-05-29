# LMS 项目 Makefile
# 用法（在 backup/ 目录下执行）：
#   mingw32-make             编译
#   mingw32-make run          编译并运行
#   mingw32-make clean        清理
#   mingw32-make list         显示参与编译的文件

CXX       = C:/enviroment/mingw64/bin/g++.exe
EASYX_INC = C:/PROGRA~2/Dev-Cpp/MinGW64/include
EASYX_LIB = C:/PROGRA~2/Dev-Cpp/MinGW64/lib
TARGET    = output/LMS.exe

CXXFLAGS = -finput-charset=UTF-8 -fexec-charset=GBK -O2 -Wall -pipe
INCS     = -I. -Ipublic -Iservice -Iview -Iapp -I$(EASYX_INC)
LIBS     = -L$(EASYX_LIB) -leasyx -static-libgcc -static-libstdc++

SRCS = main.cpp            \
       public/compat_iob.c   \
       public/common.cpp     \
       service/user_service.cpp      \
       service/order_service.cpp     \
       service/warehouse_service.cpp \
       service/transport_service.cpp \
       view/control.cpp      \
       app/loginWin.cpp      \
       app/startWin.cpp      \
       app/orderWin.cpp      \
       app/mainWin.cpp       \
       app/transportWin.cpp  \
       app/warehouseWin.cpp

.PHONY: all run clean list

all: $(TARGET)

$(TARGET): $(SRCS)
	@if not exist output mkdir output
	$(CXX) $(CXXFLAGS) $(INCS) $(SRCS) -o $@ $(LIBS)
	@echo [OK] $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	@if exist output\LMS.exe del /q output\LMS.exe 2>nul
	@echo [CLEAN] output

list:
	@echo === 参与编译的源文件 ===
	@echo $(SRCS)
