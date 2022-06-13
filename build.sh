clang --target=wasm32 *.c -c -O3 -mbulk-memory -flto -fvisibility=default
wasm-ld *.o -o pbkdf2.wasm --no-entry --export=pbkdf2 --export=__heap_base --strip-all
rm *o
