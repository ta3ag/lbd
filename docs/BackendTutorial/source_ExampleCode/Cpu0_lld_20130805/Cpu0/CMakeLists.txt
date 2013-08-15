add_lld_library(lldCpu0ELFTarget
  Cpu0TargetInfo.cpp
  Cpu0TargetHandler.cpp
  Cpu0RelocationHandler.cpp
  )

target_link_libraries(lldCpu0ELFTarget
  lldCore
  )
