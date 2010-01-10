
# Create the extension library
ADD_LIBRARY(Extensions_LevelSet
  LevelSet/SDF.cpp
  LevelSet/SDF.h
  LevelSet/CPUStrategy.cpp
  LevelSet/CPUStrategy.h
)
TARGET_LINK_LIBRARIES(Extensions_LevelSet
  Extensions_TexUtils
)