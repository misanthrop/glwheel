CXXFLAGS += -std=c++11

os := $(shell uname -s)

ifneq (,$(findstring MINGW,$(os)))
	os := Windows
endif

ifeq ($(os),Linux)
	libs	   += GL X11
else ifeq ($(os),Windows)
	target	 := $(target).exe
	libs	 += glew32 gdi32 opengl32 ws2_32
	CXXFLAGS += -D_WIN32_WINNT=0x0500 -DUNICODE -DGLEW_STATIC
	LDFLAGS  += -static -Xlinker -subsystem=windows
endif

ifeq ($(CDEBUG),-g)
	CXXFLAGS := $(filter-out -O1 -O2 -O3 -Os -s, $(CXXFLAGS))
	LDFLAGS  := $(filter-out -O1 -O2 -O3 -Os -s, $(LDFLAGS))
endif

objs	:= $(addsuffix .o,$(sources))
depends	:= $(addsuffix .dp,$(sources))

compile = $(CXX) $(CDEBUG) $(CPPFLAGS) $(CXXFLAGS)
link	= $(CXX) $(CDEBUG) $(LDFLAGS) $(objs) $(addprefix -l,$(libs))

.PHONY: all clean
.PRECIOUS: %.dp
.SUFFIXES:

all:: $(target)

clean::
	$(RM) $(objs) $(depends) $(target)

%.o: %.cpp

%.o: %.cpp %.dp
	$(compile) -c  $< -o $@

%.dp: %.cpp
	@$(compile) -MM $< -MG -MP -MT $*.o -MF $@

-include $(depends)

$(target): $(objs)
	$(link) -o $(target)
