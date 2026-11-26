#
# Half-life Makefile for x86 Linux
#
#

OS:=$(shell uname)
HOSTNAME:=$(shell hostname)

ifeq "$(CFG)" ""
	CFG=release
endif

SOURCE_DIR=..
BUILD_DIR=$(CFG)

ENGINE_SRC_DIR=$(SOURCE_DIR)/engine
COMMON_SRC_DIR=$(SOURCE_DIR)/common
PM_SRC_DIR=$(SOURCE_DIR)/pm_shared
GAME_SHARED_SRC_DIR=$(SOURCE_DIR)/game_shared
PUBLIC_SRC_DIR=$(SOURCE_DIR)/public

BUILD_OBJ_DIR=$(BUILD_DIR)/obj

INSTALLER_PAYLOAD_DIR=$(SOURCE_DIR)/installer/payload

ARCH=i386
ELF-GC-DYNSTR=./elf-gc-dynstr

ifeq "$(COMPILER)" ""
	COMPILER=g++
endif

CPLUS="$(COMPILER) -m32"
# BSVR start
# libstdc++ linked statically with any compiler: the explicit -lstdc++ made it a dynamic dependency on the build machine's
# version, and the -L dir was the default g++'s, not $(COMPILER)'s. Released builds come from build-steamrt.sh.
# The static libstdc++.a is also named here, ahead of the shared libraries on client.so's link line: libopenvr_api.so
# exports the C++ runtime (__cxa_*), and otherwise the linker takes it from there, leaving those symbols to be bound
# at run time to the system's libstdc++, which doesn't match this copy (crash in std::locale on load).
# --exclude-libs,ALL keeps the static libraries' symbols private. Exported, the GNU unique ones (the locale facet ids)
# are merged with the same symbols of the system's libstdc++, which the engine has already loaded, and the locale
# setup of this copy corrupts memory (free(): invalid size on load). -Bsymbolic doesn't apply to unique symbols.
# CPP_LIB:=-L$(shell g++ -m32 -print-file-name=libstdc++.so | xargs dirname) -lstdc++ -ldl -lpthread -static-libstdc++ -static-libgcc - original
CPP_LIB:=-Wl,--exclude-libs,ALL -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic -ldl -lpthread -static-libstdc++ -static-libgcc
# BSVR end

ARCH_CFLAGS=-march=pentium-m -mfpmath=387 -mno-sse

ifeq "$(CFG)" "release"
	ARCH_CFLAGS+=-O3 
else
	ARCH_CFLAGS+=-gdwarf-2 -g2 
endif

BASE_CFLAGS=-std=c++17 -fpermissive -fno-strict-aliasing -fno-exceptions -fexpensive-optimizations -Werror=return-type -w
BASE_CFLAGS+=-DNDEBUG -DPOSIX -D_POSIX -DLINUX -D_LINUX -DGNUC -DNO_MALLOC_OVERRIDE -DCLIENT_WEAPONS

# flifetime-dse=1 is needed to disable a compiler optimization that optimizes out std::memset calls in CBaseEntity::operator new
# See https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-flifetime-dse for more information about this flag
# fno-gnu-unique is needed to disable a compiler optimization that prevents dlclose from unloading mod dlls,
# causing them to retain state and crash when the engine tries to access memory in an invalid way
# See https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html#index-fno-gnu-unique for more information about this flag
# BSVR start
# Also for versioned names like g++-9, which would otherwise build without these flags
# ifeq "$(COMPILER)" "g++" - original
ifneq "$(filter g++%,$(COMPILER))" ""
# BSVR end
	BASE_CFLAGS+=-flifetime-dse=1 -fno-gnu-unique
endif

SHLIBEXT=so
SHLIBCFLAGS=
ifeq "$(CFG)" "release"
	SHLIBLDFLAGS="-shared -Wl,-Map,$@_map.txt -Wl,--no-undefined"
else
	SHLIBLDFLAGS="-shared -gdwarf-2 -g2 -Wl,-Map,$@_map.txt -Wl,--no-undefined"
endif

AR=ar
LIBEXT=a
MAKE+= -j8

MAKE_HL_LIB=$(MAKE) -f Makefile.hldll
MAKE_HL_CDLL=$(MAKE) -f Makefile.hl_cdll
MAKE_RP3D=$(MAKE) -f Makefile.rp3d

RP3D_COMMON_VARS=CPLUS=$(CPLUS) AR=$(AR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR)

#############################################################################
# SETUP AND BUILD
#############################################################################

all: build_dir targets

TARGETS=

TARGETS+= \
 	hl \
	hl_cdll \
	installer_payload \

build_dir:
	-mkdir -p $(BUILD_DIR);
	cd $(BUILD_DIR)

targets: $(TARGETS)

hl_cdll: build_dir
	$(MAKE_HL_CDLL) CPLUS=$(CPLUS) ARCH=$(ARCH) ARCH_CFLAGS="$(ARCH_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CPP_LIB="$(CPP_LIB)" CFG=$(CFG) OS=$(OS) BASE_CFLAGS="$(BASE_CFLAGS)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) PM_SRC_DIR=$(PM_SRC_DIR)

# Copy the built libraries where the installer embeds them (installer/payload/dlls and cl_dlls)
installer_payload: hl hl_cdll
	mkdir -p $(INSTALLER_PAYLOAD_DIR)/dlls $(INSTALLER_PAYLOAD_DIR)/cl_dlls
	cp -f $(BUILD_DIR)/hl.$(SHLIBEXT) $(INSTALLER_PAYLOAD_DIR)/dlls/
	cp -f $(BUILD_DIR)/client.$(SHLIBEXT) $(INSTALLER_PAYLOAD_DIR)/cl_dlls/

# ReactPhysics3D is prebuilt and tracked (external/reactphysics3d/lib/Linux/libreactphysics3d.a). Rebuild it only when
# its sources change, with build-steamrt.sh rp3d, and commit the result.
rp3d: build_dir
	$(MAKE_RP3D) $(RP3D_COMMON_VARS)

clean_rp3d:
	$(MAKE_RP3D) $(RP3D_COMMON_VARS) clean

hl: build_dir
	$(MAKE_HL_LIB) CPLUS=$(CPLUS) ARCH=$(ARCH) ARCH_CFLAGS="$(ARCH_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CPP_LIB="$(CPP_LIB)" CFG=$(CFG) OS=$(OS) BASE_CFLAGS="$(BASE_CFLAGS)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) PM_SRC_DIR=$(PM_SRC_DIR)

clean:
	-rm -rf $(BUILD_OBJ_DIR)
