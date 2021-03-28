#!/usr/bin/env bash

executable_path_with_cache_grid="valgrind --tool=cachegrind $1"
${executable_path_with_cache_grid}
