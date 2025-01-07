#!/bin/bash -e

echo "Building Host..."

clang++ -s -o test-host main.cpp \
-I/home/adi/dotnet/packs/Microsoft.NETCore.App.Host.linux-x64/9.0.0/runtimes/linux-x64/native \
-L/home/adi/dotnet/packs/Microsoft.NETCore.App.Host.linux-x64/9.0.0/runtimes/linux-x64/native \
-ldl -lm -pthread \
-Wl,-rpath,/home/adi/dotnet/packs/Microsoft.NETCore.App.Host.linux-x64/9.0.0/runtimes/linux-x64/native \
-lnethost

echo "Building .NET Library..."

dotnet build -c Release -o build --no-restore Gtk4

