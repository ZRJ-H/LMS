#!/bin/bash
# LMS 项目构建脚本 — 自动发现源文件，一键编译
# 用法（在 backup/ 目录下执行）：
#   bash build.sh           编译
#   bash build.sh run       编译并运行
#   bash build.sh clean     清理
#   bash build.sh list      显示参与编译的文件

set -e
SCRIPT="$(realpath "$0")"
cd "$(dirname "$SCRIPT")"

CXX="/c/enviroment/mingw64/bin/g++.exe"
EASYX_INC="/c/PROGRA~2/Dev-Cpp/MinGW64/include"
EASYX_LIB="/c/PROGRA~2/Dev-Cpp/MinGW64/lib"
TARGET="output/LMS.exe"

# 收集源文件（每个子目录一行，新增目录在这里加）
SRCS=""
for dir in . public service view app; do
    for f in "$dir"/*.cpp "$dir"/*.c; do
        [ -f "$f" ] || continue
        [[ "$(basename "$f")" == test_* ]] && continue
        SRCS="$SRCS $f"
    done
done

CXXFLAGS="-finput-charset=UTF-8 -fexec-charset=GBK -O2 -Wall"
INCS="-I. -Ipublic -Iservice -Iview -Iapp -I$EASYX_INC"
LIBS="-L$EASYX_LIB -leasyx -lmsvcrt -static-libgcc -static-libstdc++"

case "${1:-build}" in
clean)
    rm -rf output/*.exe output/*.o output/**/*.o 2>/dev/null
    rm -rf output/output 2>/dev/null  # 清理 mkdir -p 创建的嵌套目录
    echo "[CLEAN] done"
    ;;
list)
    echo "=== 参与编译的源文件 ==="
    for f in $SRCS; do echo "  $f"; done
    echo "=== 目标: $TARGET ==="
    ;;
run)
    bash "$SCRIPT" build
    ./"$TARGET"
    ;;
build|*)
    mkdir -p output
    # 复制运行时需要的资源文件到 output/（只复制不覆盖已有）
    [ -d "../image" ] && cp -r ../image output/ 2>/dev/null || true
    echo "[BUILD] compiling..."
    $CXX $CXXFLAGS $INCS $SRCS -o "$TARGET" $LIBS
    echo "[OK] $TARGET"
    ;;
esac
