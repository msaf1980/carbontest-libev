#!/bin/sh

conan remote add conan-center "https://conan.bintray.com"

conan_create_local() {
[ "$1" == "" -o "$2" == "" ] && return 1
local repo_url="$1"
local repo_dir="$2"
local repo_branch="$3"

if [ "${repo_branch}" == "" ]; then
    git clone "${repo_url}" || return 1
else
    git clone --single-branch --branch "${repo_branch}" "${repo_url}" || return 1
fi
cd "${repo_dir}" && conan export . local/stable || return 1
cd .. && rm -rf "${repo_dir}"
return 0
}

conan_create_local https://github.com/darcamo/conan-cxxopts conan-cxxopts

#conan_create_local https://github.com/msaf1980/conan-libevfibers conan-libevfibers 0.4.1-60-gd9ad92c

#conan_create_local https://github.com/msaf1980/conan-libev conan-libev 4.43

#git clone https://github.com/msaf1980/conan-plog || exit 1
#cd conan-plog && conan create . local/stable || exit 1
#cd .. && rm -rf conan-plog

#cd contrib && {
#  git submodule add https://github.com/cameron314/concurrentqueue
#  git submodule add https://github.com/SergiusTheBest/plog
#  git submodule add https://github.com/msaf1980/c_procs
#  git submodule add https://github.com/msaf1980/cpp_procs
#}

#git submodule update --init

