# LMS 项目 Makefile — 自动发现源文件，一条 make 即编译
# 用法（在 backup/ 目录下执行）：
#   make             编译
#   make run          编译并运行
#   make clean        清理
#   make list         显示参与编译的文件

CXX       = C:/enviroment/mingw64/bin/g++.exe
EASYX_INC = C:/PROGRA~2/Dev-Cpp/MinGW64/include
EASYX_LIB = C:/PROGRA~2/Dev-Cpp/MinGW64/lib
TARGET    = output/LMS.exe

# 显式列出每个子目录的源文件（新增子目录时在这里加一行即可）
SRCS = $(wildcard *.cpp)         \
       $(wildcard public/*.c)     \
       $(wildcard public/*.cpp)   \
       $(wildcard service/*.cpp)  \
       $(wildcard view/*.cpp)     \
       $(wildcard app/*.cpp)
# 排除测试文件
SRCS := $(filter-out test_%, $(SRCS))

CXXFLAGS = -finput-charset=UTF-8 -fexec-charset=GBK -O2 -Wall -pipe
INCS     = -I. -Ipublic -Iservice -Iview -Iapp -I$(EASYX_INC)
LIBS     = -L$(EASYX_LIB) -leasyx -static-libgcc -static-libstdc++

.PHONY: all run clean list

all: $(TARGET)

# 分两步编译：先 -c 为 .o，再链接
# 链接阶段需要 Windows 可用临时目录 → 设置为当前 output 目录
$(TARGET): $(SRCS)
	@mkdir -p output
	@for f in $^; do \
		bn=$$(basename "$$f"); \
		obj="output/$${bn%.*}.o"; \
		$(CXX) $(CXXFLAGS) $(INCS) -c "$$f" -o "$$obj" || exit 1; \
	done
	cd output && TMP=$$(pwd) TEMP=$$(pwd) $(CXX) $(CXXFLAGS) *.o -o $$(basename $@) $(LIBS)
	@echo [OK] $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf output

list:
	@echo "=== 参与编译的源文件 ($(words $(SRCS)) 个) ==="
	@$(foreach f,$(SRCS),echo "  $(f)";)
	@echo "=== 目标: $(TARGET) ==="
