# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/vinicius/bikeshare-workspace/modules/tee/tf-m/trusted-firmware-m"
  "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/tfm"
  "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/modules/trusted-firmware-m/tfm-prefix"
  "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/modules/trusted-firmware-m/tfm-prefix/tmp"
  "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/modules/trusted-firmware-m/tfm-prefix/src/tfm-stamp"
  "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/modules/trusted-firmware-m/tfm-prefix/src"
  "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/modules/trusted-firmware-m/tfm-prefix/src/tfm-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/modules/trusted-firmware-m/tfm-prefix/src/tfm-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/vinicius/bikeshare-workspace/bikeshare-firmware/build/modules/trusted-firmware-m/tfm-prefix/src/tfm-stamp${cfgdir}") # cfgdir has leading slash
endif()
