
SOURCES = main.cpp
OBJECTS = main.o 

TARGET = test-host
DOTNET_LIBDIR = $(DOTNET_ROOT)/packs/Microsoft.NETCore.App.Host.linux-x64/9.0.0/runtimes/linux-x64/native
DOTNET_INCLUDES = $(DOTNET_LIBDIR)

DOTNET_PROJECT = Gtk4/test1.csproj
DOTNET_BUILDDIR = build
DOTNET_DLLNAME = test1.dll
DOTNET_TARGET = $(DOTNET_BUILDDIR)/$(DOTNET_DLLNAME)

CXXFLAGS = -O2 -I$(DOTNET_INCLUDES)
LDFLAGS = -L$(DOTNET_LIBDIR) -Wl,-rpath,$(DOTNET_LIBDIR)
LIBS = -pthread -ldl -lm -lnethost

DOTNET = dotnet

all: $(TARGET) $(DOTNET_BUILDDIR)

$(TARGET): $(OBJECTS)
	@echo "[CXXLD]" $@
	@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)
	@strip $(TARGET)

%.o: %.cpp
	@echo "[CXX]" $@ 
	@$(CXX) -o $@ -c $< $(CXXFLAGS)

$(DOTNET_TARGET): $(DOTNET_PROJECT)
	@echo "[DOTNET]" $<
	$(MAKE) dotnet

$(DOTNET_BUILDDIR): $(DOTNET_PROJECT)
	@echo "[DOTNET]" $<
	$(MAKE) dotnet

dotnet: $(DOTNET_PROJECT)
	@$(DOTNET) build -c Release -o $(DOTNET_BUILDDIR) $(DOTNET_PROJECT)

clean:
	@rm -rf $(TARGET) $(OBJECTS) $(DOTNET_BUILDDIR)


