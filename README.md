# pbkdf2-hmac-sha512-c-webassembly
PBKDF2-HMAC-SHA-512 written in C for WebAssembly compilation. It generates a fixed size, 512-bit key.

You will need `clang` and `lld`, then just run:
```
bash build.sh
```
This command will generate a file called `pbkdf2.wasm`. You can upload it to your server and use it on the browser with code as follows:
```js
function computeKey(email, password) {
  const t0 = performance.now();
  return new Promise(resolve => {
    setTimeout(() => {
      let encoder = new TextEncoder();
      let encodedPassword = encoder.encode(password);
      let salt = encoder.encode(email + '|your-domain-here.com');

      let passwordPtr = basePtr;
      let saltPtr = passwordPtr + encodedPassword.length;
      // +4 to account for the pbkdf2 counter appended to the end of the salt
      let outputPtr = saltPtr + salt.length + 4;

      HEAPU8.set(encodedPassword, passwordPtr);
      HEAPU8.set(salt, saltPtr);

      pbkdf2(
        passwordPtr,            // *password
        encodedPassword.length, // password_len
        saltPtr,                // *salt + 4 empty bytes at the end
        salt.length,            // salt_len
        100000,                 // c
        outputPtr               // *output
      );

      // copy the data from the memory since it will be cleaned up next
      const dk = new Uint8Array(HEAPU8.subarray(outputPtr, outputPtr + 64));

      // clean up memory
      for (let i = passwordPtr; i < outputPtr + 64; ++i) {
        HEAPU8[basePtr + i] = 0x00;
      }
      pbkdf2(basePtr, 0, 1, basePtr);

      console.log(performance.now() - t0);

      resolve(dk);
    }, 0);
  });
}

WebAssembly.instantiate(PUT_THE_WASM_SRC_HERE).then(obj => {
  const { exports } = obj.instance;
  window.pbkdf2 = exports.pbkdf2;
  window.basePtr = exports.__heap_base;
  window.HEAPU8 = new Uint8Array(exports.memory.buffer);
});
```

**Full example:**
```js
// the following is the file pbkdf2.wasm encoded as base 64
// it takes more space to store, but avoids making another request to the server
// which may improve UX depending on the latency
// alternatively you can download the file using:
//    WebAssembly.instantiateStreaming(source, importObject)
const b64Wasm =
  'AGFzbQEAAAABHARgBH9/f38AYAN/f38AYAJ/fwBgBn9/f39/fwADBwYAAQIDAgEFAwEAAgYPAn8B'
  + 'QYCNBAt/AEGAjQQLByoEBm1lbW9yeQIABnNoYTUxMgAFBnBia2RmMgADC19faGVhcF9iYXNlAw'
  + 'EKhjIGpQQBAX8jgICAgABB4ANrIgQkgICAgAAgBEGQAmpBCGogAEEIaikDADcDACAEQZACakEQ'
  + 'aiAAQRBqKQMANwMAIARBkAJqQRhqIABBGGopAwA3AwAgBEGQAmpBIGogAEEgaikDADcDACAEQZ'
  + 'ACakEoaiAAQShqKQMANwMAIARBkAJqQTBqIABBMGopAwA3AwAgBEGQAmpBOGogAEE4aikDADcD'
  + 'ACAEQcAAakEIaiAAQdgBaikDADcDACAEQcAAakEQaiAAQeABaikDADcDACAEQcAAakEYaiAAQe'
  + 'gBaikDADcDACAEQcAAakEgaiAAQfABaikDADcDACAEQcAAakEoaiAAQfgBaikDADcDACAEQcAA'
  + 'akEwaiAAQYACaikDADcDACAEQcAAakE4aiAAQYgCaikDADcDACAEQQA2AtgDIARCgAE3A9ADIA'
  + 'QgACkDADcDkAIgBCAAKQPQATcDQCAEQZACaiABIAIQgYCAgAAgBEGQAmogBBCCgICAACAEQYgB'
  + 'aiAEKQMINwMAIARBkAFqIARBEGopAwA3AwAgBEGYAWogBEEYaikDADcDACAEQaABaiAEQSBqKQ'
  + 'MANwMAIARBqAFqIARBKGopAwA3AwAgBEGwAWogBEEwaikDADcDACAEQbgBaiAEQThqKQMANwMA'
  + 'IARCwAE3A4ACIARBwAA2AogCIAQgBCkDADcDgAEgBEHAAGogAxCCgICAACAEQeADaiSAgICAAA'
  + 'uHAgEDfyAAIAApA8ABIAKtfDcDwAECQCAAKALIASIDDQAgAkGAAUkNAANAIAAgARCEgICAACAB'
  + 'QYABaiEBIAJBgH9qIgJB/wBLDQALIAAoAsgBIQMLIABBwABqIgQgA2ogASACQYABIANrIgMgAi'
  + 'ADSRsiA/wKAAAgACADIAAoAsgBaiIFNgLIAQJAIAVBgAFHDQAgACAEEISAgIAAIAAgAiADayIC'
  + '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'
  + 'QoD+A4MgA0I4iISEhDcDKCACIAEpAzAiA0I4hiADQiiGQoCAgICAgMD/AIOEIANCGIZCgICAgI'
  + 'DgP4MgA0IIhkKAgICA8B+DhIQgA0IIiEKAgID4D4MgA0IYiEKAgPwHg4QgA0IoiEKA/gODIANC'
  + 'OIiEhIQ3AzAgAiABKQM4IgNCOIYgA0IohkKAgICAgIDA/wCDhCADQhiGQoCAgICA4D+DIANCCI'
  + 'ZCgICAgPAfg4SEIANCCIhCgICA+A+DIANCGIhCgID8B4OEIANCKIhCgP4DgyADQjiIhISENwM4'
  + 'IAIgASkDQCIDQjiGIANCKIZCgICAgICAwP8Ag4QgA0IYhkKAgICAgOA/gyADQgiGQoCAgIDwH4'
  + 'OEhCADQgiIQoCAgPgPgyADQhiIQoCA/AeDhCADQiiIQoD+A4MgA0I4iISEhDcDQCACIAEpA0gi'
  + 'A0I4hiADQiiGQoCAgICAgMD/AIOEIANCGIZCgICAgIDgP4MgA0IIhkKAgICA8B+DhIQgA0IIiE'
  + 'KAgID4D4MgA0IYiEKAgPwHg4QgA0IoiEKA/gODIANCOIiEhIQ3A0ggAiABKQNQIgNCOIYgA0Io'
  + 'hkKAgICAgIDA/wCDhCADQhiGQoCAgICA4D+DIANCCIZCgICAgPAfg4SEIANCCIhCgICA+A+DIA'
  + 'NCGIhCgID8B4OEIANCKIhCgP4DgyADQjiIhISENwNQIAIgASkDWCIDQjiGIANCKIZCgICAgICA'
  + 'wP8Ag4QgA0IYhkKAgICAgOA/gyADQgiGQoCAgIDwH4OEhCADQgiIQoCAgPgPgyADQhiIQoCA/A'
  + 'eDhCADQiiIQoD+A4MgA0I4iISEhDcDWCACIAEpA2AiA0I4hiADQiiGQoCAgICAgMD/AIOEIANC'
  + 'GIZCgICAgIDgP4MgA0IIhkKAgICA8B+DhIQgA0IIiEKAgID4D4MgA0IYiEKAgPwHg4QgA0IoiE'
  + 'KA/gODIANCOIiEhIQ3A2AgAiABKQNoIgNCOIYgA0IohkKAgICAgIDA/wCDhCADQhiGQoCAgICA'
  + '4D+DIANCCIZCgICAgPAfg4SEIANCCIhCgICA+A+DIANCGIhCgID8B4OEIANCKIhCgP4DgyADQj'
  + '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'
  + 'q2/LXxdYR0qMGURs';
  
// code to decode base 64, this doesn't check validity and just assumes the
// input is proper base 64
const base64Table = [
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,0,0,1,2,3,4,5,6,
  7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,26,27,28,29,
  30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0
];

function base64Decode(str) {
  const blocks = Math.ceil(str.length / 4);
  const binLength = blocks * 3;
  const bin = new Uint8Array(binLength);
  const binStr = new TextEncoder().encode(str);

  let i, dest = 0;
  for (i = 0; i < binStr.length - 4; i += 4, dest += 3) {
    bin[dest] = base64Table[binStr[i]] << 2;
    bin[dest] |= base64Table[binStr[i + 1]] >> 4;

    bin[dest + 1] = base64Table[binStr[i + 1]] << 4;
    bin[dest + 1] |= base64Table[binStr[i + 2]] >> 2;

    bin[dest + 2] = base64Table[binStr[i + 2]] << 6;
    bin[dest + 2] |= base64Table[binStr[i + 3]];
  }

  let trimN = 0;
  bin[dest] = base64Table[binStr[i]] << 2;
  bin[dest] |= base64Table[binStr[i + 1]] >> 4;

  if (str[i + 2] === '=') {
    trimN = 2;
    bin[dest + 1] = base64Table[binStr[i + 1]] << 4;
  } else {
    if (str[i + 3] !== '=') {
      bin[dest + 1] = base64Table[binStr[i + 1]] << 4;
      bin[dest + 1] |= base64Table[binStr[i + 2]] >> 2;
      bin[dest + 2] = base64Table[binStr[i + 2]] << 6;
      bin[dest + 2] |= base64Table[binStr[i + 3]];
    } else {
      trimN = 1;
      bin[dest + 1] = base64Table[binStr[i + 1]] << 4;
      bin[dest + 1] |= base64Table[binStr[i + 2]] >> 2;
    }
  }

  return bin.subarray(0, bin.length - trimN);
}

function computeKey(email, password) {
  const t0 = performance.now();
  return new Promise(resolve => {
    setTimeout(() => {
      let encoder = new TextEncoder();
      let encodedPassword = encoder.encode(password);
      let salt = encoder.encode(email + '|your-domain-here.com');

      let passwordPtr = basePtr;
      let saltPtr = passwordPtr + encodedPassword.length;
      // +4 to account for the pbkdf2 counter appended to the end of the salt
      let outputPtr = saltPtr + salt.length + 4;

      HEAPU8.set(encodedPassword, passwordPtr);
      HEAPU8.set(salt, saltPtr);

      pbkdf2(
        passwordPtr,            // *password
        encodedPassword.length, // password_len
        saltPtr,                // *salt + 4 empty bytes at the end
        salt.length,            // salt_len
        100000,                 // c
        outputPtr               // *output
      );

      // copy the data from the memory since it will be cleaned up next
      const dk = new Uint8Array(HEAPU8.subarray(outputPtr, outputPtr + 64));

      // clean up memory
      for (let i = passwordPtr; i < outputPtr + 64; ++i) {
        HEAPU8[basePtr + i] = 0x00;
      }
      pbkdf2(basePtr, 0, 1, basePtr);

      console.log(performance.now() - t0);

      resolve(dk);
    }, 0);
  });
}

WebAssembly.instantiate(base64Decode(b64Wasm)).then(obj => {
  const { exports } = obj.instance;
  window.pbkdf2 = exports.pbkdf2;
  window.basePtr = exports.__heap_base;
  window.HEAPU8 = new Uint8Array(exports.memory.buffer);
});
```

You can learn more about WebAssembly with [my video course](https://cmovz.io/learn-webassembly).
