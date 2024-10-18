#!/bin/bash
find ~ -type f -name "*.sh.e*" -delete
find ~ -type f -name "*.sh.o*" -delete
#find ~ -type f ! -name "*.*" -delete
find ~ -type f ! -name "*.*" -not \( -path '*/.*' \) -delete

